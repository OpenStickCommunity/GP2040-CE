#include "drivers/ps5/PS5Driver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

#include "drivers/ps5/PS5Auth.h"
#include "enums.pb.h"

#define PS5_KEEPALIVE_US                          5000

#define PS5_DRIVER_PRINTF_ENABLE                  0       // GP0 as UART0_TX
#if PS5_DRIVER_PRINTF_ENABLE
#   define P5DPRINTF_INIT(...)                          stdio_init_all(__VA_ARGS__)
#   define P5DPRINTF(...)                               printf(__VA_ARGS__)
#else
#   define P5DPRINTF_INIT(...)
#   define P5DPRINTF(...)
#endif

// 0x03 (Feature Info)
static constexpr uint8_t output_0x03[] = {
    0x21, 0x28, 0x03, 0xC3, 0x00, 0x2C, 0x56,
    0x01, 0x00, 0xD0, 0x07, 0x00, 0x80, 0x04, 0x00,
    0x00, 0x80, 0x0D, 0x0D, 0x84, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// 0x05 (Sensor Calibration)
static constexpr uint8_t output_0x05[] = {
    0x05, 0xff, 0xff, 0xf4, 0xff, 0xfb, 0xff, 0x92,
    0x22, 0x6a, 0xdd, 0x8d, 0x22, 0x5d, 0xdd, 0x9b,
    0x22, 0x65, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0xd2,
    0x1f, 0xf2, 0xdf, 0xd0, 0x1f, 0xb7, 0xdf, 0x04,
    0x20, 0xfc, 0xdf, 0x05, 0x00, 0x00, 0x00, 0x00,
    0x00
};

// 0x20 (Firmware Controller Revision Jun 24 2024 11:16:21)
static constexpr uint8_t output_0x20[] = {
    0x20, 0x4a, 0x75, 0x6e, 0x20, 0x32, 0x34, 0x20,
    0x32, 0x30, 0x32, 0x34, 0x31, 0x31, 0x3a, 0x31,
    0x36, 0x3a, 0x32, 0x31, 0x03, 0x00, 0x04, 0x00,
    0x13, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01,
    0x41, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x20, 0x05, 0x00, 0x00,
    0x2a, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x02, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void PS5Driver::initialize() {
    P5DPRINTF_INIT();
    P5DPRINTF("P5D:initialize\n");

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    // init feature data
    touchpadData.p1.unpressed = 1;
    touchpadData.p1.set_x(PS5_TP_X_MAX / 2);
    touchpadData.p1.set_y(PS5_TP_Y_MAX / 2);
    touchpadData.p2.unpressed = 1;
    touchpadData.p2.set_x(PS5_TP_X_MAX / 2);
    touchpadData.p2.set_y(PS5_TP_Y_MAX / 2);

    // preseed touchpad sensors with center position values
    gamepad->auxState.sensors.touchpad[0].x = PS5_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[0].y = PS5_TP_Y_MAX/2;
    gamepad->auxState.sensors.touchpad[1].x = PS5_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[1].y = PS5_TP_Y_MAX/2;
    
    touchCounter = 0;
    diff_report_repeat = 0;

    ps5Report = {
        .report_id = 0x01,
        .left_stick_x = PS5_JOYSTICK_MID,
        .left_stick_y = PS5_JOYSTICK_MID,
        .right_stick_x = PS5_JOYSTICK_MID,
        .right_stick_y = PS5_JOYSTICK_MID,
        .left_trigger = 0,
        .right_trigger = 0,
        .dpad = 0x08,
        .button_west = 0, .button_south = 0, .button_east = 0, .button_north = 0,
        .button_l1 = 0, .button_r1 = 0, .button_l2 = 0, .button_r2 = 0,
        .button_select = 0, .button_start = 0, .button_l3 = 0, .button_r3 = 0,
        .button_home = 0, .button_touchpad = 0,
        .sensor_timestamp = 0x001a0000,
        .touchpad_data = touchpadData,
    };

    class_driver = 	{
#if CFG_TUSB_DEBUG >= 2
        .name = "PS5",
#endif
        .init = hidd_init,
        .reset = hidd_reset,
        .open = hidd_open,
        .control_xfer_cb = hidd_control_xfer_cb,
        .xfer_cb = hidd_xfer_cb,
        .sof = NULL
    };

    last_report_us = to_us_since_boot(get_absolute_time());
}

void PS5Driver::initializeAux() {
    P5DPRINTF_INIT();
    P5DPRINTF("P5D:initializeAux\n");

    ps5AuthDriver = new PS5Auth();
    if ( ps5AuthDriver != nullptr && ps5AuthDriver->available() ) {
        ps5AuthDriver->initialize();
        ps5AuthData = ps5AuthDriver->getAuthData();
    }

    if (ps5AuthData) {
        P5DPRINTF("P5D:ps5AuthData Get\n");
    } else {
        P5DPRINTF("P5D:ps5AuthData Null\n");
    }
}

bool PS5Driver::getDongleAuthRequired() {
    P5DPRINTF("P5D:getDongleAuthRequired\n");
    return true;
}

bool PS5Driver::process(Gamepad * gamepad) {
    if (!ps5AuthData || !ps5AuthData->dongle_ready) {
        return false;
    }
    
    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    // P5General uses hash_ready, can we use for Mayflash S5?
    if (ps5AuthData->hash_ready) {
        if ( mutex_enter_timeout_us(&ps5AuthData->hash_mutex, 500) ) {
            if (tud_hid_ready() && tud_hid_report(0, ps5AuthData->hash_finish_buffer, sizeof(ps5AuthData->hash_finish_buffer)) == true ) {
                mutex_exit(&ps5AuthData->hash_mutex);
                last_report_us = to_us_since_boot(get_absolute_time());
                ps5AuthData->hash_ready = false;
            } else {
                mutex_exit(&ps5AuthData->hash_mutex);
                return false;
            }
        }
    }

    if (ps5AuthData->hash_pending) {
        return false;
    }

    // update gamepad
    const GamepadOptions & options = gamepad->getOptions();
    switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
    {
        case GAMEPAD_MASK_UP:                        ps5Report.dpad = PS5_HAT_UP;        break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps5Report.dpad = PS5_HAT_UPRIGHT;   break;
        case GAMEPAD_MASK_RIGHT:                     ps5Report.dpad = PS5_HAT_RIGHT;     break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps5Report.dpad = PS5_HAT_DOWNRIGHT; break;
        case GAMEPAD_MASK_DOWN:                      ps5Report.dpad = PS5_HAT_DOWN;      break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps5Report.dpad = PS5_HAT_DOWNLEFT;  break;
        case GAMEPAD_MASK_LEFT:                      ps5Report.dpad = PS5_HAT_LEFT;      break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps5Report.dpad = PS5_HAT_UPLEFT;    break;
        default:                                     ps5Report.dpad = PS5_HAT_NOTHING;   break;
    }
    bool anyA2A3A4 = gamepad->pressedA2() || gamepad->pressedA3() || gamepad->pressedA4();
    ps5Report.button_south    = gamepad->pressedB1();
    ps5Report.button_east     = gamepad->pressedB2();
    ps5Report.button_west     = gamepad->pressedB3();
    ps5Report.button_north    = gamepad->pressedB4();
    ps5Report.button_l1       = gamepad->pressedL1();
    ps5Report.button_r1       = gamepad->pressedR1();
    ps5Report.button_l2       = gamepad->pressedL2();
    ps5Report.button_r2       = gamepad->pressedR2();
    ps5Report.button_select   = options.switchTpShareForDs4 ? anyA2A3A4 : gamepad->pressedS1();
    ps5Report.button_start    = gamepad->pressedS2();
    ps5Report.button_l3       = gamepad->pressedL3();
    ps5Report.button_r3       = gamepad->pressedR3();
    ps5Report.button_home     = gamepad->pressedA1();
    ps5Report.button_touchpad = options.switchTpShareForDs4 ? gamepad->pressedS1() : anyA2A3A4;
    ps5Report.left_stick_x = static_cast<uint8_t>(gamepad->state.lx >> 8);
    ps5Report.left_stick_y = static_cast<uint8_t>(gamepad->state.ly >> 8);
    ps5Report.right_stick_x = static_cast<uint8_t>(gamepad->state.rx >> 8);
    ps5Report.right_stick_y = static_cast<uint8_t>(gamepad->state.ry >> 8);
    if (gamepad->hasAnalogTriggers) {
        ps5Report.left_trigger = gamepad->state.lt;
        ps5Report.right_trigger = gamepad->state.rt;
    } else {
        ps5Report.left_trigger = gamepad->pressedL2() ? 0xFF : 0;
        ps5Report.right_trigger = gamepad->pressedR2() ? 0xFF : 0;
    }

    // gyroscope
    if (gamepad->auxState.sensors.gyroscope.enabled) {
        ps5Report.gyroscope.x = ((gamepad->auxState.sensors.gyroscope.x & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.x & 0xFF00) >> 8);
        ps5Report.gyroscope.y = ((gamepad->auxState.sensors.gyroscope.y & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.y & 0xFF00) >> 8);
        ps5Report.gyroscope.z = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
    }

    // accelerometer
    if (gamepad->auxState.sensors.accelerometer.enabled) {
        ps5Report.accelerometer.x = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
        ps5Report.accelerometer.y = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
        ps5Report.accelerometer.z = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
    }

    // if the touchpad is pressed (note A2 vs. S1 choice above), emulate one finger of the touchpad
    touchpadData.p1.unpressed = ps5Report.button_touchpad ? 0 : 1;
    if (ps5Report.button_touchpad) {
        // make the assumption that since touchpad button is already being pressed, 
        // the first touch position is in use and no other "touches" will be present
        if (gamepad->pressedA3()) {
            touchpadData.p1.set_x(PS5_TP_X_MIN);
        } else if (gamepad->pressedA4()) {
            touchpadData.p1.set_x(PS5_TP_X_MAX);
        } else {
            touchpadData.p1.set_x(PS5_TP_X_MAX / 2);
        }
    } else {
        // if more than one touch pad sensor, sensors will never be used out of order
        if (gamepad->auxState.sensors.touchpad[0].enabled) {
            touchpadData.p1.unpressed = !gamepad->auxState.sensors.touchpad[0].active;
            touchpadData.p1.set_x(gamepad->auxState.sensors.touchpad[0].x);
            touchpadData.p1.set_y(gamepad->auxState.sensors.touchpad[0].y);
            
            if (gamepad->auxState.sensors.touchpad[1].enabled) {
                touchpadData.p2.unpressed = !gamepad->auxState.sensors.touchpad[1].active;
                touchpadData.p2.set_x(gamepad->auxState.sensors.touchpad[1].x);
                touchpadData.p2.set_y(gamepad->auxState.sensors.touchpad[1].y);
            }
        }
    }
    // check if any of the points are recently touched, rather than still being touched
    if (!pointOneTouched && !touchpadData.p1.unpressed) {
        touchCounter = (touchCounter < PS5_TP_MAX_COUNT ? touchCounter+1 : 0);

        touchpadData.p1.counter = touchCounter;

        pointOneTouched = true;
    } else if (pointOneTouched && touchpadData.p1.unpressed) {
        pointOneTouched = false;
    }
    if (!pointTwoTouched && !touchpadData.p2.unpressed) {
        touchCounter = (touchCounter < PS5_TP_MAX_COUNT ? touchCounter+1 : 0);
    
        touchpadData.p2.counter = touchCounter;
    
        pointTwoTouched = true;
    } else if (pointTwoTouched && touchpadData.p2.unpressed) {
        pointTwoTouched = false;
    }
    ps5Report.touchpad_data = touchpadData;

    // Always copy our data for PS5 dongles
    memcpy(ps5AuthData->hash_pending_buffer, &ps5Report, sizeof(ps5Report));
    ps5AuthData->hash_pending = true;
    
    if (memcmp(&ps5Report_last, &ps5Report, sizeof(ps5Report))) {
        memcpy(&ps5Report_last, &ps5Report, sizeof(ps5Report));
        diff_report_repeat = 4;
        return true;
    } else if (diff_report_repeat) {
        diff_report_repeat--;
        return true;
    } else {
        return false;
    }

    /*
    if (memcmp(&ps5Report_last, &ps5Report, sizeof(ps5Report))) {
        memcpy(&ps5Report_last, &ps5Report, sizeof(ps5Report));
        memcpy(ps5AuthData->hash_pending_buffer, &ps5Report, sizeof(ps5Report));
        ps5AuthData->hash_pending = true;
        diff_report_repeat = 4;
        return true;
    } else if (diff_report_repeat) {
        diff_report_repeat--;
        memcpy(ps5AuthData->hash_pending_buffer, &ps5Report, sizeof(ps5Report));
        ps5AuthData->hash_pending = true;
        return true;
    } else {
        return false;
    }
    */
}

void PS5Driver::processAux() {
    if ( ps5AuthDriver != nullptr && ps5AuthDriver->available() ) {
        ps5AuthDriver->process();
    }
}

USBListener * PS5Driver::get_usb_auth_listener() {
    if ( ps5AuthDriver != nullptr ) {
        P5DPRINTF("P5D:get_usb_auth_listener Call getListener\n");
        return ps5AuthDriver->getListener();
    }
    P5DPRINTF("P5D:get_usb_auth_listener NULL\n");
    return nullptr;
}

uint16_t PS5Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    P5DPRINTF("P5D:get_report %d\n", report_type);

    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        return -1;
    }

    uint16_t responseLen = 0;
    switch(report_id) {
    case PS5AuthReport::PS5_DEFINITION:
        if (reqlen < sizeof(output_0x03)) {
            return -1;
        }
        responseLen = MAX(reqlen, sizeof(output_0x03));
        memcpy(buffer, output_0x03, responseLen);
        return responseLen;
    case PS5AuthReport::PS5_GET_CALIBRATION:
        if (reqlen < sizeof(output_0x05)) {
            return -1;
        }
        responseLen = MAX(reqlen, sizeof(output_0x05));
        memcpy(buffer, output_0x05, responseLen);
        return responseLen;
    case PS5AuthReport::PS5_GET_PAIRINFO:
        if ( ps5AuthData->pair_ready == false ||
            (reqlen < sizeof(ps5AuthData->MAC_pair_report))) {
            return -1;
        }
        responseLen = MAX(reqlen, sizeof(ps5AuthData->MAC_pair_report));
        memcpy(buffer, ps5AuthData->MAC_pair_report, responseLen);
        return responseLen;
    case PS5AuthReport::PS5_GET_FIRWMARE:
        if (reqlen < sizeof(output_0x20)) {
            return -1;
        }
        responseLen = MAX(reqlen, sizeof(output_0x20));
        memcpy(buffer, output_0x20, responseLen);
        return responseLen;
    case PS5AuthReport::PS5_GET_SIGNATURE_NONCE:
        memcpy(buffer, ps5AuthData->auth_buffer + 1, 63);
        if (ps5AuthData->ps5_passthrough_state == ps5_auth_idle) {
            ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f1;
        }
        return 63;
    case PS5AuthReport::PS5_GET_SIGNING_STATE:
        memcpy(buffer, ps5AuthData->auth_buffer + 1, 15);
        if (ps5AuthData->ps5_passthrough_state == ps5_auth_idle) {
            ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f1;
        }
        return 15;
    }
    return -1;
}

