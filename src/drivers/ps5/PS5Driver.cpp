/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/PS5Driver.h"
#include "drivers/shared/driverhelper.h"
#include "peripheralmanager.h"
#include "storagemanager.h"

#define PS5_REPORT_REPEAT        4

#define PS5_STREAM_INTERVAL_US   (4ull * 1000ull)

// Fall back if the console starts auth but hasn't completed it in this long.
#define PS5_CONSOLE_AUTH_TIMEOUT_US (10000ull * 1000ull)

// Log when a bound dongle stops returning signed reports for this long.
#define PS5_SIGN_STALL_US        (1000ull * 1000ull)

// ---- Default feature reports (report ID stripped) ----

// 0x03 Controller definition (PS4ControllerConfig layout; byte 4 = controller type, 0 = gamepad)
static constexpr uint8_t output_0x03[] = {
    0x21, 0x28, 0x03, 0xC3, 0x00, 0x2C, 0x56,
    0x01, 0x00, 0xD0, 0x07, 0x00, 0x80, 0x04, 0x00,
    0x00, 0x80, 0x0D, 0x0D, 0x84, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// 0x05 Sensor calibration
static constexpr uint8_t output_0x05[] = {
    0xff, 0xff, 0xf4, 0xff, 0xfb, 0xff, 0x92, 0x22,
    0x6a, 0xdd, 0x8d, 0x22, 0x5d, 0xdd, 0x9b, 0x22,
    0x65, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0xd2, 0x1f,
    0xf2, 0xdf, 0xd0, 0x1f, 0xb7, 0xdf, 0x04, 0x20,
    0xfc, 0xdf, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 0x09 Pairing info (no paired host)
static constexpr uint8_t output_0x09[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 0x20 Firmware version (Jun 24 2024 11:16:21)
static constexpr uint8_t output_0x20[] = {
    0x4a, 0x75, 0x6e, 0x20, 0x32, 0x34, 0x20, 0x32,
    0x30, 0x32, 0x34, 0x31, 0x31, 0x3a, 0x31, 0x36,
    0x3a, 0x32, 0x31, 0x03, 0x00, 0x04, 0x00, 0x13,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01, 0x41,
    0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x05, 0x00, 0x00, 0x2a,
    0x00, 0x01, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void PS5Driver::initialize() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    touchpadData.p1.unpressed = 1;
    touchpadData.p1.set_x(PS5_TP_X_MAX / 2);
    touchpadData.p1.set_y(PS5_TP_Y_MAX / 2);
    touchpadData.p2.unpressed = 1;
    touchpadData.p2.set_x(PS5_TP_X_MAX / 2);
    touchpadData.p2.set_y(PS5_TP_Y_MAX / 2);

    gamepad->auxState.sensors.touchpad[0].x = PS5_TP_X_MAX / 2;
    gamepad->auxState.sensors.touchpad[0].y = PS5_TP_Y_MAX / 2;
    gamepad->auxState.sensors.touchpad[1].x = PS5_TP_X_MAX / 2;
    gamepad->auxState.sensors.touchpad[1].y = PS5_TP_Y_MAX / 2;

    memset(&ps5Report, 0, sizeof(ps5Report));
    ps5Report.report_id = 0x01;
    ps5Report.left_stick_x = PS5_JOYSTICK_MID;
    ps5Report.left_stick_y = PS5_JOYSTICK_MID;
    ps5Report.right_stick_x = PS5_JOYSTICK_MID;
    ps5Report.right_stick_y = PS5_JOYSTICK_MID;
    ps5Report.dpad = PS5_HAT_NOTHING;
    ps5Report.data_30_31 = 0x001a;             // as sent by the P5General
    ps5Report.touchpad_data = touchpadData;

    // Force the first report out
    memset(&lastReport, 0xFF, sizeof(lastReport));

    class_driver = {
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

    lastSentUs = getMicro();
}

void PS5Driver::initializeAux() {
    usbHostAuth = PeripheralManager::getInstance().isUSBEnabled(0);
    authDetector.setup();
    PS5_LOG("[PS5] init: USB host auth %s\n", usbHostAuth ? "enabled" : "disabled (no USB host peripheral)");
}

USBListener * PS5Driver::get_usb_auth_listener() {
    return usbHostAuth ? &authDetector : nullptr;
}

PS5Auth * PS5Driver::readyAuth() const {
    PS5Auth * auth = authDetector.auth();
    return (auth != nullptr && auth->ready()) ? auth : nullptr;
}

bool PS5Driver::getAuthSent() {
    // Like PS4 mode: true only once the console has finished an auth round.
    return consoleAuthDone && readyAuth() != nullptr;
}

bool PS5Driver::consoleAuthFailed() const {
    return consoleAuthStartUs != 0 && !consoleAuthDone &&
           (getMicro() - consoleAuthStartUs) >= PS5_CONSOLE_AUTH_TIMEOUT_US;
}

bool PS5Driver::getDongleAuthRequired() {
    return usbHostAuth;
}

bool PS5Driver::process(Gamepad * gamepad) {
    authDetector.process();

    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    buildReport(gamepad);

    bool changed = memcmp(&lastReport, &ps5Report, sizeof(ps5Report)) != 0;
    if (changed) {
        lastReport = ps5Report;
        repeatCount = PS5_REPORT_REPEAT;
    }
    bool due = changed || repeatCount > 0 || (getMicro() - lastSentUs) >= PS5_STREAM_INTERVAL_US;

    PS5Auth * auth = readyAuth();
    if ((auth != nullptr) != authWasReady) {
        authWasReady = (auth != nullptr);
        consoleAuthDone = false;    // a new (or no) provider must authenticate again
        consoleAuthStartUs = 0;
        if (authWasReady) {
            PS5_LOG("[PS5] auth ready: %s, sending signed reports\n", auth->name());
            signedSent = 0;
            submitted = 0;
            lastSignedUs = getMicro();
            stallLogged = false;
        } else {
            PS5_LOG("[PS5] auth not available, sending unsigned reports\n");
        }
    }
    if (auth == nullptr) {
        signedOutPending = false;
        return sendUnsigned(due);
    }
    return sendSigned(auth, due);
}

bool PS5Driver::sendUnsigned(bool due) {
    if (!due || !tud_hid_ready()) {
        return false;
    }
    if (!tud_hid_report(0, &ps5Report, sizeof(ps5Report))) {
        return false;
    }
    if (repeatCount > 0) {
        repeatCount--;
    }
    lastSentUs = getMicro();
    return true;
}

bool PS5Driver::sendSigned(PS5Auth * auth, bool due) {
    bool sent = false;

    // Forward the provider's signed report; hold it if the endpoint is busy.
    if (!signedOutPending) {
        signedOutPending = auth->takeSignedReport(signedOut);
    }
    if (signedOutPending && tud_hid_ready() && tud_hid_report(0, signedOut, sizeof(signedOut))) {
        signedOutPending = false;
        lastSentUs = getMicro();
        lastSignedUs = lastSentUs;
        sent = true;
        if (stallLogged) {
            PS5_LOG("[PS5] signing resumed\n");
            stallLogged = false;
        }
        if (!firstSignedDumped) {
            firstSignedDumped = true;
            PS5_LOG("[PS5] first signed report (%s):", auth->name());
            for (uint8_t i = 0; i < sizeof(signedOut); i++) {
                PS5_LOG("%s%02x", (i % 16) ? " " : "\n    ", signedOut[i]);
            }
            PS5_LOG("\n");
        }
        // First signed report, then every 1000 to confirm signing continues
        if (signedSent++ % 1000 == 0) {
            PS5_LOG("[PS5] signed reports sent: %lu\n", (unsigned long)signedSent);
        }
    }

    // Hand the provider the next report to sign.
    if (due && auth->submitReport(ps5Report)) {
        if (repeatCount > 0) {
            repeatCount--;
        }
        lastSentUs = getMicro();
        submitted++;
    }

    if (!stallLogged && (getMicro() - lastSignedUs) >= PS5_SIGN_STALL_US) {
        stallLogged = true;
        PS5_LOG("[PS5] signing stalled: no signed report for %lums (%lu submitted, %lu signed so far)\n",
                (unsigned long)(PS5_SIGN_STALL_US / 1000), (unsigned long)submitted, (unsigned long)signedSent);
    }
    return sent;
}

void PS5Driver::buildReport(Gamepad * gamepad) {
    const GamepadOptions & options = gamepad->getOptions();

    switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD) {
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

    ps5Report.left_stick_x  = static_cast<uint8_t>(gamepad->state.lx >> 8);
    ps5Report.left_stick_y  = static_cast<uint8_t>(gamepad->state.ly >> 8);
    ps5Report.right_stick_x = static_cast<uint8_t>(gamepad->state.rx >> 8);
    ps5Report.right_stick_y = static_cast<uint8_t>(gamepad->state.ry >> 8);

    if (gamepad->hasAnalogTriggers) {
        ps5Report.left_trigger  = gamepad->state.lt;
        ps5Report.right_trigger = gamepad->state.rt;
    } else {
        ps5Report.left_trigger  = gamepad->pressedL2() ? 0xFF : 0;
        ps5Report.right_trigger = gamepad->pressedR2() ? 0xFF : 0;
    }

    // Sensors are reported byte-swapped
    if (gamepad->auxState.sensors.gyroscope.enabled) {
        ps5Report.gyroscope.x = __builtin_bswap16(gamepad->auxState.sensors.gyroscope.x);
        ps5Report.gyroscope.y = __builtin_bswap16(gamepad->auxState.sensors.gyroscope.y);
        ps5Report.gyroscope.z = __builtin_bswap16(gamepad->auxState.sensors.gyroscope.z);
    }
    if (gamepad->auxState.sensors.accelerometer.enabled) {
        ps5Report.accelerometer.x = __builtin_bswap16(gamepad->auxState.sensors.accelerometer.x);
        ps5Report.accelerometer.y = __builtin_bswap16(gamepad->auxState.sensors.accelerometer.y);
        ps5Report.accelerometer.z = __builtin_bswap16(gamepad->auxState.sensors.accelerometer.z);
    }

    // Touchpad: the touchpad button emulates one finger (A3 left, A4 right),
    // otherwise pass through touchpad sensors in order.
    touchpadData.p1.unpressed = ps5Report.button_touchpad ? 0 : 1;
    if (ps5Report.button_touchpad) {
        if (gamepad->pressedA3()) {
            touchpadData.p1.set_x(PS5_TP_X_MIN);
        } else if (gamepad->pressedA4()) {
            touchpadData.p1.set_x(PS5_TP_X_MAX);
        } else {
            touchpadData.p1.set_x(PS5_TP_X_MAX / 2);
        }
    } else if (gamepad->auxState.sensors.touchpad[0].enabled) {
        touchpadData.p1.unpressed = !gamepad->auxState.sensors.touchpad[0].active;
        touchpadData.p1.set_x(gamepad->auxState.sensors.touchpad[0].x);
        touchpadData.p1.set_y(gamepad->auxState.sensors.touchpad[0].y);

        if (gamepad->auxState.sensors.touchpad[1].enabled) {
            touchpadData.p2.unpressed = !gamepad->auxState.sensors.touchpad[1].active;
            touchpadData.p2.set_x(gamepad->auxState.sensors.touchpad[1].x);
            touchpadData.p2.set_y(gamepad->auxState.sensors.touchpad[1].y);
        }
    }

    // New touches get a new counter value
    if (!pointOneTouched && !touchpadData.p1.unpressed) {
        touchCounter = (touchCounter < PS5_TP_MAX_COUNT ? touchCounter + 1 : 0);
        touchpadData.p1.counter = touchCounter;
        pointOneTouched = true;
    } else if (pointOneTouched && touchpadData.p1.unpressed) {
        pointOneTouched = false;
    }
    if (!pointTwoTouched && !touchpadData.p2.unpressed) {
        touchCounter = (touchCounter < PS5_TP_MAX_COUNT ? touchCounter + 1 : 0);
        touchpadData.p2.counter = touchCounter;
        pointTwoTouched = true;
    } else if (pointTwoTouched && touchpadData.p2.unpressed) {
        pointTwoTouched = false;
    }
    ps5Report.touchpad_data = touchpadData;
}

uint16_t PS5Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen) {
    if (report_type != HID_REPORT_TYPE_FEATURE) {
        return -1;
    }

    PS5Auth * auth = readyAuth();
    uint16_t len = 0;
    if (auth != nullptr && auth->getFeature(report_id, buffer, reqlen, len)) {
        // F2 payload: [1] frame, [2] status. "Done" ends a successful round.
        if (report_id == PS5_FEATURE_GET_SIGNING_STATE && len >= 3) {
            if (buffer[2] == PS5_AUTH_STATUS_DONE && !consoleAuthDone) {
                consoleAuthDone = true;
                PS5_LOG("[PS5] console auth complete (F2 done, frame=%02x) via %s\n", buffer[1], auth->name());
            }
        }
        return len;
    }
    return getDefaultFeature(report_id, buffer, reqlen);
}

uint16_t PS5Driver::getDefaultFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen) {
    const uint8_t * src = nullptr;
    uint16_t size = 0;

    switch (report_id) {
        case PS5_FEATURE_DEFINITION:   src = output_0x03; size = sizeof(output_0x03); break;
        case PS5_FEATURE_CALIBRATION:  src = output_0x05; size = sizeof(output_0x05); break;
        case PS5_FEATURE_PAIRING_INFO: src = output_0x09; size = sizeof(output_0x09); break;
        case PS5_FEATURE_FIRMWARE:     src = output_0x20; size = sizeof(output_0x20); break;
        case PS5_FEATURE_GET_SIGNING_STATE:
            // No authenticator: report "still signing" for the console's
            // current challenge so it keeps polling until one is plugged in.
            if (reqlen < 15) {
                return -1;
            }
            memset(buffer, 0, 15);
            buffer[1] = consoleFrameId;
            buffer[2] = (consoleAuthType == 0x03) ? PS5_AUTH_STATUS_REFRESH_NOT_READY : PS5_AUTH_STATUS_NOT_READY;
            ps5WriteAuthCrc(PS5_FEATURE_GET_SIGNING_STATE, buffer, 15);
            return 15;
        case PS5_FEATURE_GET_SIGNATURE:  // nothing to give without an authenticator
        case PS5_FEATURE_GET_TEST:
        default:
            return -1;
    }

    if (reqlen < size) {
        return -1;
    }
    memcpy(buffer, src, size);
    return size;
}

void PS5Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_FEATURE) {
        // A console sends output report 0x02 (lightbar, player LEDs, rumble)
        // once it has taken the controller on.
        if (!outputLogged) {
            outputLogged = true;
            // Interrupt OUT data arrives with report_id 0 and the ID in buffer[0].
            uint8_t id = (report_id == 0 && bufsize > 0) ? buffer[0] : report_id;
            PS5_LOG("[PS5] first output report: type=%u id=%02x len=%u\n", report_type, id, bufsize);
        }
        return;
    }

    if (report_id == PS5_FEATURE_SET_AUTH_PAYLOAD && bufsize == sizeof(PS5AuthPage)) {
        const PS5AuthPage * page = (const PS5AuthPage *)buffer;
        consoleFrameId = page->frame_id;
        consoleAuthType = page->auth_type;
        if (readyAuth() == nullptr) {
            PS5_LOG("[PS5] console F0 type=%02x frame=%02x page=%u with no auth device (answering not ready)\n",
                    page->auth_type, page->frame_id, page->page_index);
        } else if (consoleAuthStartUs == 0) {
            consoleAuthStartUs = getMicro();
        }
    }

    PS5Auth * auth = readyAuth();
    if (auth != nullptr) {
        auth->setFeature(report_id, buffer, bufsize);
    }
}

const uint16_t * PS5Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    if (index >= TU_ARRAY_SIZE(ps5_string_descriptors)) {
        return nullptr;
    }
    const char * value = (const char *)ps5_string_descriptors[index];
    return getStringDescriptor(value, index);
}

const uint8_t * PS5Driver::get_descriptor_device_cb() {
    return ps5_device_descriptor;
}

const uint8_t * PS5Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    return ps5_report_descriptor;
}

const uint8_t * PS5Driver::get_descriptor_configuration_cb(uint8_t index) {
    return ps5_configuration_descriptor;
}

uint16_t PS5Driver::GetJoystickMidValue() {
    return PS5_JOYSTICK_MID << 8;
}
