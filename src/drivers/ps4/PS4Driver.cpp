#include "drivers/ps4/PS4Driver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"
#include "CRC32.h"
#include "mbedtls/error.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include <random>
#include "class/hid/hid.h"

// PS4/PS5 Auth Systems
#include "drivers/ps4/PS4Auth.h"

#include "enums.pb.h"

// force a report to be sent every X ms
#define PS4_KEEPALIVE_TIMER 5

// Controller calibration
static constexpr uint8_t output_0x02[] = {
    0xfe, 0xff, 0x0e, 0x00, 0x04, 0x00, 0xd4, 0x22,
    0x2a, 0xdd, 0xbb, 0x22, 0x5e, 0xdd, 0x81, 0x22, 
    0x84, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0x85, 0x1f,
    0xb0, 0xe0, 0xc6, 0x20, 0xb5, 0xe0, 0xb1, 0x20,
    0x83, 0xdf, 0x0c, 0x00
};

// Controller descriptor (byte[4] = 0x00 for ps4, 0x07 for ps5)
static constexpr uint8_t output_0x03[] = {
    0x21, 0x27, 0x04, 0xcf, 0x00, 0x2c, 0x56,
    0x08, 0x00, 0x3d, 0x00, 0xe8, 0x03, 0x04, 0x00,
    0xff, 0x7f, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Bluetooth device and host details
static constexpr uint8_t output_0x12[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // device MAC address
    0x08, 0x25, 0x00,                   // BT device class
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // host MAC address
};

// Controller firmware version and datestamp
static constexpr uint8_t output_0xa3[] = {
    0x4a, 0x75, 0x6e, 0x20, 0x20, 0x39, 0x20, 0x32,
    0x30, 0x31, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0x32, 0x3a, 0x33, 0x36, 0x3a, 0x34, 0x31,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x08, 0xb4, 0x01, 0x00, 0x00, 0x00,
    0x07, 0xa0, 0x10, 0x20, 0x00, 0xa0, 0x02, 0x00
};

// Nonce Page Size: 0x38 (56)
// Response Page Size: 0x38 (56)
static constexpr uint8_t output_0xf3[] = {
    0x0, 0x38, 0x38, 0, 0, 0, 0
};

void PS4Driver::initialize() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const GamepadOptions & options = gamepad->getOptions();

    // set up device descriptor IDs depending on mode
    uint8_t descSize = sizeof(ps4_device_descriptor);
    memcpy(deviceDescriptor, &ps4_device_descriptor, descSize);

    memset(&ps4Features, 0, sizeof(ps4Features));

    bool isDeviceEmulated = options.ps4ControllerIDMode == PS4ControllerIDMode::PS4_ID_EMULATION;

    if (!isDeviceEmulated) {
        deviceDescriptor[8] = LSB(PS4_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS4_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS4_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS4_PRODUCT_ID);
    } else {
        deviceDescriptor[8] = LSB(DS4_VENDOR_ID);
        deviceDescriptor[9] = MSB(DS4_VENDOR_ID);
        deviceDescriptor[10] = LSB(DS4_PRODUCT_ID);
        deviceDescriptor[11] = MSB(DS4_PRODUCT_ID);
    }

    // init feature data
    touchpadData.p1.unpressed = 1;
    touchpadData.p1.set_x(PS4_TP_X_MAX / 2);
    touchpadData.p1.set_y(PS4_TP_Y_MAX / 2);
    touchpadData.p2.unpressed = 1;
    touchpadData.p2.set_x(PS4_TP_X_MAX / 2);
    touchpadData.p2.set_y(PS4_TP_Y_MAX / 2);

    sensorData.powerLevel = 0xB; // 0x00-0x0A, 0x00-0x0B if charging
    sensorData.charging = 1;     // set this to 1 to show as plugged in
    sensorData.headphones = 0;
    sensorData.microphone = 0;
    sensorData.extension = 0;
    sensorData.gyroscope.x = 0;
    sensorData.gyroscope.y = 0;
    sensorData.gyroscope.z = 0;
    sensorData.accelerometer.x = 0;
    sensorData.accelerometer.y = 0;
    sensorData.accelerometer.z = 0;

    // preseed touchpad sensors with center position values
    gamepad->auxState.sensors.touchpad[0].x = PS4_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[0].y = PS4_TP_Y_MAX/2;
    gamepad->auxState.sensors.touchpad[1].x = PS4_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[1].y = PS4_TP_Y_MAX/2;

    touchCounter = 0;

    ps4Report = {
        .report_id = 0x01,
        .left_stick_x = PS4_JOYSTICK_MID,
        .left_stick_y = PS4_JOYSTICK_MID,
        .right_stick_x = PS4_JOYSTICK_MID,
        .right_stick_y = PS4_JOYSTICK_MID,
        .dpad = 0x08,
        .button_west = 0, .button_south = 0, .button_east = 0, .button_north = 0,
        .button_l1 = 0, .button_r1 = 0, .button_l2 = 0, .button_r2 = 0,
        .button_select = 0, .button_start = 0, .button_l3 = 0, .button_r3 = 0, .button_home = 0,
        .sensor_data = sensorData, .touchpad_active = 0, .padding = 0, .tpad_increment = 0,
        .touchpad_data = touchpadData,
        .mystery_2 = { }
    };

    class_driver = {
    #if CFG_TUSB_DEBUG >= 2
        .name = "PS4",
    #endif
        .init = hidd_init,
        .reset = hidd_reset,
        .open = hidd_open,
        .control_xfer_cb = hidd_control_xfer_cb,
        .xfer_cb = hidd_xfer_cb,
        .sof = NULL
    };

    last_report_counter = 0; // PS4 Reports
    last_axis_counter = 0;
    last_report_timer = to_ms_since_boot(get_absolute_time());
    cur_nonce_id = 1; // PS4 Auth
    cur_nonce_chunk = 0;
}

