// IMPORTANT: Do NOT include tusb.h or any TinyUSB header in this translation unit.
// hid_report_type_t is defined by both TinyUSB and BTstack — including both
// in the same translation unit causes a compile error.

#include "BLEHIDManager.h"
#include "FlashPROM.h"
#include <pico/btstack_flash_bank.h>
#include "ble/le_device_db.h"

// Battery-sense macros (BATTERY_ADC_GPIO / BATTERY_ADC_CHANNEL /
// BATTERY_VOLTAGE_DIVIDER / BATTERY_MIN_VOLTAGE / BATTERY_MAX_VOLTAGE) live in
// the per-board BatteryConfig.h when the board has battery sense hardware.
// BoardConfig.h itself cannot be included here — it pulls in TinyUSB's hid.h,
// which conflicts with BTstack's hid_report_type_t in this translation unit.
// Without these macros, _readBatteryPercent() silently returns the 100 %
// fallback, so battery reporting appears stuck at full.
#if defined(__has_include)
#  if __has_include("BatteryConfig.h")
#    include "BatteryConfig.h"
#  endif
#endif

#include "ble_identity.h"
#include "version.h"

#include <string.h>

// Pico SDK CYW43 / BTstack headers
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/cyw43_arch.h"
#include "pico/unique_id.h"
#include "pico/async_context.h"
#include "hardware/adc.h"

// BTstack core
#include "btstack.h"

// Generated GATT database header (from src/ble_hid.gatt via pico_btstack_make_gatt_header)
#include "ble_hid.h"

extern char __flash_binary_end;

static constexpr uint32_t CONFIG_STORAGE_OFFSET = EEPROM_ADDRESS_START - XIP_BASE;
static constexpr uint32_t CONFIG_STORAGE_END = CONFIG_STORAGE_OFFSET + EEPROM_SIZE_BYTES;
static constexpr uint32_t BTSTACK_STORAGE_OFFSET = PICO_FLASH_BANK_STORAGE_OFFSET;
static constexpr uint32_t BTSTACK_STORAGE_END = BTSTACK_STORAGE_OFFSET + PICO_FLASH_BANK_TOTAL_SIZE;

static_assert(BTSTACK_STORAGE_END <= CONFIG_STORAGE_OFFSET ||
              CONFIG_STORAGE_END <= BTSTACK_STORAGE_OFFSET,
              "BTstack flash storage overlaps GP2040-CE config storage");
static_assert(CONFIG_STORAGE_END <= PICO_FLASH_SIZE_BYTES,
              "GP2040-CE config storage exceeds flash size");

// HID Report Descriptor: Report ID 1 + 18 generic HID buttons + 6 padding bits
//   + 4 stick axes and 2 trigger axes = 9-byte report body.
//   Button order follows the W3C Standard Gamepad layout, with the D-pad as
//   discrete buttons 13..16 (no hat switch). All four stick axes and both triggers are sent.
// The Report ID is carried by the GATT Report Reference descriptor, not the notification.
static const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,              // USAGE (Game Pad)
    0xA1, 0x01,              // COLLECTION (Application)
    0x85, 0x01,              // REPORT_ID (1)

    // ── 18 digital buttons (Standard Gamepad order) ────────────────────────
    // Button 1..8   = B1, B2, B3, B4, L1, R1, L2, R2
    // Button 9..12  = S1, S2, L3, R3
    // Button 13..16 = D-pad Up, Down, Left, Right
    // Button 17..18 = A1, A2
    0x05, 0x09,              //   USAGE_PAGE (Button)
    0x19, 0x01,              //   USAGE_MINIMUM (Button 1)
    0x29, 0x12,              //   USAGE_MAXIMUM (Button 18)
    0x15, 0x00,              //   LOGICAL_MINIMUM (0)
    0x25, 0x01,              //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,              //   REPORT_SIZE (1)
    0x95, 0x12,              //   REPORT_COUNT (18)
    0x81, 0x02,              //   INPUT (Data,Var,Abs)

    // 6 padding bits to complete byte 2
    0x75, 0x01,              //   REPORT_SIZE (1)
    0x95, 0x06,              //   REPORT_COUNT (6)
    0x81, 0x03,              //   INPUT (Cnst,Var,Abs)

    // ── Sticks and triggers: LX, LY, RX, RY, LT, RT ─────────────────────────
    0x05, 0x01,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,              //   USAGE (X)
    0x09, 0x31,              //   USAGE (Y)
    0x09, 0x32,              //   USAGE (Z)
    0x09, 0x35,              //   USAGE (Rz)
    0x09, 0x33,              //   USAGE (Rx)
    0x09, 0x34,              //   USAGE (Ry)
    0x15, 0x00,              //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,        //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,              //   REPORT_SIZE (8)
    0x95, 0x06,              //   REPORT_COUNT (6)
    0x81, 0x02,              //   INPUT (Data,Var,Abs)

    0xC0,                    // END_COLLECTION
};

