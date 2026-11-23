#include "addons/nfcreader.h"
#include "peripheralmanager.h"
#include "drivermanager.h"
#include "storagemanager.h"

#include "hardware/sync.h"
#include "pico/time.h"

#include <stdio.h>
#include <string.h>

#define PN532_CMD_GET_FIRMWARE_VERSION 0x02
#define PN532_CMD_SAM_CONFIGURATION    0x14
#define PN532_CMD_RF_CONFIGURATION     0x32
#define PN532_CMD_IN_DATA_EXCHANGE     0x40
#define PN532_CMD_IN_COMMUNICATE_THRU  0x42
#define PN532_CMD_IN_LIST_PASSIVE      0x4A

#define PN532_HOST_TO_PN532 0xD4
#define PN532_PN532_TO_HOST 0xD5
#define PN532_IC_ID         0x32

#define NTAG_CMD_READ      0x30
#define NTAG_CMD_READ_SIG  0x3C
#define NTAG_READ_SIZE     16
#define NTAG215_PAGES      135
#define NTAG215_LAST_READ  (NTAG215_PAGES - NTAG_READ_SIZE / 4)
#define NTAG_UID_SIZE      7
#define NTAG_SAK           0x00

#define PN532_I2C_TIMEOUT_US   5000
#define PN532_STATUS_POLL_US   1000
#define PN532_CMD_TIMEOUT_MS   150
#define PN532_POWER_UP_MS      100
#define PN532_FIND_INTERVAL_MS 1000
#define PN532_SCAN_INTERVAL_MS 3000
#define PN532_POLL_INTERVAL_MS 120
#define PN532_FIELD_OFF_MS     10
#define PN532_FIELD_SETTLE_MS  5
#define PN532_REMOVE_MISSES    2
#define PN532_MAX_ERRORS       4

static const uint8_t PN532_ACK[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };

static NFCReaderStatus readerStatus = {};
static uint8_t tappedUid[NFC_UID_MAX_SIZE];
static volatile uint8_t tappedUidLength = 0;
static volatile uint32_t taps = 0;

static uint32_t nowMs() {
    return to_ms_since_boot(get_absolute_time());
}

static bool reached(uint32_t deadline, uint32_t now) {
    return (int32_t)(now - deadline) >= 0;
}

static uint8_t readStartPage(uint8_t page) {
    // Real NTAG215s wrap past the last page, but emulators reject the read.
    return page > NTAG215_LAST_READ ? NTAG215_LAST_READ : page;
}

static void setNote(const char * note) {
    snprintf(readerStatus.note, sizeof(readerStatus.note), "%s", note);
}

NFCReaderStatus NFCReaderAddon::status() {
    return readerStatus;
}

uint32_t NFCReaderAddon::tapCount() {
    return taps;
}

uint8_t NFCReaderAddon::tapUid(uint8_t * out) {
    const uint8_t length = tappedUidLength;
    memcpy(out, tappedUid, length);
    return length;
}

bool NFCReaderAddon::available() {
    const AddonOptions & options = Storage::getInstance().getAddonOptions();
    readAmiibo = options.amiiboOptions.enabled && DriverManager::getInstance().getInputMode() == INPUT_MODE_SWITCH2_PRO;
    readerStatus.enabled = options.nfcReaderOptions.enabled;
    return readerStatus.enabled;
}

void NFCReaderAddon::setup() {
    phase = Phase::Find;
    io = Io::Idle;
    nextActionMs = nowMs() + PN532_POWER_UP_MS;
}

void NFCReaderAddon::process() {
    if (io != Io::Idle) {
        const int8_t result = pollResponse();
        if (result < 0 && phase == Phase::ReadSignature) {
            abortCommand();
            finishAmiiboRead(nullptr);
        } else if (result < 0) {
            fail();
        } else if (result > 0) {
            handleResponse();
        }
        return;
    }
    if (reached(nextActionMs, nowMs()))
        issue();
}

