#ifndef _GamepadUSBHostListener_H
#define _GamepadUSBHostListener_H

#include "usblistener.h"
#include "gamepad.h"
#include "host/usbh.h"

#include "gphost.h"

// Move to individuals
//#define GAMEPAD_HOST_DEBUG 1

const uint32_t GAMEPAD_HOST_POLL_INTERVAL_MS = 3;

// Add other controller structs here
class GamepadUSBHostListener : public USBListener {
    public:// USB Listener Features
        virtual void setup();
        virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
        virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
        virtual void unmount(uint8_t dev_addr);
        virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
        virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
        virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
        virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
        void update();
    private:
        uint8_t _controller_dev_addr;
        uint8_t _controller_instance;
        GPHost * _controller_host;
        uint32_t _next_update;
};

#endif
