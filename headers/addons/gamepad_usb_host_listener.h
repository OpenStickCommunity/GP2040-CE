#ifndef _GamepadUSBHostListener_H
#define _GamepadUSBHostListener_H

#include "usblistener.h"
#include "gamepad.h"
#include "class/hid/hid.h"

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

// Dualshock 4
typedef struct TU_ATTR_PACKED
{
    uint8_t  reportId;

    uint8_t  GD_GamePadPointerX;                       // Usage 0x00010030: X, Value = 0 to 255
    uint8_t  GD_GamePadPointerY;                       // Usage 0x00010031: Y, Value = 0 to 255
    uint8_t  GD_GamePadPointerZ;                       // Usage 0x00010032: Z, Value = 0 to 255
    uint8_t  GD_GamePadPointerRz;                      // Usage 0x00010035: Rz, Value = 0 to 255

    struct
    {
        uint8_t  GD_GamePadHatSwitch : 4;                  // Usage 0x00010039: Hat switch, Value = 0 to 7, Physical = Value x 45 in degrees
        uint8_t  BTN_GamePadButton1 : 1;                   // Usage 0x00090001: Button 1 Primary/trigger, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton2 : 1;                   // Usage 0x00090002: Button 2 Secondary, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton3 : 1;                   // Usage 0x00090003: Button 3 Tertiary, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton4 : 1;                   // Usage 0x00090004: Button 4, Value = 0 to 1, Physical = Value x 315
    };

    struct
    {
        uint8_t  BTN_GamePadButton5 : 1;                   // Usage 0x00090005: Button 5, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton6 : 1;                   // Usage 0x00090006: Button 6, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton7 : 1;                   // Usage 0x00090007: Button 7, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton8 : 1;                   // Usage 0x00090008: Button 8, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton9 : 1;                   // Usage 0x00090009: Button 9, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton10 : 1;                  // Usage 0x0009000A: Button 10, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton11 : 1;                  // Usage 0x0009000B: Button 11, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton12 : 1;                  // Usage 0x0009000C: Button 12, Value = 0 to 1, Physical = Value x 315
    };

    struct
    {
        uint8_t  BTN_GamePadButton13 : 1;                  // Usage 0x0009000D: Button 13, Value = 0 to 1, Physical = Value x 315
        uint8_t  BTN_GamePadButton14 : 1;                  // Usage 0x0009000E: Button 14, Value = 0 to 1, Physical = Value x 315
        uint8_t  counter : 6;                              // Usage 0xFF000020: , Value = 0 to 127, Physical = Value x 315 / 127
    };

    uint8_t  SIM_GamePadBrake;                         // Usage 0x00010033: Rx, Value = 0 to 255, Physical = Value x 21 / 17
    uint8_t  SIM_GamePadAccelerator;                   // Usage 0x00010034: Ry, Value = 0 to 255, Physical = Value x 21 / 17
    uint8_t  VEN_GamePad0021[54];
} dualshock4_t;

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

// Add other controller structs here
class GamepadUSBHostListener : public USBListener {
    public:// USB Listener Features
        virtual void setup();
        virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
        virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {}
        virtual void unmount(uint8_t dev_addr);
        virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
        virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
        virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
        virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
        void process();
    private:
        GamepadState _controller_host_state;
        bool _controller_host_enabled;
        void process_ctrlr_report(uint8_t dev_addr, uint8_t const* report, uint16_t len);

        // Controller report processor functions
        void process_ds4(uint8_t const* report);
        void process_stadia(uint8_t const* report);
        void process_ultrastik360(uint8_t const* report);

        uint16_t controller_pid, controller_vid;

        uint16_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint16_t out_min, uint16_t out_max);

        // check if different than 2
        bool diff_than_2(uint8_t x, uint8_t y);
        // check if 2 reports are different enough
        bool diff_report(dualshock4_t const* rpt1, dualshock4_t const* rpt2);
};

#endif
