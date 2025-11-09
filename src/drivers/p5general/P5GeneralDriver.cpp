#include "drivers/p5general/P5GeneralDriver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

#include "drivers/p5general/P5GeneralAuth.h"
#include "enums.pb.h"

#define P5GENERAL_KEEPALIVE_US                          5000

#define P5GENERAL_DRIVER_PRINTF_ENABLE                  0       // GP0 as UART0_TX
#if P5GENERAL_DRIVER_PRINTF_ENABLE
#   define P5DRPINTF_INIT(...)                          stdio_init_all(__VA_ARGS__)
#   define P5DRPINTF(...)                               printf(__VA_ARGS__)
#else
#   define P5DRPINTF_INIT(...)
#   define P5DRPINTF(...)
#endif

static constexpr uint8_t output_0x03[] = {
    0x21, 0x28, 0x03, 0xC3, 0x00, 0x2C, 0x56,
    0x01, 0x00, 0xD0, 0x07, 0x00, 0x80, 0x04, 0x00,
    0x00, 0x80, 0x0D, 0x0D, 0x84, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void P5GeneralDriver::initialize() {
    P5DRPINTF_INIT();
    P5DRPINTF("P5D:initialize\n");

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    // init feature data
    touchpadData.p1.unpressed = 1;
    touchpadData.p1.set_x(P5GENERAL_TP_X_MAX / 2);
    touchpadData.p1.set_y(P5GENERAL_TP_Y_MAX / 2);
    touchpadData.p2.unpressed = 1;
    touchpadData.p2.set_x(P5GENERAL_TP_X_MAX / 2);
    touchpadData.p2.set_y(P5GENERAL_TP_Y_MAX / 2);

    // preseed touchpad sensors with center position values
    gamepad->auxState.sensors.touchpad[0].x = P5GENERAL_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[0].y = P5GENERAL_TP_Y_MAX/2;
    gamepad->auxState.sensors.touchpad[1].x = P5GENERAL_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[1].y = P5GENERAL_TP_Y_MAX/2;
    
    touchCounter = 0;
    diff_report_repeat = 0;

    p5GeneralReport = {
        .report_id = 0x01,
        .left_stick_x = P5GENERAL_JOYSTICK_MID,
        .left_stick_y = P5GENERAL_JOYSTICK_MID,
        .right_stick_x = P5GENERAL_JOYSTICK_MID,
        .right_stick_y = P5GENERAL_JOYSTICK_MID,
        .left_trigger = 0,
        .right_trigger = 0,
        .dpad = 0x08,
        .button_west = 0, .button_south = 0, .button_east = 0, .button_north = 0,
        .button_l1 = 0, .button_r1 = 0, .button_l2 = 0, .button_r2 = 0,
        .button_select = 0, .button_start = 0, .button_l3 = 0, .button_r3 = 0,
        .button_home = 0, .button_touchpad = 0,
        .data_30_31_0x001a = 0x001a,
        .touchpad_data = touchpadData,
    };

    class_driver = 	{
#if CFG_TUSB_DEBUG >= 2
        .name = "P5GENERAL",
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

void P5GeneralDriver::initializeAux() {
    P5DRPINTF_INIT();
    P5DRPINTF("P5D:initializeAux\n");

    p5GeneralAuthDriver = new P5GeneralAuth();
    if ( p5GeneralAuthDriver != nullptr && p5GeneralAuthDriver->available() ) {
        p5GeneralAuthDriver->initialize();
        p5GeneralAuthData = p5GeneralAuthDriver->getAuthData();
    }

    if (p5GeneralAuthData) {
        P5DRPINTF("P5D:p5GeneralAuthData Get\n");
    } else {
        P5DRPINTF("P5D:p5GeneralAuthData Null\n");
    }
}

bool P5GeneralDriver::getDongleAuthRequired() {
    P5DRPINTF("P5D:getDongleAuthRequired\n");
    return true;
}

bool P5GeneralDriver::process(Gamepad * gamepad) {
    if (!p5GeneralAuthData || !p5GeneralAuthData->dongle_ready) {
        return false;
    }
    
    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    if (p5GeneralAuthData->hash_ready) {
        if (tud_hid_ready() && tud_hid_report(0, p5GeneralAuthData->hash_finish_buffer, sizeof(p5GeneralAuthData->hash_finish_buffer)) == true ) {
            last_report_us = to_us_since_boot(get_absolute_time());
            p5GeneralAuthData->hash_ready = false;
        } else {
            return false;
        }
    }

    if (p5GeneralAuthData->hash_pending) {
        return false;
    }

    // update gamepad
    const GamepadOptions & options = gamepad->getOptions();
    switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
    {
        case GAMEPAD_MASK_UP:                        p5GeneralReport.dpad = P5GENERAL_HAT_UP;        break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   p5GeneralReport.dpad = P5GENERAL_HAT_UPRIGHT;   break;
        case GAMEPAD_MASK_RIGHT:                     p5GeneralReport.dpad = P5GENERAL_HAT_RIGHT;     break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: p5GeneralReport.dpad = P5GENERAL_HAT_DOWNRIGHT; break;
        case GAMEPAD_MASK_DOWN:                      p5GeneralReport.dpad = P5GENERAL_HAT_DOWN;      break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  p5GeneralReport.dpad = P5GENERAL_HAT_DOWNLEFT;  break;
        case GAMEPAD_MASK_LEFT:                      p5GeneralReport.dpad = P5GENERAL_HAT_LEFT;      break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    p5GeneralReport.dpad = P5GENERAL_HAT_UPLEFT;    break;
        default:                                     p5GeneralReport.dpad = P5GENERAL_HAT_NOTHING;   break;
    }
    bool anyA2A3A4 = gamepad->pressedA2() || gamepad->pressedA3() || gamepad->pressedA4();
    p5GeneralReport.button_south    = gamepad->pressedB1();
    p5GeneralReport.button_east     = gamepad->pressedB2();
    p5GeneralReport.button_west     = gamepad->pressedB3();
    p5GeneralReport.button_north    = gamepad->pressedB4();
    p5GeneralReport.button_l1       = gamepad->pressedL1();
    p5GeneralReport.button_r1       = gamepad->pressedR1();
    p5GeneralReport.button_l2       = gamepad->pressedL2();
    p5GeneralReport.button_r2       = gamepad->pressedR2();
    p5GeneralReport.button_select   = options.switchTpShareForDs4 ? anyA2A3A4 : gamepad->pressedS1();
    p5GeneralReport.button_start    = gamepad->pressedS2();
    p5GeneralReport.button_l3       = gamepad->pressedL3();
    p5GeneralReport.button_r3       = gamepad->pressedR3();
    p5GeneralReport.button_home     = gamepad->pressedA1();
    p5GeneralReport.button_touchpad = options.switchTpShareForDs4 ? gamepad->pressedS1() : anyA2A3A4;
    p5GeneralReport.left_stick_x = static_cast<uint8_t>(gamepad->state.lx >> 8);
    p5GeneralReport.left_stick_y = static_cast<uint8_t>(gamepad->state.ly >> 8);
    p5GeneralReport.right_stick_x = static_cast<uint8_t>(gamepad->state.rx >> 8);
    p5GeneralReport.right_stick_y = static_cast<uint8_t>(gamepad->state.ry >> 8);
    if (gamepad->hasAnalogTriggers) {
        p5GeneralReport.left_trigger = gamepad->state.lt;
        p5GeneralReport.right_trigger = gamepad->state.rt;
    } else {
        p5GeneralReport.left_trigger = gamepad->pressedL2() ? 0xFF : 0;
        p5GeneralReport.right_trigger = gamepad->pressedR2() ? 0xFF : 0;
    }

    // gyroscope
    if (gamepad->auxState.sensors.gyroscope.enabled) {
        p5GeneralReport.gyroscope.x = ((gamepad->auxState.sensors.gyroscope.x & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.x & 0xFF00) >> 8);
        p5GeneralReport.gyroscope.y = ((gamepad->auxState.sensors.gyroscope.y & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.y & 0xFF00) >> 8);
        p5GeneralReport.gyroscope.z = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
    }

    // accelerometer
    if (gamepad->auxState.sensors.accelerometer.enabled) {
        p5GeneralReport.accelerometer.x = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
        p5GeneralReport.accelerometer.y = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
        p5GeneralReport.accelerometer.z = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
    }

    // if the touchpad is pressed (note A2 vs. S1 choice above), emulate one finger of the touchpad
    touchpadData.p1.unpressed = p5GeneralReport.button_touchpad ? 0 : 1;
    if (p5GeneralReport.button_touchpad) {
        // make the assumption that since touchpad button is already being pressed, 
        // the first touch position is in use and no other "touches" will be present
        if (gamepad->pressedA3()) {
            touchpadData.p1.set_x(P5GENERAL_TP_X_MIN);
        } else if (gamepad->pressedA4()) {
            touchpadData.p1.set_x(P5GENERAL_TP_X_MAX);
        } else {
            touchpadData.p1.set_x(P5GENERAL_TP_X_MAX / 2);
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
        touchCounter = (touchCounter < P5GENERAL_TP_MAX_COUNT ? touchCounter+1 : 0);

        touchpadData.p1.counter = touchCounter;

        pointOneTouched = true;
    } else if (pointOneTouched && touchpadData.p1.unpressed) {
        pointOneTouched = false;
    }
    if (!pointTwoTouched && !touchpadData.p2.unpressed) {
        touchCounter = (touchCounter < P5GENERAL_TP_MAX_COUNT ? touchCounter+1 : 0);
    
        touchpadData.p2.counter = touchCounter;
    
        pointTwoTouched = true;
    } else if (pointTwoTouched && touchpadData.p2.unpressed) {
        pointTwoTouched = false;
    }
    p5GeneralReport.touchpad_data = touchpadData;

    if (memcmp(&p5GeneralReport_last, &p5GeneralReport, sizeof(p5GeneralReport))) {
        memcpy(&p5GeneralReport_last, &p5GeneralReport, sizeof(p5GeneralReport));
        memcpy(p5GeneralAuthData->hash_pending_buffer, &p5GeneralReport, sizeof(p5GeneralReport));
        p5GeneralAuthData->hash_pending = true;
        diff_report_repeat = 4;
        return true;
    } else if (diff_report_repeat) {
        diff_report_repeat--;
        memcpy(p5GeneralAuthData->hash_pending_buffer, &p5GeneralReport, sizeof(p5GeneralReport));
        p5GeneralAuthData->hash_pending = true;
        return true;
    } else {
        return false;
    }
}

void P5GeneralDriver::processAux() {
    if ( p5GeneralAuthDriver != nullptr && p5GeneralAuthDriver->available() ) {
        p5GeneralAuthDriver->process();
    }
}

USBListener * P5GeneralDriver::get_usb_auth_listener() {
    if ( p5GeneralAuthDriver != nullptr ) {
        P5DRPINTF("P5D:get_usb_auth_listener Call getListener\n");
        return p5GeneralAuthDriver->getListener();
    }
    P5DRPINTF("P5D:get_usb_auth_listener NULL\n");
    return nullptr;
}

uint16_t P5GeneralDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    P5DRPINTF("P5D:get_report %d\n", report_type);

    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        return -1;
    }

    uint16_t responseLen = 0;
    switch(report_id) {
    case P5GeneralAuthReport::P5GENERAL_DEFINITION:
        if (reqlen < sizeof(output_0x03)) {
            return -1;
        }
        responseLen = MAX(reqlen, sizeof(output_0x03));
        memcpy(buffer, output_0x03, responseLen);
        return responseLen;
    case P5GeneralAuthReport::P5GENERAL_GET_SIGNATURE_NONCE:
        memcpy(buffer, p5GeneralAuthData->auth_buffer + 1, 63);
        if (p5GeneralAuthData->passthrough_state == p5g_auth_idle) {
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f1;
        }
        return 63;
    case P5GeneralAuthReport::P5GENERAL_GET_SIGNING_STATE:
        memcpy(buffer, p5GeneralAuthData->auth_buffer + 1, 15);
        if (p5GeneralAuthData->passthrough_state == p5g_auth_idle) {
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f1;
        }
        return 15;
    }
    return -1;
}

void P5GeneralDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    P5DRPINTF("P5D:set_report %d size %d\n", report_type, bufsize);

    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        return;
    }

    if (report_id == P5GeneralAuthReport::P5GENERAL_SET_AUTH_PAYLOAD) {
        if (bufsize != 63) {
            return;
        }
        if (p5GeneralAuthData->passthrough_state == p5g_auth_idle) {
            p5GeneralAuthData->auth_buffer[0] = report_id;
            memcpy(p5GeneralAuthData->auth_buffer + 1, buffer, bufsize);
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_send_f0;
        }
    }
}

const uint16_t * P5GeneralDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const char *value = (const char *)p5general_string_descriptors[index];
    P5DRPINTF("P5D:get_descriptor_string_cb Index %d. langid %d, value %x\n", index, langid, (uint32_t)value);
    return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * P5GeneralDriver::get_descriptor_device_cb() {
    P5DRPINTF("P5D:p5general_device_descriptor\n");
    return p5general_device_descriptor;
}

const uint8_t * P5GeneralDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    P5DRPINTF("P5D:p5general_report_descriptor\n");
    return p5general_report_descriptor;
}

const uint8_t * P5GeneralDriver::get_descriptor_configuration_cb(uint8_t index) {
    P5DRPINTF("P5D:get_descriptor_configuration_cb\n");
    return p5general_configuration_descriptor;
}

uint16_t P5GeneralDriver::GetJoystickMidValue() {
    return P5GENERAL_JOYSTICK_MID << 8;
}