void PS5Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    P5DPRINTF("P5D:set_report %d size %d\n", report_type, bufsize);

    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        return;
    }

    if (report_id == PS5AuthReport::PS5_SET_AUTH_PAYLOAD) {
        if (bufsize != 63) {
            return;
        }
        if (ps5AuthData->ps5_passthrough_state == ps5_auth_idle) {
            ps5AuthData->auth_buffer[0] = report_id;
            memcpy(ps5AuthData->auth_buffer + 1, buffer, bufsize);
            ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_send_f0;
        }
    }
}

const uint16_t * PS5Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const char *value = (const char *)p5g_string_descriptors[index];
    P5DPRINTF("P5D:get_descriptor_string_cb Index %d. langid %d, value %x\n", index, langid, (uint32_t)value);
    return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PS5Driver::get_descriptor_device_cb() {
    P5DPRINTF("P5D:p5general_device_descriptor\n");
    return p5g_device_descriptor;
}

const uint8_t * PS5Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    P5DPRINTF("P5D:p5general_report_descriptor\n");
    return p5g_report_descriptor;
}

const uint8_t * PS5Driver::get_descriptor_configuration_cb(uint8_t index) {
    P5DPRINTF("P5D:get_descriptor_configuration_cb\n");
    return p5g_configuration_descriptor;
}

uint16_t PS5Driver::GetJoystickMidValue() {
    return PS5_JOYSTICK_MID << 8;
}
