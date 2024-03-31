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

void PS4Driver::initialize() {
    //touchpadData = {
    //	.p1 = 0x00,
    //	.p2 = 0x00
    //};

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
        .padding = 0,
        .mystery = { },
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

    last_report_counter = 0;
    last_axis_counter = 0;
    cur_nonce_id = 1;
    last_report_timer = to_ms_since_boot(get_absolute_time());
    send_nonce_part = 0;

    // for PS4 encryption
    ps4State = PS4State::no_nonce;
    authsent = false;
    memset(nonce_buffer, 0, 256);
}

void PS4Driver::initializeAux() {
    authDriver = nullptr;
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( controllerType == PS4ControllerType::PS4_CONTROLLER ) {
        authDriver = new PS4Auth(gamepadOptions.ps4AuthType);
    } else if ( controllerType == PS4ControllerType::PS4_ARCADESTICK ) {
        // Setup PS5 Auth System
        authDriver = new PS4Auth(gamepadOptions.ps5AuthType);
    }
    // If authentication driver is set AND auth driver can load (usb enabled, i2c enabled, keys loaded, etc.)
    if ( authDriver != nullptr && authDriver->available() ) {
        authDriver->initialize();
    }
}

void PS4Driver::process(Gamepad * gamepad, uint8_t * outBuffer) {
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

    ps4Report.button_south    = gamepad->pressedB1();
    ps4Report.button_east     = gamepad->pressedB2();
    ps4Report.button_west     = gamepad->pressedB3();
    ps4Report.button_north    = gamepad->pressedB4();
    ps4Report.button_l1       = gamepad->pressedL1();
    ps4Report.button_r1       = gamepad->pressedR1();
    ps4Report.button_l2       = gamepad->pressedL2();
    ps4Report.button_r2       = gamepad->pressedR2();
    ps4Report.button_select   = options.switchTpShareForDs4 ? gamepad->pressedA2() : gamepad->pressedS1();
    ps4Report.button_start    = gamepad->pressedS2();
    ps4Report.button_l3       = gamepad->pressedL3();
    ps4Report.button_r3       = gamepad->pressedR3();
    ps4Report.button_home     = gamepad->pressedA1();
    ps4Report.button_touchpad = options.switchTpShareForDs4 ? gamepad->pressedS1() : gamepad->pressedA2();

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

    // set touchpad to nothing
    touchpadData.p1.unpressed = 1;
    touchpadData.p2.unpressed = 1;
    ps4Report.touchpad_data = touchpadData;

    // Wake up TinyUSB device
    if (tud_suspended())
        tud_remote_wakeup();

    uint32_t now = to_ms_since_boot(get_absolute_time());
    void * report = &ps4Report;
    uint16_t report_size = sizeof(ps4Report);
    if (memcmp(last_report, report, report_size) != 0)
    {
        // HID ready + report sent, copy previous report
        if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
            memcpy(last_report, report, report_size);
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
}

// Called by Core1, PS4 key signing will lock the CPU
void PS4Driver::processAux() {
    // If authentication driver is set AND auth driver can load (usb enabled, i2c enabled, keys loaded, etc.)
    if ( authDriver != nullptr && authDriver->available() ) {
        ((PS4Auth*)authDriver)->process(ps4State, nonce_id, nonce_buffer);
    }
}

USBListener * PS4Driver::get_usb_auth_listener() {
    if ( authDriver != nullptr && authDriver->getAuthType() == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        return authDriver->getListener();;
    }
    return nullptr;
}


// Controller descriptor (byte[4] = 0x00 for ps4, 0x07 for ps5)
static constexpr uint8_t output_0x03[] = {
    0x21, 0x27, 0x04, 0xcf, 0x00, 0x2c, 0x56,
    0x08, 0x00, 0x3d, 0x00, 0xe8, 0x03, 0x04, 0x00,
    0xff, 0x7f, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Nonce Page Size: 0x38 (56)
// Response Page Size: 0x38 (56)
static constexpr uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

// tud_hid_get_report_cb
uint16_t PS4Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        memcpy(buffer, &ps4Report, sizeof(ps4Report));
        return sizeof(ps4Report);
    }

    PS4Auth * ps4AuthDriver = (PS4Auth*)authDriver;
    uint8_t * ps4_auth_buffer = ps4AuthDriver->getAuthBuffer();
    bool ps4_auth_buffer_ready = ps4AuthDriver->getAuthReady();
    uint8_t data[64] = {};
    uint32_t crc32;
    //ps4_out_buffer[0] = report_id;
    switch(report_id) {
        // Controller Definition Report
        case PS4AuthReport::PS4_DEFINITION:
            if (reqlen != sizeof(output_0x03)) {
                return -1;
            }
            memcpy(buffer, output_0x03, reqlen);
            buffer[4] = (uint8_t)controllerType; // Change controller type in definition
            return reqlen;
        // Use our private RSA key to sign the nonce and return chunks
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // We send 56 byte chunks back to the PS4, we've already calculated these
            data[0] = 0xF1;
            data[1] = cur_nonce_id;    // nonce_id
            data[2] = send_nonce_part; // next_part
            data[3] = 0;

            // 56 byte chunks
            memcpy(&data[4], &ps4_auth_buffer[send_nonce_part*56], 56);

            // calculate the CRC32 of the buffer and write it back
            crc32 = CRC32::calculate(data, 60);
            memcpy(&data[60], &crc32, sizeof(uint32_t));
            memcpy(buffer, &data[1], 63); // move data over to buffer
            if ( (++send_nonce_part) == 19 ) {
                ps4State = PS4State::no_nonce;
                authsent = true;
                send_nonce_part = 0;
            }
            return 63;
        // Are we ready to sign?
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
              data[0] = 0xF2;
            data[1] = cur_nonce_id;
            data[2] = ps4_auth_buffer_ready == true ? 0 : 16; // 0 means auth is ready, 16 means we're still signing
            memset(&data[3], 0, 9);
            crc32 = CRC32::calculate(data, 12);
            memcpy(&data[12], &crc32, sizeof(uint32_t));
            memcpy(buffer, &data[1], 15); // move data over to buffer
            return 15;
        case PS4AuthReport::PS4_RESET_AUTH: // Reset the Authentication
            if (reqlen != sizeof(output_0xf3)) {
                return -1;
            }
            memcpy(buffer, output_0xf3, reqlen);
            ps4State = PS4State::no_nonce;
            if ( authDriver != nullptr ) {
                ((PS4Auth*)authDriver)->resetAuth(); // reset the auth driver if it exists
            }
            return reqlen;
        default:
            break;
    };
    return -1;
}

