#include "addons/amiibo.h"
#include "addons/amiibo_crypto.h"
#include "drivermanager.h"
#include "pico/rand.h"
#include "storagemanager.h"
#include "FlashPROM.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"

#include <string.h>

#define AMIIBO_SLOT_ADDRESS(i) (EEPROM_ADDRESS_START - ((i) + 1) * FLASH_SECTOR_SIZE)
#define AMIIBO_MAGIC           0x4F424D41

typedef struct {
    uint32_t magic;
    uint16_t length;
    uint16_t reserved;
    char name[AMIIBO_NAME_SIZE];
    uint8_t data[AMIIBO_FILE_SIZE];
} AmiiboSlot;

extern char __flash_binary_end;

#define AMIIBO_KEY_START       (EEPROM_ADDRESS_START - 7 * FLASH_SECTOR_SIZE)
#define AMIIBO_KEY_MAGIC       0x59454B41

struct AmiiboKeys {
    uint32_t magic;
    uint8_t present[2];
    uint8_t data[2][AMIIBO_KEY_SIZE];
};

static const AmiiboKeys * storedKeys() {
    return reinterpret_cast<const AmiiboKeys *>(AMIIBO_KEY_START);
}

static uint32_t tapUntilMs = 0;
static uint32_t generation = 0;
static uint8_t activeData[AMIIBO_DATA_SIZE];
static uint8_t activeSignature[AMIIBO_SIGNATURE_SIZE];
static bool hasSignature = false;

static const AmiiboSlot * storedSlot(uint8_t slot) {
    return reinterpret_cast<const AmiiboSlot *>(AMIIBO_SLOT_ADDRESS(slot));
}

static bool storageAvailable() {
    return (uintptr_t)&__flash_binary_end <= AMIIBO_KEY_START;
}

static void programSector(uint32_t address, const uint8_t * data, uint32_t length) {
    multicore_lockout_start_blocking();
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(address - XIP_BASE, FLASH_SECTOR_SIZE);
    if (data != nullptr)
        flash_range_program(address - XIP_BASE, data, length);
    restore_interrupts(interrupts);
    multicore_lockout_end_blocking();
}

static void programSlot(uint8_t slot, const uint8_t * page) {
    programSector(AMIIBO_SLOT_ADDRESS(slot), page, sizeof(AmiiboSlot) + FLASH_PAGE_SIZE - (sizeof(AmiiboSlot) % FLASH_PAGE_SIZE));
}

bool AmiiboAddon::available() {
    return Storage::getInstance().getAddonOptions().amiiboOptions.enabled;
}

void AmiiboAddon::setup() {
    GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
    for (uint8_t i = 0; i < AMIIBO_SLOT_COUNT; i++) {
        slotPinMasks[i] = 0;
        for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
            if (pinMappings[pin].action == GpioAction::BUTTON_PRESS_AMIIBO_1 + i)
                slotPinMasks[i] |= Mask_t{1} << pin;
        }
    }
}

void AmiiboAddon::reinit() {
    setup();
}

void AmiiboAddon::process() {
    const Mask_t gpio = Storage::getInstance().GetGamepad()->debouncedGpio;
    Mask_t pressed = 0;
    for (uint8_t i = 0; i < AMIIBO_SLOT_COUNT; i++) {
        const Mask_t slotPins = gpio & slotPinMasks[i];
        if (slotPins && !(pressedPins & slotPinMasks[i])) {
            tap(i);
        }
        pressed |= slotPins;
    }
    pressedPins = pressed;
}

void AmiiboAddon::tap(uint8_t slot) {
    if (slot >= AMIIBO_SLOT_COUNT || !slotFilled(slot))
        return;
    tapUntilMs = 0;
    hasSignature = false;
    const uint8_t * original = storedSlot(slot)->data;
    if (slotRandomizeSerial(slot) && keyPresent(false) && keyPresent(true)) {
        uint8_t uid[7] = { 0x04 };
        do {
            for (size_t i = 1; i < sizeof(uid); i++)
                uid[i] = get_rand_32() & 0xFF;
        } while ((memcmp(uid, activeData, 3) == 0 && memcmp(uid + 3, activeData + 4, 4) == 0)
            || (memcmp(uid, original, 3) == 0 && memcmp(uid + 3, original + 4, 4) == 0));
        if (!amiiboRandomize(original, activeData, storedKeys()->data[0], storedKeys()->data[1], uid)) {
            return;
        }
    } else {
        memcpy(activeData, original, sizeof(activeData));
        hasSignature = slotSize(slot) == AMIIBO_FILE_SIZE;
        if (hasSignature)
            memcpy(activeSignature, original + AMIIBO_DATA_SIZE, sizeof(activeSignature));
    }
    generation++;
    tapUntilMs = to_ms_since_boot(get_absolute_time()) + AMIIBO_TAP_MS;
}

uint32_t AmiiboAddon::tapGeneration() {
    return generation;
}

bool AmiiboAddon::tagPresent() {
    return tapUntilMs != 0 && (int32_t)(tapUntilMs - to_ms_since_boot(get_absolute_time())) > 0;
}

const uint8_t * AmiiboAddon::tagData() {
    return tagPresent() ? activeData : nullptr;
}

const uint8_t * AmiiboAddon::tagSignature() {
    return tagPresent() && hasSignature ? activeSignature : nullptr;
}