void PS4Driver::initializeAux() {
    ps4AuthDriver = nullptr;
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( controllerType == PS4ControllerType::PS4_CONTROLLER ) {
        ps4AuthDriver = new PS4Auth(gamepadOptions.ps4AuthType);
    } else if ( controllerType == PS4ControllerType::PS4_ARCADESTICK ) {
        // Setup PS5 Auth System
        ps4AuthDriver = new PS4Auth(gamepadOptions.ps5AuthType);
    }
    // If authentication driver is set AND auth driver can load (usb enabled, i2c enabled, keys loaded, etc.)
    if ( ps4AuthDriver != nullptr && ps4AuthDriver->available() ) {
        ps4AuthDriver->initialize();
        ps4AuthData = ps4AuthDriver->getAuthData();
        authsent = false;
    }
}

bool PS4Driver::getDongleAuthRequired() {
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( (controllerType == PS4ControllerType::PS4_CONTROLLER && 
                gamepadOptions.ps4AuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) ||
         (controllerType == PS4ControllerType::PS4_ARCADESTICK &&
                gamepadOptions.ps5AuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB )) {
        return true;
    }
    return false;
}

bool PS4Driver::process(Gamepad * gamepad) {
    const GamepadOptions & options = gamepad->getOptions();
    switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
    {
        case GAMEPAD_MASK_UP:                        ps4Report.dpad = PS4_HAT_UP;        break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps4Report.dpad = PS4_HAT_UPRIGHT;   break;
        case GAMEPAD_MASK_RIGHT:                     ps4Report.dpad = PS4_HAT_RIGHT;     break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps4Report.dpad = PS4_HAT_DOWNRIGHT; break;
        case GAMEPAD_MASK_DOWN:                      ps4Report.dpad = PS4_HAT_DOWN;      break;
        case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps4Report.dpad = PS4_HAT_DOWNLEFT;  break;
        case GAMEPAD_MASK_LEFT:                      ps4Report.dpad = PS4_HAT_LEFT;      break;
        case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps4Report.dpad = PS4_HAT_UPLEFT;    break;
        default:                                     ps4Report.dpad = PS4_HAT_NOTHING;   break;
    }

    bool anyA2A3A4 = gamepad->pressedA2() || gamepad->pressedA3() || gamepad->pressedA4();

    ps4Report.button_south    = gamepad->pressedB1();
    ps4Report.button_east     = gamepad->pressedB2();
    ps4Report.button_west     = gamepad->pressedB3();
    ps4Report.button_north    = gamepad->pressedB4();
    ps4Report.button_l1       = gamepad->pressedL1();
    ps4Report.button_r1       = gamepad->pressedR1();
    ps4Report.button_l2       = gamepad->pressedL2();
    ps4Report.button_r2       = gamepad->pressedR2();
    ps4Report.button_select   = options.switchTpShareForDs4 ? anyA2A3A4 : gamepad->pressedS1();
    ps4Report.button_start    = gamepad->pressedS2();
    ps4Report.button_l3       = gamepad->pressedL3();
    ps4Report.button_r3       = gamepad->pressedR3();
    ps4Report.button_home     = gamepad->pressedA1();
    ps4Report.button_touchpad = options.switchTpShareForDs4 ? gamepad->pressedS1() : anyA2A3A4;

    ps4Report.left_stick_x = static_cast<uint8_t>(gamepad->state.lx >> 8);
    ps4Report.left_stick_y = static_cast<uint8_t>(gamepad->state.ly >> 8);
    ps4Report.right_stick_x = static_cast<uint8_t>(gamepad->state.rx >> 8);
    ps4Report.right_stick_y = static_cast<uint8_t>(gamepad->state.ry >> 8);

    if (gamepad->hasAnalogTriggers)
    {
        ps4Report.left_trigger = gamepad->state.lt;
        ps4Report.right_trigger = gamepad->state.rt;
    } else {
        ps4Report.left_trigger = gamepad->pressedL2() ? 0xFF : 0;
        ps4Report.right_trigger = gamepad->pressedR2() ? 0xFF : 0;
    }

    // if the touchpad is pressed (note A2 vs. S1 choice above), emulate one finger of the touchpad
    touchpadData.p1.unpressed = ps4Report.button_touchpad ? 0 : 1;
    ps4Report.touchpad_active = ps4Report.button_touchpad ? 0x01 : 0x00;
    if (ps4Report.button_touchpad) {
        // make the assumption that since touchpad button is already being pressed, 
        // the first touch position is in use and no other "touches" will be present
        if (gamepad->pressedA3()) {
            touchpadData.p1.set_x(PS4_TP_X_MIN);
        } else if (gamepad->pressedA4()) {
            touchpadData.p1.set_x(PS4_TP_X_MAX);
        } else {
            touchpadData.p1.set_x(PS4_TP_X_MAX / 2);
        }
    } else {
        // if more than one touch pad sensor, sensors will never be used out of order
        if (gamepad->auxState.sensors.touchpad[0].enabled) {
            touchpadData.p1.unpressed = !gamepad->auxState.sensors.touchpad[0].active;
            ps4Report.touchpad_active = gamepad->auxState.sensors.touchpad[0].active;
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
        touchCounter = (touchCounter < PS4_TP_MAX_COUNT ? touchCounter+1 : 0);

        touchpadData.p1.counter = touchCounter;

        pointOneTouched = true;
    } else if (pointOneTouched && touchpadData.p1.unpressed) {
        pointOneTouched = false;
    }
    if (!pointTwoTouched && !touchpadData.p2.unpressed) {
        touchCounter = (touchCounter < PS4_TP_MAX_COUNT ? touchCounter+1 : 0);
    
        touchpadData.p2.counter = touchCounter;
    
        pointTwoTouched = true;
    } else if (pointTwoTouched && touchpadData.p2.unpressed) {
        pointTwoTouched = false;
    }
    ps4Report.touchpad_data = touchpadData;

    if (gamepad->auxState.sensors.accelerometer.enabled) {
        ps4Report.sensor_data.accelerometer.x = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
        ps4Report.sensor_data.accelerometer.y = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
        ps4Report.sensor_data.accelerometer.z = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
    }

    if (gamepad->auxState.sensors.gyroscope.enabled) {
        ps4Report.sensor_data.gyroscope.x = ((gamepad->auxState.sensors.gyroscope.x & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.x & 0xFF00) >> 8);
        ps4Report.sensor_data.gyroscope.y = ((gamepad->auxState.sensors.gyroscope.y & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.y & 0xFF00) >> 8);
        ps4Report.sensor_data.gyroscope.z = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
    }

    // Wake up TinyUSB device
    if (tud_suspended())
        tud_remote_wakeup();

    bool reportSent = false;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    void * report = &ps4Report;
    uint16_t report_size = sizeof(ps4Report);
    if (memcmp(last_report, report, report_size) != 0)
    {
        // HID ready + report sent, copy previous report
        if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
            memcpy(last_report, report, report_size);
            reportSent = true;
        }
        // keep track of our last successful report, for keepalive purposes
        last_report_timer = now;
    } else {
        // some games apparently can miss reports, or they rely on official behavior of getting frequent
        // updates. we normally only send a report when the value changes; if we increment the counters
        // every time we generate the report (every GP2040::run loop), we apparently overburden
        // TinyUSB and introduce roughly 1ms of latency. but we want to loop often and report on every
        // true update in order to achieve our tight <1ms report timing when we *do* have a different
        // report to send.
        if ((now - last_report_timer) > PS4_KEEPALIVE_TIMER) {
            last_report_counter = (last_report_counter+1) & 0x3F;
            ps4Report.report_counter = last_report_counter;		// report counter is 6 bits
            ps4Report.axis_timing = now;		 		// axis counter is 16 bits
            // the *next* process() will be a forced report (or real user input)
        }
    }

    uint16_t featureSize = sizeof(PS4FeatureOutputReport);
    if (memcmp(lastFeatures, &ps4Features, featureSize) != 0) {
        memcpy(lastFeatures, &ps4Features, featureSize);
        Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

        if (gamepad->auxState.haptics.leftActuator.enabled) {
            gamepad->auxState.haptics.leftActuator.active = (ps4Features.rumbleLeft > 0);
            gamepad->auxState.haptics.leftActuator.intensity = ps4Features.rumbleLeft;
        }

        if (gamepad->auxState.haptics.rightActuator.enabled) {
            gamepad->auxState.haptics.rightActuator.active = (ps4Features.rumbleRight > 0);
            gamepad->auxState.haptics.rightActuator.intensity = ps4Features.rumbleRight;
        }

        if (gamepad->auxState.sensors.statusLight.enabled) {
            uint32_t rgbColor = 0;

            gamepad->auxState.sensors.statusLight.active = true;
            gamepad->auxState.sensors.statusLight.color.red = ps4Features.ledRed;
            gamepad->auxState.sensors.statusLight.color.green = ps4Features.ledGreen;
            gamepad->auxState.sensors.statusLight.color.blue = ps4Features.ledBlue;

            rgbColor = (ps4Features.ledRed << 16) | (ps4Features.ledGreen << 8) | (ps4Features.ledBlue << 0);

            // set player ID based on color combos
            gamepad->auxState.playerID.active = true;
            gamepad->auxState.playerID.ledBlinkOn = (ps4Features.ledBlinkOn * 10); // centiseconds to milliseconds
            gamepad->auxState.playerID.ledBlinkOff = (ps4Features.ledBlinkOff * 10); // centiseconds to milliseconds
            if (rgbColor == 0x000040) {
                gamepad->auxState.playerID.value = 1;
                gamepad->auxState.playerID.ledValue = 1;
            } else if (rgbColor == 0x400000) {
                gamepad->auxState.playerID.value = 2;
                gamepad->auxState.playerID.ledValue = 2;
            } else if (rgbColor == 0x004000) {
                gamepad->auxState.playerID.value = 3;
                gamepad->auxState.playerID.ledValue = 3;
            } else if (rgbColor == 0x200020) {
                gamepad->auxState.playerID.value = 4;
                gamepad->auxState.playerID.ledValue = 4;
            }
        }
    }

    return reportSent;
}