// Only PS4 does anything with set report
void PS4Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    if ( report_type != HID_REPORT_TYPE_FEATURE )
        return;

    uint8_t nonce_id;
    uint8_t nonce_page;
    uint32_t crc32;
    uint8_t sendBuffer[64];
    uint8_t nonce[56]; // max nonce data
    uint16_t noncelen;
    uint16_t buflen;

    if (report_id == PS4AuthReport::PS4_SET_AUTH_PAYLOAD) {
        if (bufsize != 63 ) {
            return;
        }

        // Setup CRC32 buffer
        sendBuffer[0] = report_id;
        memcpy(&sendBuffer[1], buffer, bufsize);
        buflen = bufsize + 1;

        nonce_id = buffer[0];
        nonce_page = buffer[1];
        // data[2] is zero padding

        crc32 = CRC32::calculate(sendBuffer, buflen-sizeof(uint32_t));
        if ( crc32 != *((unsigned int*)&sendBuffer[buflen-sizeof(uint32_t)])) {
            return; // CRC32 failed on set report
        }

        // 256 byte nonce, with 56 byte packets leaves 24 extra bytes on the last packet?
        if ( nonce_page == 4 ) {
            // Copy/append data from buffer[4:64-28] into our nonce
            noncelen = 32; // from 4 to 64 - 24 - 4
            nonce_id = nonce_id; // for pass-through only
        } else {
            // Copy/append data from buffer[4:64-4] into our nonce
            noncelen = 56;
            // from 4 to 64 - 4
        }

        memcpy(nonce, &sendBuffer[4], noncelen);
        save_nonce(nonce_id, nonce_page, nonce, noncelen);
    }
}

void PS4Driver::save_nonce(uint8_t nonce_id, uint8_t nonce_page, uint8_t * buffer, uint16_t buflen) {
    if ( nonce_page != 0 && nonce_id != cur_nonce_id ) {
        ps4State = PS4State::no_nonce;
        return; // setting nonce with mismatched id
    }

    memcpy(&nonce_buffer[nonce_page*56], buffer, buflen);
    if ( nonce_page == 4 ) {
        ps4State = PS4State::nonce_ready;
    } else if ( nonce_page == 0 ) {
        cur_nonce_id = nonce_id;
        ps4State = PS4State::receiving_nonce;
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
    return ps4_device_descriptor;
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