bool NFCReaderAddon::i2cWrite(const uint8_t * data, size_t length) {
    return i2c_write_timeout_us(i2c, address, data, length, false, PN532_I2C_TIMEOUT_US) == (int)length;
}

bool NFCReaderAddon::i2cRead(uint8_t * data, size_t length) {
    return i2c_read_timeout_us(i2c, address, data, length, false, PN532_I2C_TIMEOUT_US) == (int)length;
}

bool NFCReaderAddon::startCommand(const uint8_t * cmd, uint8_t length) {
    uint8_t out[NFC_FRAME_MAX];
    const uint8_t frameLength = length + 1;
    uint8_t sum = PN532_HOST_TO_PN532;
    size_t n = 0;
    out[n++] = 0x00;
    out[n++] = 0x00;
    out[n++] = 0xFF;
    out[n++] = frameLength;
    out[n++] = (uint8_t)(~frameLength + 1);
    out[n++] = PN532_HOST_TO_PN532;
    for (uint8_t i = 0; i < length; i++) {
        out[n++] = cmd[i];
        sum += cmd[i];
    }
    out[n++] = (uint8_t)(~sum + 1);
    out[n++] = 0x00;
    if (!i2cWrite(out, n))
        return false;
    command = cmd[0];
    io = Io::WaitAck;
    ioDeadlineMs = nowMs() + PN532_CMD_TIMEOUT_MS;
    nextPollUs = time_us_32() + PN532_STATUS_POLL_US;
    return true;
}

void NFCReaderAddon::abortCommand() {
    if (i2c != nullptr)
        i2cWrite(PN532_ACK, sizeof(PN532_ACK));
    io = Io::Idle;
}

// Returns -1 on error or timeout, 0 while waiting, 1 when a response is ready.
int8_t NFCReaderAddon::pollResponse() {
    const uint32_t nowUs = time_us_32();
    if ((int32_t)(nowUs - nextPollUs) < 0)
        return 0;
    nextPollUs = nowUs + PN532_STATUS_POLL_US;
    if (reached(ioDeadlineMs, nowMs()))
        return -1;

    // Every I2C read starts with a status byte whose bit 0 says a frame is ready.
    uint8_t status = 0;
    if (!i2cRead(&status, 1))
        return -1;
    if ((status & 0x01) == 0)
        return 0;

    if (io == Io::WaitAck) {
        uint8_t ack[1 + sizeof(PN532_ACK)];
        if (!i2cRead(ack, sizeof(ack)) || memcmp(&ack[1], PN532_ACK, sizeof(PN532_ACK)) != 0)
            return -1;
        io = Io::WaitResponse;
        return 0;
    }

    if (!i2cRead(frame, sizeof(frame)))
        return -1;
    io = Io::Idle;

    size_t start = 1;
    while (start < 5 && !(frame[start] == 0x00 && frame[start + 1] == 0xFF))
        start++;
    if (!(frame[start] == 0x00 && frame[start + 1] == 0xFF))
        return -1;
    const uint8_t length = frame[start + 2];
    if ((uint8_t)(length + frame[start + 3]) != 0 || length < 2)
        return -1;
    const size_t body = start + 4;
    if (body + length + 1 > sizeof(frame))
        return -1;
    uint8_t sum = 0;
    for (size_t i = 0; i <= length; i++)
        sum += frame[body + i];
    if (sum != 0 || frame[body] != PN532_PN532_TO_HOST || frame[body + 1] != (uint8_t)(command + 1))
        return -1;
    response = &frame[body + 2];
    responseLength = length - 2;
    return 1;
}

void NFCReaderAddon::schedule(Phase next, uint32_t delayMs) {
    phase = next;
    nextActionMs = nowMs() + delayMs;
}

void NFCReaderAddon::idle() {
    // A tag on the reader is checked with the field left on where possible: many amiibo
    // emulators pick a new random UID every time they power up.
    const bool checkTag = tagLoaded && checkByRead && !amiiboPending;
    schedule(checkTag ? Phase::CheckPresent : Phase::FieldOff, PN532_POLL_INTERVAL_MS);
}