static_assert(sizeof(hid_report_descriptor) > 0, "HID descriptor must not be empty");

// BLE advertising data: Flags + Appearance (Gamepad, 964 = 0x03C4) + HID Service UUID
static uint8_t adv_data[] = {
    2, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,                  // LE General Discoverable, BR/EDR Not Supported
    3, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC4, 0x03,       // Gamepad (964 = 0x03C4, little-endian)
    7, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
        0x12, 0x18,   // HID Service (0x1812)
        0x0F, 0x18,   // Battery Service (0x180F)
        0x0A, 0x18,   // Device Information Service (0x180A)
};

// Scan response: complete local name
static uint8_t scan_resp_data[2 + sizeof(BLEHIDManager::DEVICE_NAME) - 1];

static void refreshBondRecency(int index) {
    int addressType = BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t address = {};
    sm_key_t irk = {};
    uint16_t ediv = 0;
    uint8_t rand[8] = {};
    sm_key_t ltk = {};
    int keySize = 0;
    int authenticated = 0;
    int authorized = 0;
    int secureConnection = 0;

    le_device_db_info(index, &addressType, address, irk);
    if (addressType == BD_ADDR_TYPE_UNKNOWN) return;
    le_device_db_encryption_get(index, &ediv, rand, ltk, &keySize,
                                &authenticated, &authorized, &secureConnection);

#ifdef ENABLE_LE_SIGNED_WRITE
    sm_key_t localCsrk = {};
    sm_key_t remoteCsrk = {};
    le_device_db_local_csrk_get(index, localCsrk);
    le_device_db_remote_csrk_get(index, remoteCsrk);
    const uint32_t localCounter = le_device_db_local_counter_get(index);
    const uint32_t remoteCounter = le_device_db_remote_counter_get(index);
#endif

    if (le_device_db_add(addressType, address, irk) != index) return;
    le_device_db_encryption_set(index, ediv, rand, ltk, keySize,
                                authenticated, authorized, secureConnection);
#ifdef ENABLE_LE_SIGNED_WRITE
    le_device_db_local_csrk_set(index, localCsrk);
    le_device_db_remote_csrk_set(index, remoteCsrk);
    le_device_db_local_counter_set(index, localCounter);
    le_device_db_remote_counter_set(index, remoteCounter);
#endif
}

class BTstackLock {
public:
    BTstackLock() { async_context_acquire_lock_blocking(cyw43_arch_async_context()); }
    ~BTstackLock() { async_context_release_lock(cyw43_arch_async_context()); }
    BTstackLock(const BTstackLock&) = delete;
    BTstackLock& operator=(const BTstackLock&) = delete;
};

// BTstack event handler registrations
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

// BLE timing constants
static constexpr uint32_t INIT_RETRY_DELAY_MS = 5000;
static constexpr uint32_t BATTERY_UPDATE_INTERVAL_MS = 30000;

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void BLEHIDManager::init(bool configMode) {
    if (_initRequested) return;
    _initRequested = true;
    _configMode = configMode;
    _pairingMode = false;
    _bootTimeMs = to_ms_since_boot(get_absolute_time());
}

