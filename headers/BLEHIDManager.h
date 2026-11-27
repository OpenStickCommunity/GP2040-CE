#ifndef BLE_HID_MANAGER_H
#define BLE_HID_MANAGER_H

// IMPORTANT: Do NOT include tusb.h or any TinyUSB header in this file.
// hid_report_type_t is defined by both TinyUSB and BTstack — including both
// in the same translation unit causes a compile error.

#include <stdint.h>
#include <stdbool.h>

struct BLEHIDStatus {
    bool ready;
    bool initFailed;
    bool connected;
    bool notifying;
    bool pairing;
    uint8_t bondCount;
};

class BLEHIDManager {
public:
    static constexpr char DEVICE_NAME[] = "GP2040-CE Gamepad";
    static BLEHIDManager& getInstance() {
        static BLEHIDManager instance;
        return instance;
    }

    BLEHIDManager(const BLEHIDManager&) = delete;
    BLEHIDManager& operator=(const BLEHIDManager&) = delete;

    // Called once at startup to record boot time. Does NOT call cyw43_arch_init.
    void init(bool configMode = false);

    // Called every main loop iteration.
    // Handles deferred radio initialization and BTstack maintenance.
    void process();

    // Queue a HID input report for transmission via BLE ATT notification.
    // Returns false if not connected or notifications not enabled.
    bool sendReport(const uint8_t* report, uint16_t len);

    // Enable or disable BLE pairing (advertising with general discoverability).
    bool setPairingMode(bool enabled);
    bool clearBonds();
    void clearStorageForFactoryReset();
    BLEHIDStatus getStatus();

    bool isConnected() const    { return _connected; }
    bool isEnabled() const      { return _ready; }
    bool isNotifying() const    { return _notificationsEnabled; }
    bool hasBondedPeers() const { return _hasBondedPeers; }
    uint8_t getBatteryLevel() const { return _lastBatteryLevel; }

private:
    // BLE HID gamepad report body (no Report ID byte):
    //   byte 0   : Buttons 1..8   (B1, B2, B3, B4, L1, R1, L2, R2)
    //   byte 1   : Buttons 9..16  (S1, S2, L3, R3, Up, Down, Left, Right)
    //   byte 2   : Buttons 17..18 + 6 pad bits (A1, A2, 0, 0, 0, 0, 0, 0)
    //   byte 3-8 : X, Y, Z, Rz sticks and Rx/Ry triggers (0..255)
    static constexpr uint8_t REPORT_SIZE_BYTES = 9;
    static constexpr uint8_t REPORT_QUEUE_DEPTH = 4;

    BLEHIDManager() = default;

    void _doInit();
    void _resetReportQueue();
    void _handleDisconnect(uint16_t connectionHandle);
    static uint8_t _readBatteryPercent();

    static void _hciPacketHandler(uint8_t packetType, uint16_t channel,
                                  uint8_t* packet, uint16_t size);
    static void _smPacketHandler(uint8_t packetType, uint16_t channel,
                                 uint8_t* packet, uint16_t size);

    bool     _initialized          = false;
    bool     _initRequested        = false;
    bool     _configMode           = false;
    volatile bool _ready           = false;
    volatile bool _advStarted      = false;
    volatile bool _pairingMode     = false;
    bool     _initFailed           = false;
    uint32_t _bootTimeMs           = 0;
    uint32_t _initDelayMs          = 3000;
    uint32_t _retryTimeMs          = 0;

    // Written by BTstack IRQ context (async_context_threadsafe_background) and
    // read by the main thread — must be volatile so the compiler does not cache
    // them in registers across loop iterations.
    volatile bool     _connected            = false;
    volatile bool     _notificationsEnabled = false;
    volatile bool     _reportPending        = false;
    volatile bool     _hasBondedPeers       = false;
    volatile bool     _needsAdvRestart      = false;
    volatile uint16_t _conHandle            = 0xFFFF;
    volatile uint32_t _lastBatteryUpdateMs  = 0;
    volatile uint8_t  _lastBatteryLevel     = 255;  // 255 = uninitialized → forces first update
    volatile uint8_t  _reportQueueHead      = 0;
    volatile uint8_t  _reportQueueTail      = 0;
    volatile uint8_t  _reportQueueCount     = 0;
    uint16_t _reportQueueLen[REPORT_QUEUE_DEPTH] = {};
    uint8_t  _reportQueue[REPORT_QUEUE_DEPTH][REPORT_SIZE_BYTES] = {};
    uint8_t  _lastSentReport[REPORT_SIZE_BYTES]   = {};  // previous payload; compared in CAN_SEND_NOW to detect changes
};

#endif // BLE_HID_MANAGER_H