void NFCReaderAddon::fail() {
    abortCommand();
    if (++errors >= PN532_MAX_ERRORS) {
        errors = 0;
        tagLoaded = false;
        amiiboPending = false;
        i2c = nullptr;
        readerStatus.detected = false;
        readerStatus.firmware[0] = 0;
        readerStatus.tagPresent = false;
        readerStatus.amiibo = false;
        AmiiboAddon::clearReaderTag();
        schedule(Phase::Find, PN532_FIND_INTERVAL_MS);
    } else if (phase == Phase::GetFirmware || phase == Phase::SamConfig || phase == Phase::SetRetries) {
        schedule(Phase::GetFirmware, 50);
    } else {
        idle();
    }
}

void NFCReaderAddon::scanBuses() {
    char out[sizeof(readerStatus.i2cScan)] = {};
    size_t n = 0;
    bool anyBlock = false;
    PeripheralManager & peripherals = PeripheralManager::getInstance();
    for (uint8_t block = 0; block < NUM_I2CS; block++) {
        if (!peripherals.isI2CEnabled(block))
            continue;
        anyBlock = true;
        i2c_inst_t * bus = peripherals.getI2C(block)->getController();
        n += snprintf(out + n, sizeof(out) - n, "%si2c%u:", n ? " " : "", block);
        bool found = false;
        for (uint8_t addr = 0x08; addr < 0x78 && n < sizeof(out) - 6; addr++) {
            uint8_t rx;
            if (i2c_read_timeout_us(bus, addr, &rx, 1, false, 2000) >= 0) {
                n += snprintf(out + n, sizeof(out) - n, " 0x%02X", addr);
                found = true;
            }
        }
        if (!found && n < sizeof(out) - 8)
            n += snprintf(out + n, sizeof(out) - n, " none");
    }
    snprintf(readerStatus.i2cScan, sizeof(readerStatus.i2cScan), "%s", anyBlock ? out : "no I2C block enabled");
}

void NFCReaderAddon::tagPlaced(const uint8_t * tagUid, uint8_t length, uint8_t sak) {
    memcpy(uid, tagUid, length);
    uidLength = length;
    tagLoaded = true;
    misses = 0;
    checkByRead = length == NTAG_UID_SIZE && sak == NTAG_SAK;

    readerStatus.tagPresent = true;
    memcpy(readerStatus.uid, tagUid, length);
    readerStatus.uidLength = length;
    readerStatus.amiibo = false;
    readerStatus.hasSignature = false;
    setNote("");

    memcpy(tappedUid, tagUid, length);
    tappedUidLength = length;
    __dmb();
    taps = taps + 1;

    amiiboPending = readAmiibo && checkByRead;
    if (amiiboPending) {
        page = 0;
        schedule(Phase::ReadPages, 0);
        return;
    }
    if (readAmiibo) {
        char note[sizeof(readerStatus.note)];
        snprintf(note, sizeof(note), "not an amiibo (UID length %u, SAK 0x%02X)", length, sak);
        setNote(note);
    }
    idle();
}

void NFCReaderAddon::tagMissing() {
    if (tagLoaded && ++misses >= PN532_REMOVE_MISSES) {
        tagLoaded = false;
        amiiboPending = false;
        misses = 0;
        readerStatus.tagPresent = false;
        readerStatus.amiibo = false;
        readerStatus.hasSignature = false;
        AmiiboAddon::clearReaderTag();
    }
    schedule(Phase::FieldOff, PN532_POLL_INTERVAL_MS);
}

void NFCReaderAddon::finishAmiiboRead(const uint8_t * signature) {
    AmiiboAddon::setReaderTag(dump, signature);
    amiiboPending = false;
    readerStatus.amiibo = true;
    readerStatus.hasSignature = signature != nullptr;
    setNote(signature != nullptr ? "" : "tag did not return a signature");
    idle();
}

