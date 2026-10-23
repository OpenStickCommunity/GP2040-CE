#include "hosts/Xbox360Host.h"
#include "tusb_config.h"
#include "tusb.h"
#include "drivers/shared/xinput_host.h"
#include <storagemanager.h>
#include <algorithm>

bool Xbox360Host::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t controller_type) {    
    if ( controller_type == 1 ) {    
        // Microsoft
        if ( vendor_id == 0x045E ) {
            switch(product_id) {
                case 0x028E: // Xbox 360 Controller
                    return true;
            }
        }

        // 8BitDo
        if ( vendor_id == 0x2DC8 ) {
            switch(product_id) {
                case 0x3106: // 8BitDo Ultimate Wireless / 2.4G Receiver (XInput mode)
                case 0x310B: // 8BitDo Ultimate 2 Wireless / 2.4G Receiver (XInput mode)
                    return true;
            }
        }
    }

    // Xbox 360 wireless receiver (Microsoft 045E:0719 and clones), matched by interface protocol
    if ( controller_type == XBOX360_WIRELESS ) {
        return true;
    }
    return false;
}

void Xbox360Host::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Setup our vars
    _dev_addr = dev_addr;
    _instance = instance;
    _vendor_id = vendor_id;
    _product_id = product_id;

    reset_state();

    last_left_rumble = 0;
    last_right_rumble = 0;

    _wireless = (tuh_xinput_type(dev_addr, instance) == XBOX360_WIRELESS);
    _wireless_connected = false;
    _wireless_connect_pending = false;

    if (_wireless) {
        // The receiver answers with a presence packet if a controller is already synced
        wireless_send(XBOX360_WIRELESS_INQUIRE_PRESENT, sizeof(XBOX360_WIRELESS_INQUIRE_PRESENT));
        tuh_xinput_receive_report(_dev_addr, _instance);
        return;
    }

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

void Xbox360Host::reset_state() {
    _controller_host_state.buttons = 0;
    _controller_host_state.dpad = 0;
    _controller_host_state.lx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ly = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.rx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ry = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.lt = 0;
    _controller_host_state.rt = 0;

    memset(&prev_report, 0, sizeof(XInputReport));
}

void Xbox360Host::update() {
    // Wireless controller just synced: enable rumble and light up player 1 (sent here, not from the report callback)
    if (_wireless && _wireless_connect_pending) {
        _wireless_connect_pending = false;
        wireless_send(XBOX360_WIRELESS_RUMBLE_ENABLE, sizeof(XBOX360_WIRELESS_RUMBLE_ENABLE));
        xbox360_set_led(1);
        last_left_rumble = 0;
        last_right_rumble = 0;
    }
    if (_wireless && !_wireless_connected) {
        return;
    }

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
    if (_wireless) {
        process_wireless(report, len);
        return;
    }

    XInputReport controller_report;

    if (len < sizeof(XInputReport)) {
#ifdef GAMEPAD_HOST_DEBUG
        printf("Xbox 360 report too small: %d bytes\n", len);
#endif
        return;
    }

    memcpy(&controller_report, report, sizeof(controller_report));
    process_report(controller_report);
}

void Xbox360Host::process_wireless(uint8_t const* report, uint16_t len) {
    if (len < 2) return;

    // Presence packet: byte 1 bit 7 = controller synced, 0x00 = controller gone
    if (report[0] & 0x08) {
        if ((report[1] & 0x80) && !_wireless_connected) {
            _wireless_connected = true;
            _wireless_connect_pending = true;
        } else if (report[1] == 0x00 && _wireless_connected) {
            _wireless_connected = false;
            reset_state();
        }
    }

    // Input packet: standard XInput report starting at byte 4
    if ((report[1] & 0x01) && len >= 18 && (report[5] == 0x13 || report[5] == 0x14)) {
        XInputReport controller_report;
        memset(&controller_report, 0, sizeof(controller_report));
        memcpy(&controller_report, report + 4, std::min<size_t>(len - 4, sizeof(controller_report)));
        process_report(controller_report);
    }
}

void Xbox360Host::process_report(XInputReport const& controller_report) {
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

    int32_t inv_ly = (controller_report.ly == INT16_MIN) ? 32767 : -controller_report.ly;
    int32_t inv_ry = (controller_report.ry == INT16_MIN) ? 32767 : -controller_report.ry;

    _controller_host_state.lx = static_cast<uint16_t>(controller_report.lx - INT16_MIN);
    _controller_host_state.ly = static_cast<uint16_t>(inv_ly - INT16_MIN);
    _controller_host_state.rx = static_cast<uint16_t>(controller_report.rx - INT16_MIN);
    _controller_host_state.ry = static_cast<uint16_t>(inv_ry - INT16_MIN);

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

bool Xbox360Host::wireless_send(uint8_t const* cmd, uint16_t cmd_len) {
    uint8_t out[XBOX360_WIRELESS_PACKET_SIZE] = { 0 };
    memcpy(out, cmd, cmd_len);
    tuh_xinput_wait_for_tx(_dev_addr, _instance);
    bool ret = tuh_xinput_send_report(_dev_addr, _instance, out, sizeof(out));
    if (ret) {
        tuh_xinput_wait_for_tx(_dev_addr, _instance);
    }
    return ret;
}

void Xbox360Host::xbox360_set_led(uint8_t quadrant) {
    if (_wireless) {
        // 0x40 + 6..9 = player 1..4 LED solid on
        uint8_t out[sizeof(XBOX360_WIRELESS_LED)];
        memcpy(out, XBOX360_WIRELESS_LED, sizeof(XBOX360_WIRELESS_LED));
        out[3] += (quadrant == 0) ? 0 : (quadrant + 5);
        wireless_send(out, sizeof(out));
        return;
    }

    uint8_t out[32] = { 0 };

    memcpy(out, XBOX360_WIRED_LED, sizeof(XBOX360_WIRED_LED));
    out[2] = (quadrant == 0) ? 0 : (quadrant + 5);
    bool ret = tuh_xinput_send_report(_dev_addr, _instance, out, sizeof(XBOX360_WIRED_LED));
    if (ret) {
        tuh_xinput_wait_for_tx(_dev_addr, _instance);
    }
}

void Xbox360Host::xinput_set_rumble(uint8_t left, uint8_t right) {
    if (_wireless) {
        uint8_t out[sizeof(XBOX360_WIRELESS_RUMBLE)];
        memcpy(out, XBOX360_WIRELESS_RUMBLE, sizeof(XBOX360_WIRELESS_RUMBLE));
        out[5] = left;
        out[6] = right;
        wireless_send(out, sizeof(out));
        return;
    }

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
