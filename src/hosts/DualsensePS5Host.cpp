#include "hosts/DualsensePS5Host.h"

bool DualsensePS5Host::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Playstation 5 Dualsense
    if ( vendor_id == 0x054c) {
        switch(product_id) {
            case 0x0CE6:               // DualSense
                return true;
        }
    }
    
    return false;
}

void DualsensePS5Host::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
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

    memset(&prevReport, 0, sizeof(P5GenerorReport));
}

void DualsensePS5Host::process(uint8_t const* report, uint16_t len) {
    P5GenerorReport controller_report;
    if (report[0] == 1) {
        memcpy(&controller_report, report, sizeof(controller_report));

        if ( prevReport.reportCounter != controller_report.reportCounter ) {
            _controller_host_state.lx = map(controller_report.left_stick_x, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ly = map(controller_report.left_stick_y, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.rx = map(controller_report.right_stick_x,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ry = map(controller_report.right_stick_y,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.lt = controller_report.left_trigger;
            _controller_host_state.rt = controller_report.right_trigger;

            _controller_host_state.buttons = 0;
            if (controller_report.button_touchpad) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
            if (controller_report.button_select) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
            if (controller_report.button_r3) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
            if (controller_report.button_l3) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
            if (controller_report.button_home) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
            if (controller_report.button_start) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
            if (controller_report.button_r1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
            if (controller_report.button_l1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
            if (controller_report.button_north) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
            if (controller_report.button_east) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
            if (controller_report.button_south) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
            if (controller_report.button_west) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
            if (controller_report.button_r2) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
            if (controller_report.button_l2) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

            _controller_host_state.dpad = 0;
            if (controller_report.dpad == P5GENERAL_HAT_UP) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
            if (controller_report.dpad == P5GENERAL_HAT_UPRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == P5GENERAL_HAT_RIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == P5GENERAL_HAT_DOWNRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == P5GENERAL_HAT_DOWN) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == P5GENERAL_HAT_DOWNLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == P5GENERAL_HAT_LEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == P5GENERAL_HAT_UPLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
        }
    }

    memcpy(&prevReport, &controller_report, sizeof(P5GenerorReport));
}

void DualsensePS5Host::gamepad(Gamepad * gamepad) {
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

uint32_t DualsensePS5Host::map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
