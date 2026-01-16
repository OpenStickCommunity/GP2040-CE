#pragma once

#include "usblistener.h"
#include <cstdint>

// ===== Dual protocol =====

enum class DualCommand : uint8_t {
    DEVICE_CONNECTED       = 0x01,
    DEVICE_DISCONNECTED    = 0x02,
    REPORT_RECEIVED        = 0x03,
};

struct device_connected_t {
    uint8_t  command;
    uint16_t vid;
    uint16_t pid;
    uint8_t  dev_addr;
    uint8_t  interface;
    uint8_t  hub_port;
    uint8_t  itf_num;
    uint8_t  report_descriptor[];
};

struct device_disconnected_t {
    uint8_t command;
    uint8_t dev_addr;
    uint8_t interface;
};

struct report_received_t {
    uint8_t command;
    uint8_t dev_addr;
    uint8_t interface;
    uint8_t report[];
};

// ===== Listener =====

class DualUartListener : public USBListener {
public:
    void setup() override;
    void process();

private:
    bool handleMessage(const uint8_t* data, uint16_t len);

    // UART + SLIP
    bool serialRead();
};