// Called by Core1, PS4 key signing will lock the CPU
void PS4Driver::processAux() {
    // If authentication driver is set AND auth driver can load (usb enabled, i2c enabled, keys loaded, etc.)
    if ( ps4AuthDriver != nullptr && ps4AuthDriver->available() ) {
        ps4AuthDriver->process();
    }
}

USBListener * PS4Driver::get_usb_auth_listener() {
    if ( ps4AuthDriver != nullptr && ps4AuthDriver->getAuthType() == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        return ps4AuthDriver->getListener();
    }
    return nullptr;
}

// tud_hid_get_report_cb
uint16_t PS4Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        memcpy(buffer, &ps4Report, sizeof(ps4Report));
        return sizeof(ps4Report);
    }

    // Do nothing if we do not have host authentication data or a driver to run on
    if ( ps4AuthData == nullptr || ps4AuthDriver == nullptr) {
        return sizeof(ps4Report);
    }

    uint8_t data[64] = {};
    uint32_t crc32;
    uint16_t responseLen = 0;
    switch(report_id) {
        // Controller Definition Report
        case PS4AuthReport::PS4_GET_CALIBRATION:
            if (reqlen < sizeof(output_0x02)) {
                return -1;
            }
            responseLen = MAX(reqlen, sizeof(output_0x02));
            memcpy(buffer, output_0x02, responseLen);
            return responseLen;
        case PS4AuthReport::PS4_DEFINITION:
            if (reqlen < sizeof(output_0x03)) {
                return -1;
            }
            responseLen = MAX(reqlen, sizeof(output_0x03));
            memcpy(buffer, output_0x03, responseLen);
            buffer[4] = (uint8_t)controllerType; // Change controller type in definition
            return responseLen;
        case PS4AuthReport::PS4_GET_MAC_ADDRESS:
            if (reqlen < sizeof(output_0x12)) {
                return -1;
            }
            responseLen = MAX(reqlen, sizeof(output_0x12));
            memcpy(buffer, output_0x12, responseLen);
            return responseLen;
        case PS4AuthReport::PS4_GET_VERSION_DATE:
            if (reqlen < sizeof(output_0xa3)) {
                return -1;
            }
            responseLen = MAX(reqlen, sizeof(output_0xa3));
            memcpy(buffer, output_0xa3, responseLen);
            return responseLen;
        // Use our private RSA key to sign the nonce and return chunks
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // We send 56 byte chunks back to the PS4, we've already calculated these
            data[0] = 0xF1;
            data[1] = cur_nonce_id;    // nonce_id
            data[2] = cur_nonce_chunk; // next_part
            data[3] = 0;

            // 56 byte chunks
            memcpy(&data[4], &ps4AuthData->ps4_auth_buffer[cur_nonce_chunk*56], 56);

            // calculate the CRC32 of the buffer and write it back
            crc32 = CRC32::calculate(data, 60);
            memcpy(&data[60], &crc32, sizeof(uint32_t));
            memcpy(buffer, &data[1], 63); // move data over to buffer
            cur_nonce_chunk++;
            if ( cur_nonce_chunk == 19 ) { // done!
                ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
                authsent = true;
                cur_nonce_chunk = 0;
            }
            return 63;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:  // Are we ready to sign?
            data[0] = 0xF2;
            data[1] = cur_nonce_id;
            data[2] = (ps4AuthData->passthrough_state == GPAuthState::send_auth_dongle_to_console) ? 0 : 16; // 0 means auth is ready, 16 means we're still signing
            memset(&data[3], 0, 9);
            crc32 = CRC32::calculate(data, 12);
            memcpy(&data[12], &crc32, sizeof(uint32_t));
            memcpy(buffer, &data[1], 15); // move data over to buffer
            return 15;
        case PS4AuthReport::PS4_RESET_AUTH:         // Reset the Authentication
            if (reqlen < sizeof(output_0xf3)) {
                return -1;
            }
            responseLen = MAX(reqlen, sizeof(output_0xf3));
            memcpy(buffer, output_0xf3, responseLen);
            ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
            ps4AuthDriver->resetAuth(); // reset our auth driver (ps4 keys or usb host)
            return responseLen;
        default:
            break;
    };
    return -1;
}

