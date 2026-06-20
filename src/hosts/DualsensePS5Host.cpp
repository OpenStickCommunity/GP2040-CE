#include "hosts/DualsensePS5Host.h"

void DualsensePS5Host::initialize() {
    requiresInit = false;
}

bool DualsensePS5Host::match(uint8_t vendor_id, uint8_t product_id) {
    // Playstation 5 Dualsense
    switch(product_id) {
        case 0x0CE6:               // DualSense
            return true;
        default:
            break;
    }
    
    return false;
}


void DualsensePS5Host::process(uint8_t const* report, uint16_t len) {
    DSReport controller_report;

    // previous report used to compare for changes
    static DSReport prev_ds_report = { 0 };

    uint8_t const report_id = report[0];

    if (report_id == 1) {
        memcpy(&controller_report, report, sizeof(controller_report));

        if ( prev_ds_report.reportCounter != controller_report.reportCounter ) {
            _controller_host_state.lx = map(controller_report.leftStickX, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ly = map(controller_report.leftStickY, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.rx = map(controller_report.rightStickX,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ry = map(controller_report.rightStickY,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.lt = controller_report.leftTrigger;
            _controller_host_state.rt = controller_report.rightTrigger;
            _controller_host_analog = true;

            _controller_host_state.buttons = 0;
            if (controller_report.buttonTouchpad) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
            if (controller_report.buttonSelect) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
            if (controller_report.buttonR3) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
            if (controller_report.buttonL3) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
            if (controller_report.buttonHome) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
            if (controller_report.buttonStart) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
            if (controller_report.buttonR1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
            if (controller_report.buttonL1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
            if (controller_report.buttonNorth) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
            if (controller_report.buttonEast) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
            if (controller_report.buttonSouth) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
            if (controller_report.buttonWest) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
            if (controller_report.buttonR2) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
            if (controller_report.buttonL2) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

            _controller_host_state.dpad = 0;
            if (controller_report.dpad == PS4_HAT_UP) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
            if (controller_report.dpad == PS4_HAT_UPRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_RIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_DOWNRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWN) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWNLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_LEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_UPLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
        }
    }

    prev_ds_report = controller_report;
}
