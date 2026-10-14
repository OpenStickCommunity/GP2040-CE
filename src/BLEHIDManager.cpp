// IMPORTANT: Do NOT include tusb.h or any TinyUSB header in this translation unit.
// hid_report_type_t is defined by both TinyUSB and BTstack — including both
// in the same translation unit causes a compile error.

#include "BLEHIDManager.h"

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
#include "pico/btstack_run_loop_async_context.h"
#include "pico/unique_id.h"
#include "pico/async_context.h"
#include "hardware/adc.h"

// BTstack core
#include "btstack.h"

// Generated GATT database header (from src/ble_hid.gatt via pico_btstack_make_gatt_header)
#include "ble_hid.h"

// HID Report Descriptor: Report ID 1 + 18 generic HID buttons + 6 padding bits
//   + 4 x 8-bit stick axes (X, Y, Z, Rz) = 7-byte report body.
//   Button order follows the W3C Standard Gamepad layout, with the D-pad as
//   discrete buttons 13..16 (no hat switch). Axes are currently sent neutral.
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

    // ── Analog sticks: LX, LY, RX, RY (centered = 0x80) ─────────────────────
    0x05, 0x01,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,              //   USAGE (X)
    0x09, 0x31,              //   USAGE (Y)
    0x09, 0x32,              //   USAGE (Z)
    0x09, 0x35,              //   USAGE (Rz)
    0x15, 0x00,              //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,        //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,              //   REPORT_SIZE (8)
    0x95, 0x04,              //   REPORT_COUNT (4)
    0x81, 0x02,              //   INPUT (Data,Var,Abs)

    0xC0,                    // END_COLLECTION
};

static_assert(sizeof(hid_report_descriptor) > 0, "HID descriptor must not be empty");

// BLE advertising data: Flags + Appearance (Gamepad, 964 = 0x03C4) + HID Service UUID
static const uint8_t adv_data[] = {
    2, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,                  // LE General Discoverable, BR/EDR Not Supported
    3, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC4, 0x03,       // Gamepad (964 = 0x03C4, little-endian)
    7, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
        0x12, 0x18,   // HID Service (0x1812)
        0x0F, 0x18,   // Battery Service (0x180F)
        0x0A, 0x18,   // Device Information Service (0x180A)
};

// Scan response: complete local name
static uint8_t scan_resp_data[2 + sizeof(BLEHIDManager::DEVICE_NAME) - 1];

class BTstackLock {
public:
    BTstackLock() { async_context_acquire_lock_blocking(cyw43_arch_async_context()); }
    ~BTstackLock() { async_context_release_lock(cyw43_arch_async_context()); }
    BTstackLock(const BTstackLock&) = delete;
    BTstackLock& operator=(const BTstackLock&) = delete;
};

// Static TLV context removed — bonding database is now backed by protobuf config.

// BTstack event handler registrations
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

// BLE timing constants
static constexpr uint32_t INIT_RETRY_DELAY_MS = 5000;
static constexpr uint32_t BATTERY_UPDATE_INTERVAL_MS = 30000;
static constexpr uint32_t IDLE_TIMEOUT_MS = 30000;
static constexpr uint32_t REPORT_IDLE_THROTTLE_MS = 50;
static constexpr uint32_t STATUS_DUMP_INTERVAL_MS = 5000;
static constexpr uint32_t SEND_LOG_INTERVAL_MS = 2000;

// LED blink type codes for diagnostic and status signaling
static constexpr uint8_t PENDING_BLINK_REPORT_SENT = 1;
static constexpr uint8_t PENDING_BLINK_NOTIFICATIONS_DISABLED = 3;
static constexpr uint8_t PENDING_BLINK_NOTIFICATIONS_ENABLED = 5;