// Only PS4 does anything with set report
void PS4Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    if (( report_type != HID_REPORT_TYPE_FEATURE ) && ( report_type != HID_REPORT_TYPE_OUTPUT ))
        return;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        if (report_id == 0) {
            memcpy(&ps4Features, buffer, bufsize);
        }
    } else if (report_type == HID_REPORT_TYPE_FEATURE) {
        if (report_id == PS4AuthReport::PS4_SET_HOST_MAC) {
            // 
        } else if (report_id == PS4AuthReport::PS4_SET_USB_BT_CONTROL) {
            // 
        } else if (report_id == PS4AuthReport::PS4_SET_AUTH_PAYLOAD) {
            uint8_t sendBuffer[64];
            uint8_t nonce_id;
            uint8_t nonce_page;
            uint16_t buflen;
            if (bufsize != 63 ) {
                return;
            }
            // Calculate CRC32 of buffer
            sendBuffer[0] = report_id;
            memcpy(&sendBuffer[1], buffer, bufsize);
            buflen = bufsize + 1;
            if ( CRC32::calculate(sendBuffer, buflen-sizeof(uint32_t)) !=
                    *((unsigned int*)&sendBuffer[buflen-sizeof(uint32_t)])) {
                return; // CRC32 failed on set report
            }
            // 256 byte nonce, 4 56-byte chunks, 1 32-byte chunk
            nonce_id = buffer[0];
            nonce_page = buffer[1];
            if ( nonce_page == 4 ) {    // Nonce page 4 : 32 bytes
                memcpy(&ps4AuthData->ps4_auth_buffer[nonce_page*56], &sendBuffer[4], 32);
                ps4AuthData->nonce_id = nonce_id;
                ps4AuthData->passthrough_state = GPAuthState::send_auth_console_to_dongle;
            } else {                    // Nonce page 0-3 : 56 bytes
                memcpy(&ps4AuthData->ps4_auth_buffer[nonce_page*56], &sendBuffer[4], 56);
            }
            if ( nonce_page == 0 ) { // Set our passthrough state on first nonce
                cur_nonce_id = nonce_id; // update current nonce ID
            } else if ( nonce_id != cur_nonce_id ) {
                // ERROR: Nonce ID is incorrect
                ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
            }
        }
    }
}


// Only XboxOG and Xbox One use vendor control xfer cb
bool PS4Driver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * PS4Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const char *value = (const char *)ps4_string_descriptors[index];
    return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PS4Driver::get_descriptor_device_cb() {
    return deviceDescriptor;
}

const uint8_t * PS4Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    return ps4_report_descriptor;
}

const uint8_t * PS4Driver::get_descriptor_configuration_cb(uint8_t index) {
    return ps4_configuration_descriptor;
}

const uint8_t * PS4Driver::get_descriptor_device_qualifier_cb() {
    return nullptr;
}

uint16_t PS4Driver::GetJoystickMidValue() {
    return PS4_JOYSTICK_MID << 8;
}