void BLEHIDManager::process() {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (!_initialized) {
        if (!_initRequested) return;

        // Retry-delay after a failed cyw43_arch_init
        if (_initFailed) {
            if ((now - _retryTimeMs) < INIT_RETRY_DELAY_MS) return;
            _initFailed = false;
        }

        // Deferred init: wait _initDelayMs ms after boot before touching the radio
        if ((now - _bootTimeMs) < _initDelayMs) return;

        _doInit();
        return;
    }

    BTstackLock lock;
    if (!_ready) return;

    // Periodic battery level reporting — throttled to once per 30 seconds.
    // Uses the board-defined ADC channel and voltage divider.
    // Only fires when connected and notifications are enabled.
    {
        uint32_t nowMs = to_ms_since_boot(get_absolute_time());
        if (_connected && _notificationsEnabled &&
                (nowMs - _lastBatteryUpdateMs >= BATTERY_UPDATE_INTERVAL_MS || _lastBatteryLevel == 255)) {
            uint8_t level = _readBatteryPercent();
            if (level != _lastBatteryLevel) {
                battery_service_server_set_battery_value(level);
                _lastBatteryLevel = level;
            }
            _lastBatteryUpdateMs = nowMs;
        }
    }

    // Deferred advertising restart — set by disconnect handler, executed here in main loop
    if (_needsAdvRestart && !_connected) {
        adv_data[2] = _pairingMode ? 0x06 : 0x04;
        gap_advertisements_set_data(sizeof(adv_data), adv_data);
        const bool advertise = _pairingMode || _hasBondedPeers;
        gap_advertisements_enable(advertise ? 1 : 0);
        _advStarted = advertise;
        _needsAdvRestart = false;
    }

    if (_reportPending && _connected && _notificationsEnabled) {
        hids_device_request_can_send_now_event(_conHandle);
    }
}

bool BLEHIDManager::sendReport(const uint8_t* report, uint16_t len) {
    if (!_initialized) return false;
    BTstackLock lock;
    if (!_connected || !_notificationsEnabled) return false;

    if (len > REPORT_SIZE_BYTES) len = REPORT_SIZE_BYTES;

    const uint8_t* previousReport = _lastSentReport;
    uint16_t previousLen = REPORT_SIZE_BYTES;
    if (_reportQueueCount > 0) {
        uint8_t lastIndex = (_reportQueueTail + REPORT_QUEUE_DEPTH - 1) % REPORT_QUEUE_DEPTH;
        previousReport = _reportQueue[lastIndex];
        previousLen = _reportQueueLen[lastIndex];
    }

    if (previousLen == len && memcmp(previousReport, report, len) == 0) {
        return true;
    }

    uint8_t writeIndex;
    if (_reportQueueCount < REPORT_QUEUE_DEPTH) {
        writeIndex = _reportQueueTail;
        _reportQueueTail = (_reportQueueTail + 1) % REPORT_QUEUE_DEPTH;
        _reportQueueCount++;
    } else {
        // Preserve the oldest unsent state and collapse only the newest queued state.
        writeIndex = (_reportQueueTail + REPORT_QUEUE_DEPTH - 1) % REPORT_QUEUE_DEPTH;
    }

    memset(_reportQueue[writeIndex], 0, REPORT_SIZE_BYTES);
    memcpy(_reportQueue[writeIndex], report, len);
    _reportQueueLen[writeIndex] = len;
    _reportPending    = (_reportQueueCount > 0);
    return true;
}

BLEHIDStatus BLEHIDManager::getStatus() {
    if (!_initialized) {
        return {false, _initFailed, false, false, false,
                static_cast<uint8_t>(le_device_db_count())};
    }
    BTstackLock lock;
    return {_ready, _initFailed, _connected, _notificationsEnabled, _pairingMode,
            static_cast<uint8_t>(le_device_db_count())};
}

bool BLEHIDManager::clearBonds() {
    if (!_initialized) return false;
    async_context_acquire_lock_blocking(cyw43_arch_async_context());
    if (!_ready || _connected || _pairingMode) {
        async_context_release_lock(cyw43_arch_async_context());
        return false;
    }

    for (int index = 0; index < le_device_db_max_count(); ++index) {
        int addressType = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t address = {};
        le_device_db_info(index, &addressType, address, nullptr);
        if (addressType != BD_ADDR_TYPE_UNKNOWN) {
            gap_delete_bonding(static_cast<bd_addr_type_t>(addressType), address);
        }
    }
    _hasBondedPeers = le_device_db_count() > 0;
    if (!_hasBondedPeers) {
        gap_advertisements_enable(0);
        _advStarted = false;
    }
    async_context_release_lock(cyw43_arch_async_context());
    return true;
}