// Disconnect reason diagnostic blink parameters
static constexpr uint8_t DISCONNECT_REASON_BLINK_CAP = 15;
static constexpr uint32_t DISCONNECT_REASON_INITIAL_PAUSE_MS = 1000;
static constexpr uint32_t DISCONNECT_REASON_BLINK_MS = 300;

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void BLEHIDManager::init(bool configMode) {
    if (_initRequested) return;
    _initRequested = true;
    _configMode = configMode;
    _pairingMode = !configMode;
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

    cyw43_arch_poll();
    BTstackLock lock;
    if (!_ready) return;
    _saveBonds();

    // Periodic battery level reporting — throttled to once per 30 seconds.
    // Uses direct ADC read on GPIO29 (ADC3) via 3:1 voltage divider.
    // Only fires when connected and notifications are enabled.
    {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (_connected && _notificationsEnabled &&
                (now - _lastBatteryUpdateMs >= BATTERY_UPDATE_INTERVAL_MS || _lastBatteryLevel == 255)) {
            uint8_t level = _readBatteryPercent();
            if (level != _lastBatteryLevel) {
                battery_service_server_set_battery_value(level);
                _lastBatteryLevel = level;
            }
            _lastBatteryUpdateMs = now;
        }
    }

    // Power state management — transition ACTIVE → IDLE after 30s of no input change.
    if (_powerState == BLEPowerState::ACTIVE) {
        if ((now - _lastInputChangeMs) >= IDLE_TIMEOUT_MS) {
            _powerState = BLEPowerState::IDLE;
            // Optional connection parameter request for longer interval in idle:
            // gap_request_connection_parameter_update(_conHandle, 80, 80, 0, 200);
        }
    }

    // Deferred advertising restart — set by disconnect handler, executed here in main loop
    if (_needsAdvRestart && !_connected) {
        gap_advertisements_enable(_pairingMode ? 1 : 0);
        _advStarted = _pairingMode;
        _needsAdvRestart = false;
    }

    // Non-blocking LED status signals keep USB Web Config responsive.
    // _pendingBlinkType is set by the IRQ handler; we execute the blink here without sleep_ms().
    {
        static uint8_t         blinkRemaining = 0;
        static bool            blinkLedOn     = false;
        static uint32_t        blinkOnMs      = 200;
        static uint32_t        blinkOffMs     = 200;
        static absolute_time_t blinkNext      = {0};

        if (_pendingBlinkType != 0) {
            uint8_t blinkType = _pendingBlinkType;
            _pendingBlinkType = 0;
            if (blinkType == PENDING_BLINK_NOTIFICATIONS_ENABLED) {
                blinkOnMs = 200; blinkOffMs = 200; blinkRemaining = 10;
            } else if (blinkType == PENDING_BLINK_NOTIFICATIONS_DISABLED) {
                blinkOnMs = 300; blinkOffMs = 300; blinkRemaining = 6;
            } else if (blinkType == PENDING_BLINK_REPORT_SENT) {
                blinkOnMs = 50;  blinkOffMs = 50;  blinkRemaining = 2;
            }
            blinkLedOn = true;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            blinkNext  = make_timeout_time_ms(blinkOnMs);
        }

        if (blinkRemaining > 0 &&
                absolute_time_diff_us(blinkNext, get_absolute_time()) >= 0) {
            blinkRemaining--;
            blinkLedOn = !blinkLedOn;
            if (blinkRemaining == 0) {
                // Sequence complete — always end with LED off
                blinkLedOn = false;
            }
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blinkLedOn ? 1 : 0);
            if (blinkRemaining > 0) {
                blinkNext = make_timeout_time_ms(blinkLedOn ? blinkOnMs : blinkOffMs);
            }
        }
    }

    // Blink out the HCI disconnect reason code (N slow blinks = reason code value, capped at 15)
    // Common codes: 8=timeout, 0x13=remote terminated, 0x16=local terminated, 0x3B=bad params
    if (_lastDisconnectReason != 0 && _advStarted && !_connected) {
        static uint8_t diagBlinkCount = 0;
        static absolute_time_t diagBlinkNext = {0};
        static bool diagBlinkInit = false;
        if (!diagBlinkInit) {
            diagBlinkCount = (_lastDisconnectReason > DISCONNECT_REASON_BLINK_CAP) ?
                    DISCONNECT_REASON_BLINK_CAP : _lastDisconnectReason;
            diagBlinkNext = make_timeout_time_ms(DISCONNECT_REASON_INITIAL_PAUSE_MS);
            diagBlinkInit = true;
        }
        if (diagBlinkCount > 0 && absolute_time_diff_us(diagBlinkNext, get_absolute_time()) >= 0) {
            static bool diagLedOn = false;
            diagLedOn = !diagLedOn;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, diagLedOn ? 1 : 0);
            diagBlinkNext = make_timeout_time_ms(DISCONNECT_REASON_BLINK_MS);
            if (!diagLedOn) diagBlinkCount--;
            if (diagBlinkCount == 0) {
                _lastDisconnectReason = 0;
                diagBlinkInit = false;
            }
        }
    }

    if (_reportPending && _connected && _notificationsEnabled) {
        hids_device_request_can_send_now_event(_conHandle);
    }

    // Periodic UART status dump (every 5 s)
    {
        static uint32_t lastDumpMs = 0;
        if (now - lastDumpMs >= STATUS_DUMP_INTERVAL_MS) {
            lastDumpMs = now;
            printf("[BLE] status: connected=%d notif=%d power=%d bonds=%d handle=0x%04X\n",
                   (int)_connected, (int)_notificationsEnabled,
                   (int)_powerState, (int)_hasBondedPeers, (unsigned)_conHandle);
        }
    }
}

