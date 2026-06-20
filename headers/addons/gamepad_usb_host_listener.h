#ifndef _GamepadUSBHostListener_H
#define _GamepadUSBHostListener_H

#include "usblistener.h"
#include "gamepad.h"
#include "host/usbh.h"
#include "drivers/ps4/PS4Descriptors.h"
#include "drivers/switchpro/SwitchProDescriptors.h"

#include "gphost.h"

// Move to individuals
#define GAMEPAD_HOST_DEBUG false

// Move to DS4 and Switch Pro
#define GAMEPAD_HOST_USE_FEATURES true

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
        void gamepadFeatureUpdate();
    private:
        bool _controller_host_mounted;
        uint8_t _controller_dev_addr;
        uint8_t _controller_instance;
        GPHost * _controller_host;
        uint32_t _next_update;

        /*
        //GamepadState _controller_host_state;
        //bool _controller_host_analog = true;

        //uint8_t _controller_type = 0; // 0 = HID, 1 = Xbox 360, 2 = Xbox One (see xinput_type_t)
        uint32_t _next_update = 0;
        // reliable update (e.g for rumble) called every GAMEPAD_HOST_POLL_INTERVAL_MS ms
        //void update_ctrlr();
        //void process_ctrlr_report(uint8_t dev_addr, uint8_t const* report, uint16_t len);

        void process_ds(uint8_t const* report, uint16_t len);

        // switch pro
        bool switchProFinished = false;
        uint8_t switchProState = SwitchOutputSubtypes::IDENTIFY;
        uint8_t switchReportCounter = 0;
        uint8_t lastSwitchLed = 0;
        void setup_switch_pro(uint8_t const *report, uint16_t len);
        void update_switch_pro();
        void process_switch_pro(uint8_t const* report, uint16_t len);
        uint8_t get_next_switch_counter();

        void process_stadia(uint8_t const* report, uint16_t len);

        void process_ultrastik360(uint8_t const* report, uint16_t len);

        void xbox360_set_led(uint8_t dev_addr, uint8_t instance, uint8_t quadrant);
        // universal xinput rumble packet
        void xinput_set_rumble(uint8_t dev_addr, uint8_t instance, uint8_t left, uint8_t right);
        // universal xinput setup (xbox one init not included)
        void setup_xinput(uint8_t dev_addr, uint8_t instance);
        // sync controller rumble and LED states
        void update_xinput(uint8_t dev_addr, uint8_t instance);
        void process_xbox360(uint8_t const* report, uint16_t len);

        uint16_t controller_pid, controller_vid;

        uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);

        // check if different than 2
        bool diff_than_2(uint8_t x, uint8_t y);
        // check if 2 reports are different enough
        bool diff_report(PS4Report const* rpt1, PS4Report const* rpt2);

        // wheel check
        bool isDFInit = false;
        void setup_df_wheel();
        void process_dfgt(uint8_t const* report, uint16_t len);
        */
};

#endif
