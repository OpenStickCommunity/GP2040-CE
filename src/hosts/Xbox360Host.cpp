#include "hosts/Xbox360Host.h"


void GamepadUSBHostListener::xbox360_set_led(uint8_t dev_addr, uint8_t instance, uint8_t quadrant) {
    uint8_t out[32] = { 0 };

    memcpy(out, XBOX360_WIRED_LED, sizeof(XBOX360_WIRED_LED));
    out[2] = (quadrant == 0) ? 0 : (quadrant + 5);
    bool ret = tuh_xinput_send_report(dev_addr, instance, out, sizeof(XBOX360_WIRED_LED));
    if (ret) {
        tuh_xinput_wait_for_tx(dev_addr, instance);
    }
}

void GamepadUSBHostListener::xinput_set_rumble(uint8_t dev_addr, uint8_t instance, uint8_t left, uint8_t right) {
    uint8_t out[32] = { 0 };
    uint16_t len = 0;
    switch (_controller_type) {
        case xinput_type_t::XBOX360: {
            memcpy(out, XBOX360_WIRED_RUMBLE, sizeof(XBOX360_WIRED_RUMBLE));
            out[3] = left;
            out[4] = right;
            len = sizeof(XBOX360_WIRED_RUMBLE);
            break;
        }
        case xinput_type_t::XBOXONE: {
            memcpy(out, XBOXONE_RUMBLE, sizeof(XBOXONE_RUMBLE));
            out[8] = left >> 1; // 7-bit (0-127)
            out[9] = right >> 1; // 7-bit (0-127)
            len = sizeof(XBOXONE_RUMBLE);
            break;
        }
        default:
            return;
    }
    tuh_xinput_wait_for_tx(dev_addr, instance);
    bool ret = tuh_xinput_send_report(dev_addr, instance, out, len);
    if (ret) {
        tuh_xinput_wait_for_tx(dev_addr, instance);
    }
}

void GamepadUSBHostListener::setup_xinput(uint8_t dev_addr, uint8_t instance) {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

    switch (_controller_type) {
        case xinput_type_t::XBOX360: {
            uint32_t quadrants = gamepad->auxState.playerID.value;
            if (quadrants == 0)
                quadrants = 1;
            for (uint32_t i = 0; i < quadrants; i++) {
                xbox360_set_led(dev_addr, instance, i);
            }

            xinput_set_rumble(dev_addr, instance, 0, 0);
            break;
        }
        case xinput_type_t::XBOXONE: {
            // implement xbox one init here... currently not supported
            break;
        }
        default: // unsupported
            break;
    }
    tuh_xinput_receive_report(dev_addr, instance);
}

void GamepadUSBHostListener::update_xinput(uint8_t dev_addr, uint8_t instance) {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

    static uint8_t last_left_rumble = 0;
    static uint8_t last_right_rumble = 0;

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

    xinput_set_rumble(dev_addr, instance, leftRumble, rightRumble);
}

void GamepadUSBHostListener::process_xbox360(uint8_t const* report, uint16_t len) {
    XInputReport controller_report;

    static XInputReport prev_report = { 0 };

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
    _controller_host_analog = true;

    prev_report = controller_report;
}