bool BLEHIDManager::sendReport(const uint8_t* report, uint16_t len) {
    if (!_initialized) return false;
    BTstackLock lock;
    if (!_connected || !_notificationsEnabled) {
        // Periodic "blocked" log — avoids flooding at frame rate
        static uint32_t lastBlockLogMs = 0;
        uint32_t now3 = to_ms_since_boot(get_absolute_time());
        if (now3 - lastBlockLogMs >= SEND_LOG_INTERVAL_MS) {
            lastBlockLogMs = now3;
            printf("[BLE] sendReport blocked: connected=%d notif=%d\n",
                   (int)_connected, (int)_notificationsEnabled);
        }
        return false;
    }
    // Periodic "flowing" log
    {
        static uint32_t lastSendMs = 0;
        uint32_t now4 = to_ms_since_boot(get_absolute_time());
        if (now4 - lastSendMs >= SEND_LOG_INTERVAL_MS) {
            lastSendMs = now4;
            printf("[BLE] sendReport queued len=%u\n", (unsigned)len);
        }
    }

    // In IDLE state, throttle report submission to ~50ms to reduce power consumption.
    if (_powerState == BLEPowerState::IDLE) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - _lastReportMs) < REPORT_IDLE_THROTTLE_MS) return false;
    }

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
    _lastReportMs     = to_ms_since_boot(get_absolute_time());
    return true;
}

BLEHIDStatus BLEHIDManager::getStatus() {
    if (!_initialized) {
        return {false, _initFailed, _bondSaveFailed, false, false, false,
                static_cast<uint8_t>(le_device_db_count())};
    }
    BTstackLock lock;
    return {_ready, _initFailed, _bondSaveFailed, _connected, _notificationsEnabled, _pairingMode,
            static_cast<uint8_t>(le_device_db_count())};
}

bool BLEHIDManager::setPairingMode(bool enabled) {
    if (!_initialized) return false;
    BTstackLock lock;
    if (!_ready || _connected) return false;
    _pairingMode = enabled;
    _needsAdvRestart = false;
    gap_advertisements_enable(enabled ? 1 : 0);
    _advStarted = enabled;
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
}

