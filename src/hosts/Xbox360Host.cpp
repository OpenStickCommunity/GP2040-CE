#include "hosts/Xbox360Host.h"
#include "tusb_config.h"
#include "tusb.h"
#include "drivers/shared/xinput_host.h"
#include <storagemanager.h>

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
    rumble_dirty = false;
    left_rumble_start = 0;
    right_rumble_start = 0;
    left_kickstart_start = 0;
    right_kickstart_start = 0;
    left_kickstart_active = false;
    right_kickstart_active = false;
    left_rumble_active = false;
    right_rumble_active = false;
    sustained_left_rumble = 0;
    sustained_right_rumble = 0;

    memset(&prev_report, 0, sizeof(XInputReport));

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    
    uint32_t quadrants = gamepad->auxState.playerID.value;
    if (quadrants == 0)
        quadrants = 1;
    for (uint32_t i = 0; i < quadrants; i++) {
        xbox360_set_led(i);
    }

    rumble_dirty = !xinput_set_rumble(0, 0);
    tuh_xinput_receive_report(_dev_addr, _instance);
}

void Xbox360Host::update() {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    if (gamepad == nullptr) return;

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

    const GamepadOptions& options = Storage::getInstance().getGamepadOptions();
    const uint32_t rumble_mode = (options.ermRumbleMode > 2) ? 2 : options.ermRumbleMode;

    if (rumble_mode == 0) {
        leftRumble = 0;
        rightRumble = 0;
        left_rumble_active = false;
        right_rumble_active = false;
        left_kickstart_active = false;
        right_kickstart_active = false;
    } else {
        const uint32_t left_floor = (options.ermRumbleMotorFloor > 255) ? 255 : options.ermRumbleMotorFloor;
        const uint32_t left_kickstart_ms = (options.ermRumbleKickstartMs > 500) ? 500 : options.ermRumbleKickstartMs;
        const uint32_t left_sustain_ms = (options.ermRumbleSustainMs > 500) ? 500 : options.ermRumbleSustainMs;

        // In Frequency Crossover mode (2), the Right (Light / High-Frequency) motor uses its own
        // independent tuning parameters; in Dual / Symmetric mode (1), both motors share the primary parameters.
        const uint32_t right_floor = (rumble_mode == 2)
            ? ((options.ermRumbleRightMotorFloor > 255) ? 255 : options.ermRumbleRightMotorFloor)
            : left_floor;
        const uint32_t right_kickstart_ms = (rumble_mode == 2)
            ? ((options.ermRumbleRightKickstartMs > 500) ? 500 : options.ermRumbleRightKickstartMs)
            : left_kickstart_ms;
        const uint32_t right_sustain_ms = (rumble_mode == 2)
            ? ((options.ermRumbleRightSustainMs > 500) ? 500 : options.ermRumbleRightSustainMs)
            : left_sustain_ms;
        const uint32_t right_kickstart_boost = (rumble_mode == 2) ? 20 : 50;
        const uint32_t boost_percent = (options.ermRumbleBoostPercent > 100) ? 100 : options.ermRumbleBoostPercent;

        // 1. Motor Floor: map any non-zero intensity into [motor_floor .. 255]
        // to overcome physical static friction (stiction) of ERM motor brushes.
        if (left_floor > 0 && leftRumble > 0) {
            leftRumble = left_floor +
                         ((uint32_t)leftRumble * (255 - left_floor)) / 255;
        }
        if (right_floor > 0 && rightRumble > 0) {
            rightRumble = right_floor +
                          ((uint32_t)rightRumble * (255 - right_floor)) / 255;
        }

        // 2. Global Boost
        if (boost_percent > 0) {
            if (leftRumble > 0) {
                uint32_t boosted = (uint32_t)leftRumble * (100 + boost_percent) / 100;
                leftRumble = (boosted > 255) ? 255 : (uint8_t)boosted;
            }
            if (rightRumble > 0) {
                uint32_t boosted = (uint32_t)rightRumble * (100 + boost_percent) / 100;
                rightRumble = (boosted > 255) ? 255 : (uint8_t)boosted;
            }
        }

        const uint32_t now = to_ms_since_boot(get_absolute_time());

        // 3. Overdrive Kickstart + Pulse Sustain (wrap-safe elapsed time comparisons):
        // Left actuator sustain:
        if (left_sustain_ms > 0) {
            if (leftRumble > 0) {
                sustained_left_rumble = leftRumble;
                left_rumble_start = now;
                left_rumble_active = true;
            } else if (left_rumble_active) {
                if ((uint32_t)(now - left_rumble_start) < left_sustain_ms) {
                    leftRumble = sustained_left_rumble;
                } else {
                    left_rumble_active = false;
                }
            }
        }

        // Left actuator kickstart overdrive:
        if (left_kickstart_ms > 0) {
            if (leftRumble > 0) {
                if (last_left_rumble == 0) {
                    left_kickstart_start = now;
                    left_kickstart_active = true;
                }
                if (left_kickstart_active) {
                    if ((uint32_t)(now - left_kickstart_start) < left_kickstart_ms) {
                        uint32_t kick = (uint32_t)leftRumble + 50;
                        leftRumble = (kick > 255) ? 255 : (uint8_t)kick;
                    } else {
                        left_kickstart_active = false;
                    }
                }
            } else {
                left_kickstart_active = false;
            }
        }

        // Right actuator sustain:
        if (right_sustain_ms > 0) {
            if (rightRumble > 0) {
                sustained_right_rumble = rightRumble;
                right_rumble_start = now;
                right_rumble_active = true;
            } else if (right_rumble_active) {
                if ((uint32_t)(now - right_rumble_start) < right_sustain_ms) {
                    rightRumble = sustained_right_rumble;
                } else {
                    right_rumble_active = false;
                }
            }
        }

        // Right actuator kickstart overdrive:
        if (right_kickstart_ms > 0) {
            if (rightRumble > 0) {
                if (last_right_rumble == 0) {
                    right_kickstart_start = now;
                    right_kickstart_active = true;
                }
                if (right_kickstart_active) {
                    if ((uint32_t)(now - right_kickstart_start) < right_kickstart_ms) {
                        uint32_t kick = (uint32_t)rightRumble + right_kickstart_boost;
                        rightRumble = (kick > 255) ? 255 : (uint8_t)kick;
                    } else {
                        right_kickstart_active = false;
                    }
                }
            } else {
                right_kickstart_active = false;
            }
        }
    }

    if (!rumble_dirty && leftRumble == last_left_rumble && rightRumble == last_right_rumble) {
        return; // no change
    }

    if (xinput_set_rumble(leftRumble, rightRumble)) {
        last_left_rumble = leftRumble;
        last_right_rumble = rightRumble;
        rumble_dirty = false;
    } else {
        rumble_dirty = true;
    }
}