void NFCReaderAddon::issue() {
    if (phase == Phase::Find) {
        PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice({ NFC_READER_I2C_ADDRESS });
        if (result.address < 0) {
            if (reached(nextScanMs, nowMs())) {
                scanBuses();
                nextScanMs = nowMs() + PN532_SCAN_INTERVAL_MS;
            }
            schedule(Phase::Find, PN532_FIND_INTERVAL_MS);
            return;
        }
        readerStatus.i2cScan[0] = 0;
        address = result.address;
        i2c = PeripheralManager::getInstance().getI2C(result.block)->getController();
        errors = 0;
        schedule(Phase::GetFirmware, 10);
        return;
    }

    uint8_t cmd[5];
    uint8_t length = 0;
    switch (phase) {
        case Phase::GetFirmware:
            cmd[length++] = PN532_CMD_GET_FIRMWARE_VERSION;
            break;
        case Phase::SamConfig:
            cmd[length++] = PN532_CMD_SAM_CONFIGURATION;
            cmd[length++] = 0x01;
            cmd[length++] = 0x14;
            cmd[length++] = 0x01;
            break;
        case Phase::SetRetries:
            // Without a passive activation retry limit, InListPassiveTarget never returns when no tag is present.
            cmd[length++] = PN532_CMD_RF_CONFIGURATION;
            cmd[length++] = 0x05;
            cmd[length++] = 0xFF;
            cmd[length++] = 0x01;
            cmd[length++] = 0x01;
            break;
        case Phase::FieldOff:
        case Phase::FieldOn:
            // Briefly cutting the field resets a tag that is already selected, so it answers the next poll.
            cmd[length++] = PN532_CMD_RF_CONFIGURATION;
            cmd[length++] = 0x01;
            cmd[length++] = phase == Phase::FieldOn ? 0x03 : 0x02;
            break;
        case Phase::ListTarget:
            cmd[length++] = PN532_CMD_IN_LIST_PASSIVE;
            cmd[length++] = 0x01;
            cmd[length++] = 0x00;
            break;
        case Phase::CheckPresent:
        case Phase::ReadPages:
            cmd[length++] = PN532_CMD_IN_DATA_EXCHANGE;
            cmd[length++] = 0x01;
            cmd[length++] = NTAG_CMD_READ;
            cmd[length++] = phase == Phase::CheckPresent ? 0x00 : readStartPage(page);
            break;
        case Phase::ReadSignature:
            cmd[length++] = PN532_CMD_IN_COMMUNICATE_THRU;
            cmd[length++] = NTAG_CMD_READ_SIG;
            cmd[length++] = 0x00;
            break;
        default:
            return;
    }
    if (!startCommand(cmd, length))
        fail();
}