void BLEHIDManager::clearStorageForFactoryReset() {
    const hal_flash_bank_t* flashBank = pico_flash_bank_instance();
    if (_initialized) {
        async_context_acquire_lock_blocking(cyw43_arch_async_context());
        hci_power_control(HCI_POWER_OFF);
        _ready = false;
        _pairingMode = false;
        flashBank->erase(nullptr, 0);
        flashBank->erase(nullptr, 1);
        async_context_release_lock(cyw43_arch_async_context());
    } else {
        flashBank->erase(nullptr, 0);
        flashBank->erase(nullptr, 1);
    }
}

bool BLEHIDManager::setPairingMode(bool enabled) {
    if (!_initialized) return false;
    BTstackLock lock;
    if (!_ready || _connected) return false;
    _pairingMode = enabled;
    _needsAdvRestart = false;
    adv_data[2] = enabled ? 0x06 : 0x04;
    gap_advertisements_set_data(sizeof(adv_data), adv_data);
    const bool advertise = enabled || _hasBondedPeers;
    gap_advertisements_enable(advertise ? 1 : 0);
    _advStarted = advertise;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void BLEHIDManager::_resetReportQueue() {
    _reportPending    = false;
    _reportQueueHead  = 0;
    _reportQueueTail  = 0;
    _reportQueueCount = 0;
    memset(_lastSentReport, 0, sizeof(_lastSentReport));
}

void BLEHIDManager::_handleDisconnect(uint16_t connectionHandle) {
    if (!_connected || _conHandle != connectionHandle) return;
    _resetReportQueue();
    _connected            = false;
    _notificationsEnabled = false;
    _conHandle            = HCI_CON_HANDLE_INVALID;
    _advStarted           = false;
    // Defer advertising restart to process() to avoid race with LL cleanup
    _needsAdvRestart = true;
}

// Read battery percentage from the onboard voltage divider.
// Thresholds are derived at compile time from board-config macros:
//   BATTERY_VOLTAGE_DIVIDER — resistor divider ratio (e.g. 3.0)
//   BATTERY_MIN_VOLTAGE     — cell empty voltage in V (e.g. 3.0)
//   BATTERY_MAX_VOLTAGE     — cell full voltage in V  (e.g. 4.2)
//   VREF = 3.3 V, ADC = 12-bit (4096 counts)
//   raw = (V_bat / VREF) / BATTERY_VOLTAGE_DIVIDER * 4096
// Falls back to 100 % on boards that do not define BATTERY_ADC_GPIO.
uint8_t BLEHIDManager::_readBatteryPercent() {
#ifdef BATTERY_ADC_GPIO
    constexpr float BATTERY_VREF    = 3.3f;
    constexpr int BATTERY_ADC_MIN   = (int)((BATTERY_MIN_VOLTAGE / BATTERY_VREF) / BATTERY_VOLTAGE_DIVIDER * 4096.0f);
    constexpr int BATTERY_ADC_MAX   = (int)((BATTERY_MAX_VOLTAGE / BATTERY_VREF) / BATTERY_VOLTAGE_DIVIDER * 4096.0f);
    adc_select_input(BATTERY_ADC_CHANNEL);
    uint16_t raw = adc_read();
    if (raw <= BATTERY_ADC_MIN) return 0;
    if (raw >= BATTERY_ADC_MAX) return 100;
    return (uint8_t)((raw - BATTERY_ADC_MIN) * 100 / (BATTERY_ADC_MAX - BATTERY_ADC_MIN));
#else
    return 100;
#endif
}

void BLEHIDManager::_doInit() {
    const uintptr_t firmwareEndOffset =
        reinterpret_cast<uintptr_t>(&__flash_binary_end) - XIP_BASE;
    if (firmwareEndOffset > BTSTACK_STORAGE_OFFSET ||
        firmwareEndOffset > CONFIG_STORAGE_OFFSET) {
        panic("Firmware image overlaps reserved flash storage");
    }

    // Initialize CYW43 wireless chip
    int err = cyw43_arch_init();
    if (err != 0) {
        printf("[BLE] CYW43 initialization failed: %d\n", err);
        _initFailed  = true;
        _retryTimeMs = to_ms_since_boot(get_absolute_time());
        return;
    }

    BTstackLock lock;

    // Check for previously bonded peers — used to skip re-pairing on reconnect
    _hasBondedPeers = (le_device_db_count() > 0);

    // Core protocol layers — SM must be initialized before ATT/GATT services
    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);

    // ATT server — profile_data is generated from ble_hid.gatt by pico_btstack_make_gatt_header.
    // NULL callbacks: hids_device registers its own service handler for all HIDS characteristics.
    att_server_init(profile_data, NULL, NULL);

    // HID over GATT device — boot mode 0 (no boot keyboard/mouse)
    hids_device_init(0, hid_report_descriptor, sizeof(hid_report_descriptor));

    // The system initializes the ADC during setup; initialize only this channel.
#ifdef BATTERY_ADC_GPIO
    adc_gpio_init(BATTERY_ADC_GPIO);
#endif

    // Battery service — init() registers with ATT server; must be called before hci_power_control.
    battery_service_server_init(_readBatteryPercent());

    // Device Information Service — init() registers the ATT read handler.
    // Without this call the DIS characteristics from the imported GATT template
    // exist in the database but every read returns empty, so hosts show
    // "Manufacturer: Unavailable / Model: Unavailable" and PnP ID stays 0.
    device_information_service_server_init();

    // Device Information Service
    device_information_service_server_set_manufacturer_name(BLE_MANUFACTURER_NAME);
    device_information_service_server_set_model_number(BLE_MODEL_NUMBER);
    device_information_service_server_set_hardware_revision(BLE_HARDWARE_REVISION);
    device_information_service_server_set_firmware_revision(GP2040VERSION);
    device_information_service_server_set_software_revision(GP2040VERSION);

    // Serial Number: RP2040/RP2350 unique board ID as uppercase hex (no separators).
    // Stored static so the const char* handed to BTstack remains valid for the
    // lifetime of the DIS server.
    static char ble_serial_number[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    for (unsigned i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
        snprintf(&ble_serial_number[i * 2], 3, "%02X", board_id.id[i]);
    }
    device_information_service_server_set_serial_number(ble_serial_number);

    // PnP ID (0x2A50) — hosts like the Windows Gamepad Tester key the visible
    // vendor / product name off this characteristic. Leaving it unset is the
    // reason the controller previously showed up as
    // "Unknown Gamepad / Vendor: 0000 / Product: 0000".
    device_information_service_server_set_pnp_id(BLE_VENDOR_SOURCE_ID,
                                                 BLE_VENDOR_ID,
                                                 BLE_PRODUCT_ID,
                                                 BLE_PRODUCT_VERSION);

    // Register event handlers
    hci_event_callback_registration.callback = &_hciPacketHandler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &_smPacketHandler;
    sm_add_event_handler(&sm_event_callback_registration);

    // HIDS meta events (HIDS_SUBEVENT_INPUT_REPORT_ENABLE, HIDS_SUBEVENT_CAN_SEND_NOW, etc.)
    // are delivered ONLY through hids_device_register_packet_handler — NOT via hci_add_event_handler.
    // Without this, the handler never fires and reports can never be sent.
    hids_device_register_packet_handler(&_hciPacketHandler);

    // Set up connectable advertising; pairing mode controls discoverability.
    gap_advertisements_set_params(0x0030, 0x0060, 0, 0, NULL, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), (uint8_t*)adv_data);
    scan_resp_data[0] = sizeof(scan_resp_data) - 1;
    scan_resp_data[1] = BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(scan_resp_data + 2, DEVICE_NAME, sizeof(scan_resp_data) - 2);
    gap_scan_response_set_data(sizeof(scan_resp_data), scan_resp_data);

    // Power on the Bluetooth controller — advertising starts after it is ready.
    hci_power_control(HCI_POWER_ON);

    _initialized = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// HCI / HIDS event handler
// ─────────────────────────────────────────────────────────────────────────────

void BLEHIDManager::_hciPacketHandler(uint8_t packetType, uint16_t channel,
                                      uint8_t* packet, uint16_t size) {
    (void)channel;
    (void)size;

    BLEHIDManager& mgr = getInstance();

    if (packetType != HCI_EVENT_PACKET) return;

    uint8_t eventCode = hci_event_packet_get_type(packet);

    switch (eventCode) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                mgr._ready = true;
                adv_data[2] = mgr._pairingMode ? 0x06 : 0x04;
                gap_advertisements_set_data(sizeof(adv_data), adv_data);
                if (mgr._pairingMode || mgr._hasBondedPeers) {
                    gap_advertisements_enable(1);
                    mgr._advStarted = true;
                }
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            uint16_t connectionHandle =
                hci_event_disconnection_complete_get_connection_handle(packet);
            mgr._handleDisconnect(connectionHandle);
            break;
        }

        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) ==
                    HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                if (hci_subevent_le_connection_complete_get_status(packet) != ERROR_CODE_SUCCESS) {
                    mgr._needsAdvRestart = true;
                    break;
                }
                if (mgr._configMode && !mgr._pairingMode && !mgr._hasBondedPeers) {
                    gap_disconnect(hci_subevent_le_connection_complete_get_connection_handle(packet));
                    break;
                }
                mgr._conHandle  = hci_subevent_le_connection_complete_get_connection_handle(packet);
                mgr._connected  = true;
                mgr._advStarted = false;
                gap_request_connection_parameter_update(mgr._conHandle, 6, 12, 0, 200);
            }
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    {
                        uint8_t enable = hids_subevent_input_report_enable_get_enable(packet);
                        mgr._notificationsEnabled = (enable != 0);
                    }
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    if (mgr._reportPending && mgr._reportQueueCount > 0) {
                        uint8_t readIndex = mgr._reportQueueHead;
                        uint16_t reportLen = mgr._reportQueueLen[readIndex];

                        hids_device_send_input_report(mgr._conHandle,
                                                      mgr._reportQueue[readIndex],
                                                      reportLen);
                        if (memcmp(mgr._lastSentReport, mgr._reportQueue[readIndex], reportLen) != 0) {
                            memset(mgr._lastSentReport, 0, BLEHIDManager::REPORT_SIZE_BYTES);
                            memcpy(mgr._lastSentReport, mgr._reportQueue[readIndex], reportLen);
                        }
                        mgr._reportQueueHead = (mgr._reportQueueHead + 1) % BLEHIDManager::REPORT_QUEUE_DEPTH;
                        mgr._reportQueueCount--;
                        mgr._reportPending = (mgr._reportQueueCount > 0);
                    }
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SM (Security Manager) event handler
// ─────────────────────────────────────────────────────────────────────────────

