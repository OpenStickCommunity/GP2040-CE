#ifndef _GamepadUSBHostListener_H
#define _GamepadUSBHostListener_H

#include "usblistener.h"
#include "gamepad.h"
#include "host/usbh.h"
#include "drivers/ps4/PS4Descriptors.h"
#include "drivers/switchpro/SwitchProDescriptors.h"

#define GAMEPAD_HOST_DEBUG false
#define GAMEPAD_HOST_USE_FEATURES true

const uint32_t GAMEPAD_HOST_POLL_INTERVAL_MS = 3;

// Google Stadia controller report struct
typedef struct TU_ATTR_PACKED
{
    uint8_t  reportId;                                 // Report ID = 0x03 (3)
                                                        // Collection: CA:GamePad
    int8_t   GD_GamePadHatSwitch : 4;                  // Usage 0x00010039: Hat switch, Value =  to 7, Physical = Value x 45 in degrees
    int8_t   : 1;                                      // Pad
    int8_t   : 1;                                      // Pad
    int8_t   : 1;                                      // Pad
    int8_t   : 1;                                      // Pad
    uint8_t  BTN_GamePadButton18 : 1;                  // Usage 0x00090012: Button 18, Value = 0 to 1                :: Capture button
    uint8_t  BTN_GamePadButton17 : 1;                  // Usage 0x00090011: Button 17, Value = 0 to 1                :: Google assistant
    uint8_t  BTN_GamePadButton20 : 1;                  // Usage 0x00090014: Button 20, Value = 0 to 1                :: L2
    uint8_t  BTN_GamePadButton19 : 1;                  // Usage 0x00090013: Button 19, Value = 0 to 1                :: R2
    uint8_t  BTN_GamePadButton13 : 1;                  // Usage 0x0009000D: Button 13, Value = 0 to 1                :: Stadia button
    uint8_t  BTN_GamePadButton12 : 1;                  // Usage 0x0009000C: Button 12, Value = 0 to 1                :: menu button
    uint8_t  BTN_GamePadButton11 : 1;                  // Usage 0x0009000B: Button 11, Value = 0 to 1                :: options
    uint8_t  BTN_GamePadButton15 : 1;                  // Usage 0x0009000F: Button 15, Value = 0 to 1                :: R3
    uint8_t  BTN_GamePadButton14 : 1;                  // Usage 0x0009000E: Button 14, Value = 0 to 1                :: L3
    uint8_t  BTN_GamePadButton8 : 1;                   // Usage 0x00090008: Button 8, Value = 0 to 1                 :: R1
    uint8_t  BTN_GamePadButton7 : 1;                   // Usage 0x00090007: Button 7, Value = 0 to 1                 :: L1
    uint8_t  BTN_GamePadButton5 : 1;                   // Usage 0x00090005: Button 5, Value = 0 to 1                 :: Y button
    uint8_t  BTN_GamePadButton4 : 1;                   // Usage 0x00090004: Button 4, Value = 0 to 1                 :: X button
    uint8_t  BTN_GamePadButton2 : 1;                   // Usage 0x00090002: Button 2 Secondary, Value = 0 to 1       :: B button
    uint8_t  BTN_GamePadButton1 : 1;                   // Usage 0x00090001: Button 1 Primary/trigger, Value = 0 to 1 :: A button
    uint8_t  : 1;                                      // Pad
                                                        // Collection: CA:GamePad CP:Pointer
    uint8_t  GD_GamePadPointerX;                       // Usage 0x00010030: X, Value = 1 to 255
    uint8_t  GD_GamePadPointerY;                       // Usage 0x00010031: Y, Value = 1 to 255
    uint8_t  GD_GamePadPointerZ;                       // Usage 0x00010032: Z, Value = 1 to 255
    uint8_t  GD_GamePadPointerRz;                      // Usage 0x00010035: Rz, Value = 1 to 255
                                                        // Collection: CA:GamePad
    uint8_t  SIM_GamePadBrake;                         // Usage 0x000200C5: Brake, Value = 0 to 255
    uint8_t  SIM_GamePadAccelerator;                   // Usage 0x000200C4: Accelerator, Value = 0 to 255

} google_stadia_report_t;