bool AmiiboAddon::slotFilled(uint8_t slot) {
    if (slot >= AMIIBO_SLOT_COUNT || !storageAvailable())
        return false;
    const AmiiboSlot * stored = storedSlot(slot);
    return stored->magic == AMIIBO_MAGIC && (stored->length == AMIIBO_DATA_SIZE || stored->length == AMIIBO_FILE_SIZE);
}

uint16_t AmiiboAddon::slotSize(uint8_t slot) {
    return slotFilled(slot) ? storedSlot(slot)->length : 0;
}

const char * AmiiboAddon::slotName(uint8_t slot) {
    return slotFilled(slot) ? storedSlot(slot)->name : "";
}

bool AmiiboAddon::slotRandomizeSerial(uint8_t slot) {
    return slotFilled(slot) && storedSlot(slot)->reserved == 0;
}

bool AmiiboAddon::setSlotRandomizeSerial(uint8_t slot, bool enabled) {
    if (!DriverManager::getInstance().isConfigMode() || !slotFilled(slot))
        return false;
    if (slotRandomizeSerial(slot) == enabled)
        return true;
    const AmiiboSlot * original = storedSlot(slot);
    if (enabled && keyPresent(false) && keyPresent(true)) {
        uint8_t uid[7];
        memcpy(uid, original->data, 3);
        memcpy(uid + 3, original->data + 4, 4);
        static uint8_t checked[AMIIBO_DATA_SIZE];
        if (!amiiboRandomize(original->data, checked, storedKeys()->data[0], storedKeys()->data[1], uid))
            return false;
    }
    static uint8_t page[sizeof(AmiiboSlot) + FLASH_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));
    memcpy(page, original, sizeof(AmiiboSlot));
    reinterpret_cast<AmiiboSlot *>(page)->reserved = enabled ? 0 : 1;
    programSlot(slot, page);
    return slotFilled(slot) && slotRandomizeSerial(slot) == enabled;
}

bool AmiiboAddon::writeSlot(uint8_t slot, const uint8_t * data, uint16_t length, const char * name) {
    if (!DriverManager::getInstance().isConfigMode() || data == nullptr || name == nullptr
        || slot >= AMIIBO_SLOT_COUNT || !storageAvailable() || (length != AMIIBO_DATA_SIZE && length != AMIIBO_FILE_SIZE))
        return false;
    if (slotRandomizeSerial(slot) && keyPresent(false) && keyPresent(true)) {
        uint8_t uid[7];
        memcpy(uid, data, 3);
        memcpy(uid + 3, data + 4, 4);
        static uint8_t checked[AMIIBO_DATA_SIZE];
        if (!amiiboRandomize(data, checked, storedKeys()->data[0], storedKeys()->data[1], uid))
            return false;
    }
    static uint8_t page[sizeof(AmiiboSlot) + FLASH_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));
    AmiiboSlot * stored = reinterpret_cast<AmiiboSlot *>(page);
    stored->magic = AMIIBO_MAGIC;
    stored->length = length;
    stored->reserved = slotRandomizeSerial(slot) ? 0 : 1;
    memset(stored->name, 0, sizeof(stored->name));
    strncpy(stored->name, name, sizeof(stored->name) - 1);
    memcpy(stored->data, data, length);
    programSlot(slot, page);
    return slotFilled(slot);
}

bool AmiiboAddon::clearSlot(uint8_t slot) {
    if (!DriverManager::getInstance().isConfigMode() || slot >= AMIIBO_SLOT_COUNT || !storageAvailable())
        return false;
    programSlot(slot, nullptr);
    return !slotFilled(slot);
}

bool AmiiboAddon::keyPresent(bool locked) {
    const AmiiboKeys * keys = storedKeys();
    const size_t index = locked ? 1 : 0;
    return storageAvailable() && keys->magic == AMIIBO_KEY_MAGIC && keys->present[index] == 1
        && amiiboKeyValid(keys->data[index], locked);
}

bool AmiiboAddon::writeKey(bool locked, const uint8_t * data, size_t length) {
    if (!DriverManager::getInstance().isConfigMode() || !storageAvailable()
        || data == nullptr || length != AMIIBO_KEY_SIZE || !amiiboKeyValid(data, locked))
        return false;
    uint8_t page[FLASH_PAGE_SIZE] = {};
    AmiiboKeys * keys = reinterpret_cast<AmiiboKeys *>(page);
    keys->magic = AMIIBO_KEY_MAGIC;
    for (size_t i = 0; i < 2; i++) {
        if (keyPresent(i == 1)) {
            keys->present[i] = 1;
            memcpy(keys->data[i], storedKeys()->data[i], AMIIBO_KEY_SIZE);
        }
    }
    const size_t index = locked ? 1 : 0;
    keys->present[index] = 1;
    memcpy(keys->data[index], data, length);
    programSector(AMIIBO_KEY_START, page, sizeof(page));
    return keyPresent(locked) && memcmp(storedKeys()->data[index], data, length) == 0;
}

bool AmiiboAddon::clearKeys() {
    if (!DriverManager::getInstance().isConfigMode() || !storageAvailable())
        return false;
    programSector(AMIIBO_KEY_START, nullptr, 0);
    return !keyPresent(false) && !keyPresent(true);
}