void NFCReaderAddon::handleResponse() {
    const uint8_t * data = response;
    switch (phase) {
        case Phase::GetFirmware:
            if (responseLength < 4 || data[0] != PN532_IC_ID) {
                fail();
                return;
            }
            readerStatus.detected = true;
            snprintf(readerStatus.firmware, sizeof(readerStatus.firmware), "%u.%u", data[1], data[2]);
            schedule(Phase::SamConfig, 0);
            return;
        case Phase::SamConfig:
            schedule(Phase::SetRetries, 0);
            return;
        case Phase::SetRetries:
            errors = 0;
            schedule(Phase::FieldOff, 0);
            return;
        case Phase::FieldOff:
            schedule(Phase::FieldOn, PN532_FIELD_OFF_MS);
            return;
        case Phase::FieldOn:
            schedule(Phase::ListTarget, PN532_FIELD_SETTLE_MS);
            return;
        case Phase::ListTarget: {
            // NbTg, Tg, SENS_RES (2), SEL_RES, NFCID length, NFCID
            errors = 0;
            if (responseLength < 6 || data[0] == 0) {
                tagMissing();
                return;
            }
            const uint8_t length = data[5] < NFC_UID_MAX_SIZE ? data[5] : NFC_UID_MAX_SIZE;
            if (responseLength < 6 + length) {
                tagMissing();
                return;
            }
            const uint8_t * found = &data[6];
            if (tagLoaded && length == uidLength && memcmp(found, uid, length) == 0) {
                misses = 0;
                if (amiiboPending) {
                    page = 0;
                    schedule(Phase::ReadPages, 0);
                } else {
                    idle();
                }
                return;
            }
            if (tagLoaded)
                AmiiboAddon::clearReaderTag();
            tagPlaced(found, length, data[4]);
            return;
        }
        case Phase::CheckPresent:
            if (responseLength >= 1 + NTAG_READ_SIZE && data[0] == 0x00
                && memcmp(&data[1], uid, 3) == 0 && memcmp(&data[5], &uid[3], 4) == 0) {
                misses = 0;
                idle();
            } else {
                tagMissing();
            }
            return;
        case Phase::ReadPages: {
            if (responseLength < 1 + NTAG_READ_SIZE || data[0] != 0x00) {
                char note[sizeof(readerStatus.note)];
                snprintf(note, sizeof(note), "read failed at page %u (status 0x%02X)", page, responseLength > 0 ? data[0] : 0xFF);
                setNote(note);
                tagMissing();
                return;
            }
            memcpy(&dump[readStartPage(page) * 4], &data[1], NTAG_READ_SIZE);
            // Amiibo use a non-standard capability container, so only the UID is checked here.
            if (page == 0 && (memcmp(dump, uid, 3) != 0 || memcmp(&dump[4], &uid[3], 4) != 0)) {
                setNote("UID in page 0 does not match anticollision UID");
                amiiboPending = false;
                idle();
                return;
            }
            page += NTAG_READ_SIZE / 4;
            schedule(page < NTAG215_PAGES ? Phase::ReadPages : Phase::ReadSignature, 0);
            return;
        }
        case Phase::ReadSignature:
            finishAmiiboRead(responseLength >= 1 + AMIIBO_SIGNATURE_SIZE && data[0] == 0x00 ? &data[1] : nullptr);
            return;
        default:
            return;
    }
}

bool NFCReaderInputAddon::available() {
    return Storage::getInstance().getAddonOptions().nfcReaderOptions.enabled;
}

void NFCReaderInputAddon::setup() {
    lastTapCount = NFCReaderAddon::tapCount();
}

void NFCReaderInputAddon::preprocess() {
    const NFCReaderOptions & options = Storage::getInstance().getAddonOptions().nfcReaderOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const uint32_t now = nowMs();

    const uint32_t count = NFCReaderAddon::tapCount();
    if (count != lastTapCount) {
        lastTapCount = count;
        uint8_t tapped[NFC_UID_MAX_SIZE];
        const uint8_t length = NFCReaderAddon::tapUid(tapped);
        activeTag = -1;
        for (pb_size_t i = 0; i < options.tags_count; i++) {
            const NFCReaderTag & tag = options.tags[i];
            if (tag.actionType != NFC_TAG_ACTION_NONE && tag.uid.size == length && memcmp(tag.uid.bytes, tapped, length) == 0) {
                activeTag = i;
                holdUntilMs = now + tag.holdMs;
                break;
            }
        }
    }

    if (activeTag >= 0 && (int32_t)(now - holdUntilMs) > 0)
        activeTag = -1;
    if (activeTag < 0) {
        gamepad->setHeldHotkey(HOTKEY_NONE);
        return;
    }

    const NFCReaderTag & tag = options.tags[activeTag];
    if (tag.actionType == NFC_TAG_ACTION_HOTKEY) {
        gamepad->setHeldHotkey(tag.hotkey);
        return;
    }
    gamepad->setHeldHotkey(HOTKEY_NONE);
    if (tag.buttonMask & GAMEPAD_MASK_DU)
        gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (tag.buttonMask & GAMEPAD_MASK_DD)
        gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (tag.buttonMask & GAMEPAD_MASK_DL)
        gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (tag.buttonMask & GAMEPAD_MASK_DR)
        gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
    gamepad->state.buttons |= tag.buttonMask & ~(GAMEPAD_MASK_DU | GAMEPAD_MASK_DD | GAMEPAD_MASK_DL | GAMEPAD_MASK_DR);
}