void BLEHIDManager::_smPacketHandler(uint8_t packetType, uint16_t channel,
                                     uint8_t* packet, uint16_t size) {
    (void)channel;
    (void)size;

    if (packetType != HCI_EVENT_PACKET) return;

    BLEHIDManager& mgr = getInstance();

    switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            if (!mgr._pairingMode) {
                sm_bonding_decline(sm_event_just_works_request_get_handle(packet));
            } else {
                sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            }
            break;
        case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED:
            // BTstack recognized a reconnecting bonded peer via IRK resolution.
            // Do NOT set _notificationsEnabled here — the LTK encryption handshake
            // has not completed yet. Arming notifications before the link is encrypted
            // causes an ATT security mode error, which disconnects the peer and creates
            // a reconnect loop. _notificationsEnabled is set in HCI_EVENT_ENCRYPTION_CHANGE.
            refreshBondRecency(sm_event_identity_resolving_succeeded_get_index(packet));
            mgr._hasBondedPeers = true;
            break;
        case SM_EVENT_IDENTITY_RESOLVING_FAILED: {
            // New peers and non-resolvable addresses must not erase other saved devices.
            mgr._hasBondedPeers = (le_device_db_count() > 0);
            if (!mgr._pairingMode) {
                gap_disconnect(sm_event_identity_resolving_failed_get_handle(packet));
            }
            break;
        }
        case SM_EVENT_PAIRING_COMPLETE: {
            uint8_t status = sm_event_pairing_complete_get_status(packet);
            if (status == ERROR_CODE_SUCCESS) {
                mgr._hasBondedPeers = true;
                mgr._pairingMode = false;
                adv_data[2] = 0x04;
                gap_advertisements_set_data(sizeof(adv_data), adv_data);
            }
            // On failure: do nothing. Bond was not stored. Advertising will restart
            // on disconnect and the host can try again.
            break;
        }
        default:
            break;
    }
}
