#include "hosts/DrivingForceHost.h"

#include "tusb_config.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "device/usbd_pvt.h"

bool DrivingForceHost::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    if ( vendor_id == 0x46d) {
        switch(product_id) {
            case 0xC294: // compatible mode
            case 0xC29A:
                return true;
        }
    }
    return false;
}

void DrivingForceHost::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Setup our vars
    _dev_addr = dev_addr;
    _instance = instance;
    _vendor_id = vendor_id;
    _product_id = product_id;

    _controller_host_state.buttons = 0;
    _controller_host_state.dpad = 0;
    _controller_host_state.lx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ly = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.rx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ry = GAMEPAD_JOYSTICK_MID;

    if ( vendor_id == 0x46d && product_id == 0xC294 ) {
        // send commands to see if can be reset to Driving Force GT mode for more compatibility
        uint8_t command[8] = {0xF8, 0x09, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00};
        uint16_t commandSize = sizeof(command);
        tuh_hid_send_report(dev_addr, instance, 0, command, commandSize);
    }
}

void DrivingForceHost::process(uint8_t const* report, uint16_t len) {
    PS3ReportAlt ps3Report;
    memcpy(&ps3Report, report, len);
#ifdef GAMEPAD_HOST_DEBUG
    //printf("\033[2;0H");
    //for (uint8_t i = 0; i < len; i++) {
    //    printf("%02x ", report[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("\033[2;0H Ste:%5d Gas:%3d Brk:%3d", ps3Report.wheel.steeringWheel, ps3Report.wheel.gasPedal, ps3Report.wheel.brakePedal);
    //printf("\n-----\n");
    //printf("\033[2;0HDPad: %1d", ps3Report.wheel.dpadDirection);
    //printf("\033[3;0HWheel: %5d", ps3Report.wheel.steeringWheel);
    //printf("\033[4;0HGas: %3d", ps3Report.wheel.gasPedal);
    //printf("\033[5;0HBrake: %3d", ps3Report.wheel.brakePedal);
#endif
/*
    ps3Report.wheel.buttonNorth;
    ps3Report.wheel.buttonEast;
    ps3Report.wheel.buttonSouth;
    ps3Report.wheel.buttonWest;

    ps3Report.wheel.buttonL2;
    ps3Report.wheel.buttonR2;
    ps3Report.wheel.buttonL1;
    ps3Report.wheel.buttonR1;

    ps3Report.wheel.buttonStart;
    ps3Report.wheel.buttonSelect;
    ps3Report.wheel.buttonL3;
    ps3Report.wheel.buttonR3;

    ps3Report.wheel.buttonPS;
    ps3Report.wheel.buttonTP;
    ps3Report.wheel.dpadDirection;

    ps3Report.wheel.steeringWheel;
    ps3Report.wheel.brakePedal;
    ps3Report.wheel.gasPedal;
    ps3Report.wheel.clutchPedal;
*/
}

void DrivingForceHost::gamepad(Gamepad * gamepad) {
    // Override the Gamepad
    gamepad->hasAnalogTriggers   = true;
    gamepad->hasLeftAnalogStick  = true;
    gamepad->hasRightAnalogStick = true;
    gamepad->state.dpad     |= _controller_host_state.dpad;
    gamepad->state.buttons  |= _controller_host_state.buttons;
    gamepad->state.lx       = _controller_host_state.lx;
    gamepad->state.ly       = _controller_host_state.ly;
    gamepad->state.rx       = _controller_host_state.rx;
    gamepad->state.ry       = _controller_host_state.ry;
    gamepad->state.rt       = _controller_host_state.rt;
    gamepad->state.lt       = _controller_host_state.lt;
}