void Xbox360Host::process(uint8_t const* report, uint16_t len) {
    XInputReport controller_report;

   

    if (len < sizeof(XInputReport)) {
#ifdef GAMEPAD_HOST_DEBUG
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

void Xbox360Host::xbox360_set_led(uint8_t quadrant) {
    uint8_t out[32] = { 0 };

    memcpy(out, XBOX360_WIRED_LED, sizeof(XBOX360_WIRED_LED));
    out[2] = (quadrant == 0) ? 0 : (quadrant + 5);
    bool ret = tuh_xinput_send_report(_dev_addr, _instance, out, sizeof(XBOX360_WIRED_LED));
    if (ret) {
        tuh_xinput_wait_for_tx(_dev_addr, _instance);
    }
}

bool Xbox360Host::xinput_set_rumble(uint8_t left, uint8_t right) {
    if (!tuh_xinput_mounted(_dev_addr, _instance))
        return false;

    uint8_t out[32];
    memset(out, 0, 32);
    uint16_t len = sizeof(XBOX360_WIRED_RUMBLE);
    memcpy(out, XBOX360_WIRED_RUMBLE, sizeof(XBOX360_WIRED_RUMBLE));
    out[3] = left;
    out[4] = right;
    return tuh_xinput_send_report(_dev_addr, _instance, out, len);
}