void BLEHIDManager::_handleDisconnect(uint8_t reason) {
    printf("[BLE] Disconnected reason=0x%02X\n", (unsigned)reason);
    _lastDisconnectReason = reason;
    _resetReportQueue();
    _connected            = false;
    _notificationsEnabled = false;
    _conHandle            = HCI_CON_HANDLE_INVALID;
    _advStarted           = false;
    _powerState           = BLEPowerState::ADVERTISING;
    _lastInputChangeMs    = 0;
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
    // Initialize CYW43 wireless chip
    int err = cyw43_arch_init();
    if (err != 0) {
        printf("[BLE] CYW43 initialization failed: %d\n", err);
        _initFailed  = true;
        _retryTimeMs = to_ms_since_boot(get_absolute_time());
        return;
    }

    BTstackLock lock;

    // Initialize BTstack run loop integrated with the CYW43 async context
    btstack_run_loop_init(btstack_run_loop_async_context_get_instance(cyw43_arch_async_context()));

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

    // Battery ADC init — GPIO29/ADC3 is the onboard voltage divider on boards that define it.
    // Safe to call after cyw43_arch_init(); the CYW43 SPI and ADC reads coexist on GPIO29.
#ifdef BATTERY_ADC_GPIO
    adc_init();
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

    // Set up advertising parameters and data (adv_type = 0 = ADV_IND, undirected connectable)
    gap_advertisements_set_params(0x0030, 0x0060, 0, 0, NULL, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), (uint8_t*)adv_data);
    scan_resp_data[0] = sizeof(scan_resp_data) - 1;
    scan_resp_data[1] = BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(scan_resp_data + 2, DEVICE_NAME, sizeof(scan_resp_data) - 2);
    gap_scan_response_set_data(sizeof(scan_resp_data), scan_resp_data);

    // Power on the Bluetooth controller — advertising starts ONLY after
    // BTSTACK_EVENT_STATE / HCI_STATE_WORKING fires (see _hciPacketHandler)
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
                // Advertising starts ONLY here — not before hci_power_control returns
                if (mgr._pairingMode && !mgr._advStarted) {
                    gap_advertisements_enable(1);
                    mgr._advStarted = true;
                }
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            uint8_t reason = hci_event_disconnection_complete_get_reason(packet);
            mgr._handleDisconnect(reason);
            break;
        }

        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) ==
                    HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                if (hci_subevent_le_connection_complete_get_status(packet) != ERROR_CODE_SUCCESS) {
                    mgr._needsAdvRestart = true;
                    break;
                }
                if (mgr._configMode && !mgr._pairingMode) {
                    gap_disconnect(hci_subevent_le_connection_complete_get_connection_handle(packet));
                    break;
                }
                mgr._lastDisconnectReason = 0; // clear on new connection
                mgr._conHandle  = hci_subevent_le_connection_complete_get_connection_handle(packet);
                mgr._connected  = true;
                mgr._advStarted = false;
                if (mgr._configMode) mgr._pairingMode = false;
                printf("[BLE] Connected handle=0x%04X\n", (unsigned)mgr._conHandle);
            }
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    {
                        uint8_t enable = hids_subevent_input_report_enable_get_enable(packet);
                        mgr._notificationsEnabled = (enable != 0);
                        printf("[BLE] INPUT_REPORT_ENABLE enable=%u\n", (unsigned)enable);
                        // Set flag to blink LED in process() — don't block IRQ handler with sleep_ms
                        mgr._pendingBlinkType = (enable != 0) ?
                            PENDING_BLINK_NOTIFICATIONS_ENABLED :
                            PENDING_BLINK_NOTIFICATIONS_DISABLED;
                        if (enable != 0) {
                            mgr._powerState        = BLEPowerState::ACTIVE;
                            mgr._lastInputChangeMs = to_ms_since_boot(get_absolute_time());
                        }
                    }
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    if (mgr._reportPending && mgr._reportQueueCount > 0) {
                        uint8_t readIndex = mgr._reportQueueHead;
                        uint16_t reportLen = mgr._reportQueueLen[readIndex];

                        hids_device_send_input_report(mgr._conHandle,
                                                      mgr._reportQueue[readIndex],
                                                      reportLen);
                        // Detect input change for IDLE → ACTIVE transition.
                        // Periodic log: first send + every 2 s
                        {
                            static uint32_t lastSendLogMs = 0;
                            static bool firstSend = true;
                            uint32_t t = to_ms_since_boot(get_absolute_time());
                            if (firstSend || t - lastSendLogMs >= 2000) {
                                firstSend    = false;
                                lastSendLogMs = t;
                                printf("[BLE] CAN_SEND_NOW: sent %u bytes"
                                       " btns=[0x%02X 0x%02X 0x%02X]"
                                       " axes=[%u %u %u %u]\n",
                                        (unsigned)reportLen,
                                       mgr._reportQueue[readIndex][0], mgr._reportQueue[readIndex][1],
                                       mgr._reportQueue[readIndex][2],
                                       mgr._reportQueue[readIndex][3], mgr._reportQueue[readIndex][4],
                                       mgr._reportQueue[readIndex][5], mgr._reportQueue[readIndex][6]);
                            }
                        }
                        // _lastSentReport is only accessed here (IRQ context) so no volatile needed.
                        if (memcmp(mgr._lastSentReport, mgr._reportQueue[readIndex], reportLen) != 0) {
                            memset(mgr._lastSentReport, 0, BLEHIDManager::REPORT_SIZE_BYTES);
                            memcpy(mgr._lastSentReport, mgr._reportQueue[readIndex], reportLen);
                            mgr._lastInputChangeMs = to_ms_since_boot(get_absolute_time());
                            if (mgr._powerState == BLEPowerState::IDLE) {
                                mgr._powerState = BLEPowerState::ACTIVE;
                                // Optional: request shorter connection interval for gaming latency:
                                // gap_request_connection_parameter_update(mgr._conHandle, 6, 6, 0, 200);
                            }
                        }
                        mgr._reportQueueHead = (mgr._reportQueueHead + 1) % BLEHIDManager::REPORT_QUEUE_DEPTH;
                        mgr._reportQueueCount--;
                        mgr._reportPending = (mgr._reportQueueCount > 0);
                        // Don't blink per-report — reports fire at HID frame rate,
                        // continuously restarting the blink sequence keeps the LED solid.
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
            if (mgr._configMode && !mgr._connected && !mgr._pairingMode) {
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
            mgr._hasBondedPeers = true;
            break;
        case SM_EVENT_IDENTITY_RESOLVING_FAILED: {
            // New peers and non-resolvable addresses must not erase other saved devices.
            mgr._hasBondedPeers = (le_device_db_count() > 0);
            break;
        }
        case SM_EVENT_PAIRING_COMPLETE: {
            uint8_t status = sm_event_pairing_complete_get_status(packet);
            if (status == ERROR_CODE_SUCCESS) {
                mgr._hasBondedPeers = true;
                // Re-verify bond was stored — 2 fast blinks = pairing+bond confirmed
                mgr._pendingBlinkType = PENDING_BLINK_REPORT_SENT;
            }
            // On failure: do nothing. Bond was not stored. Advertising will restart
            // on disconnect and the host can try again.
            break;
        }
        default:
            break;
    }
}