// Ultrastik 360
typedef struct TU_ATTR_PACKED
{
    uint8_t GD_GamePadPointerX;
    uint8_t GD_GamePadPointerY;

    struct {
        uint8_t BTN_GamePadButton1 : 1;
        uint8_t BTN_GamePadButton2 : 1;
        uint8_t BTN_GamePadButton3 : 1;
        uint8_t BTN_GamePadButton4 : 1;
        uint8_t BTN_GamePadButton5 : 1;
        uint8_t BTN_GamePadButton6 : 1;
        uint8_t BTN_GamePadButton7 : 1;
        uint8_t BTN_GamePadButton8 : 1;
        uint8_t BTN_GamePadButton9 : 1;
        uint8_t BTN_GamePadButton10 : 1;
        uint8_t BTN_GamePadButton11 : 1;
        uint8_t BTN_GamePadButton12 : 1;
        uint8_t BTN_GamePadButton13 : 1;
        uint8_t BTN_GamePadButton14 : 1;
        uint8_t BTN_GamePadButton15 : 1;
        uint8_t padding : 1;
    };

} ultrastik360_t;

typedef struct __attribute__((packed)) {
    uint8_t reportID;
    uint8_t leftStickX;
    uint8_t leftStickY;
    uint8_t rightStickX;
    uint8_t rightStickY;
    uint8_t leftTrigger;
    uint8_t rightTrigger;

    // 8 bit report counter.
    uint8_t reportCounter;

    // 4 bits for the d-pad.
    uint8_t dpad : 4;

    // 14 bits for buttons.
    uint16_t buttonWest : 1;
    uint16_t buttonSouth : 1;
    uint16_t buttonEast : 1;
    uint16_t buttonNorth : 1;
    uint16_t buttonL1 : 1;
    uint16_t buttonR1 : 1;
    uint16_t buttonL2 : 1;
    uint16_t buttonR2 : 1;
    uint16_t buttonSelect : 1;
    uint16_t buttonStart : 1;
    uint16_t buttonL3 : 1;
    uint16_t buttonR3 : 1;
    uint16_t buttonHome : 1;
    uint16_t buttonTouchpad : 1;
    uint16_t buttonMicMute : 1;

    uint8_t miscData[54];
} DSReport;

typedef struct __attribute__((packed)) {
    uint8_t command;
    uint8_t counter;
    uint8_t rumble_l[4];
    uint8_t rumble_r[4];
    uint8_t subcommand;
    uint8_t subcommand_args[3];
} SwitchProHostReport;

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
        void process();
    private:
        bool awaiting_cb = false;
        bool host_get_report(uint8_t report_id, void* report, uint16_t len);
        bool host_set_report(uint8_t report_id, void* report, uint16_t len);

        GamepadState _controller_host_state;
        bool _controller_host_analog = true;
        bool _controller_host_enabled;
        uint8_t _controller_dev_addr = 0;
        uint8_t _controller_instance = 0;
        uint8_t _controller_type = 0; // 0 = HID, 1 = Xbox 360, 2 = Xbox One (see xinput_type_t)
        uint32_t _next_update = 0;
        // reliable update (e.g for rumble) called every GAMEPAD_HOST_POLL_INTERVAL_MS ms
        void update_ctrlr();
        void process_ctrlr_report(uint8_t dev_addr, uint8_t const* report, uint16_t len);

        // Controller report processor functions
        bool isDS4Identified = false;
        bool hasDS4DefReport = false;
        // ds4 initialization step, if needed
        void init_ds4(const uint8_t* descReport, uint16_t descLen);
        // general ds4 setup
        void setup_ds4();
        // update ds4 output reporting
        void update_ds4();
        // handle ds4 input reporting
        void process_ds4(uint8_t const* report, uint16_t len);
        PS4ControllerConfig ds4Config;
        uint8_t report_buffer[PS4_ENDPOINT_SIZE];

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
};

#endif
