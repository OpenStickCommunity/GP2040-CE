#include "hosts/Xbox360Host.h"
#include "drivers/shared/xinput_host.h"
#include <storagemanager.h>

bool Xbox360Host::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t controller_type) {    
    if ( controller_type == 1 ) {    
        if ( vendor_id == 0x045E ) {
            switch(product_id) {
                case 0x028E:
                    return true;
            }
        }
    }
    return false;
}

void Xbox360Host::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
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

    last_left_rumble = 0;
    last_right_rumble = 0;

    memset(&prev_report, 0, sizeof(XInputReport));

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    
    uint32_t quadrants = gamepad->auxState.playerID.value;
    if (quadrants == 0)
        quadrants = 1;
    for (uint32_t i = 0; i < quadrants; i++) {
        xbox360_set_led(i);
    }

    xinput_set_rumble(0, 0);
    tuh_xinput_receive_report(_dev_addr, _instance);
}

void Xbox360Host::update() {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

    // rumble
    gamepad->auxState.haptics.leftActuator.enabled = 1;
    gamepad->auxState.haptics.rightActuator.enabled = 1;
    uint8_t leftRumble = 0;
    uint8_t rightRumble = 0;
    if (gamepad->auxState.haptics.leftActuator.active) {
        leftRumble = gamepad->auxState.haptics.leftActuator.intensity;
    }
    if (gamepad->auxState.haptics.rightActuator.active) {
        rightRumble = gamepad->auxState.haptics.rightActuator.intensity;
    }

    if (leftRumble == last_left_rumble && rightRumble == last_right_rumble) {
        return; // no change
    }
    last_left_rumble = leftRumble;
    last_right_rumble = rightRumble;

    xinput_set_rumble(leftRumble, rightRumble);
}

void Xbox360Host::process(uint8_t const* report, uint16_t len) {
    XInputReport controller_report;

   

    if (len < sizeof(XInputReport)) {
#if GAMEPAD_HOST_DEBUG
        printf("Xbox 360 report too small: %d bytes\n", len);
#endif
        return;
    }

    memcpy(&controller_report, report, sizeof(controller_report));

    if (memcmp(&prev_report, &controller_report, sizeof(XInputReport)) == 0)
        return;

    _controller_host_state.dpad = 0;
    if (controller_report.buttons1 & XBOX_MASK_UP) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
    if (controller_report.buttons1 & XBOX_MASK_DOWN) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
    if (controller_report.buttons1 & XBOX_MASK_LEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
    if (controller_report.buttons1 & XBOX_MASK_RIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;

    _controller_host_state.buttons = 0;
    if (controller_report.buttons1 & XBOX_MASK_START) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
    if (controller_report.buttons1 & XBOX_MASK_BACK) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
    if (controller_report.buttons1 & XBOX_MASK_LS) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
    if (controller_report.buttons1 & XBOX_MASK_RS) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
    if (controller_report.buttons2 & XBOX_MASK_LB) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.buttons2 & XBOX_MASK_RB) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.buttons2 & XBOX_MASK_HOME) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
    if (controller_report.buttons2 & XBOX_MASK_A) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.buttons2 & XBOX_MASK_B) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.buttons2 & XBOX_MASK_X) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.buttons2 & XBOX_MASK_Y) _controller_host_state.buttons |= GAMEPAD_MASK_B4;

    _controller_host_state.lx = static_cast<uint16_t>(controller_report.lx - INT16_MIN);
    _controller_host_state.ly = ~static_cast<uint16_t>(controller_report.ly - INT16_MIN);
    _controller_host_state.rx = static_cast<uint16_t>(controller_report.rx - INT16_MIN);
    _controller_host_state.ry = ~static_cast<uint16_t>(controller_report.ry - INT16_MIN);

    _controller_host_state.lt = controller_report.lt;
    _controller_host_state.rt = controller_report.rt;

    memcpy(&prev_report, &controller_report, sizeof(XInputReport));
}


void Xbox360Host::gamepad(Gamepad * gamepad) {
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

void Xbox360Host::xbox360_set_led(uint8_t quadrant) {
    uint8_t out[32] = { 0 };

    memcpy(out, XBOX360_WIRED_LED, sizeof(XBOX360_WIRED_LED));
    out[2] = (quadrant == 0) ? 0 : (quadrant + 5);
    bool ret = tuh_xinput_send_report(_dev_addr, _instance, out, sizeof(XBOX360_WIRED_LED));
    if (ret) {
        tuh_xinput_wait_for_tx(_dev_addr, _instance);
    }
}

void Xbox360Host::xinput_set_rumble(uint8_t left, uint8_t right) {
    uint8_t out[32];
    memset(out, 0, 32);
    uint16_t len = sizeof(XBOX360_WIRED_RUMBLE);
    memcpy(out, XBOX360_WIRED_RUMBLE, sizeof(XBOX360_WIRED_RUMBLE));
    out[3] = left;
    out[4] = right;
    tuh_xinput_wait_for_tx(_dev_addr, _instance);
    bool ret = tuh_xinput_send_report(_dev_addr, _instance, out, len);
    if (ret) {
        tuh_xinput_wait_for_tx(_dev_addr, _instance);
    }
}
