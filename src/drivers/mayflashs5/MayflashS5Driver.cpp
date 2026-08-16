#include "drivers/mayflashs5/MayflashS5Driver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"
#include "usbhostmanager.h"

#include "CRC32.h"

#include "drivers/mayflashs5/MayflashS5Auth.h"
#include "drivers/p5general/P5GeneralDescriptors.h"
#include "enums.pb.h"

// The reference adapter presents a signed input report upstream every 4 ms.
#define PS5_KEEPALIVE_US                          4ll*1000ll

#define PS5_PACKET_SIZE 64

static MayflashS5Driver *activeMayflashS5Driver = nullptr;

static bool mayflashs5_control_xfer_cb(
        uint8_t rhport, uint8_t stage,
        tusb_control_request_t const *request) {
    if (activeMayflashS5Driver == nullptr) {
        return false;
    }
    return activeMayflashS5Driver->controlXferCb(rhport, stage, request);
}


static_assert(CFG_TUD_ENDPOINT0_SIZE == 8,
    "The captured DualSense device descriptor advertises an 8-byte EP0; "
    "CFG_TUD_ENDPOINT0_SIZE must match it (set by CMakeLists.txt).");

// Interface numbers within the 227-byte composite configuration.
#define S5_ITF_AUDIO_CONTROL   0
#define S5_ITF_AUDIO_OUT       1
#define S5_ITF_AUDIO_IN        2
#define S5_ITF_HID             3

// The DualSense configuration begins with an AudioControl interface, so
// hidd_open() -- which only accepts TUSB_CLASS_HID -- can never claim
// interface 0 and SET_CONFIGURATION fails before the parser ever reaches the
// HID interface. That is why every previous exact-identity attempt enumerated
// with no usable inputs.
//
// Claim each audio interface individually so that usbd's itf2drv[] entry is
// populated for interfaces 0, 1 and 2. Claiming all three in one open() call
// only registers interface 0, and the console's SET_INTERFACE(1,1) /
// SET_INTERFACE(2,1) then stall against a NULL driver.
static uint16_t mayflashs5_open(uint8_t rhport,
        tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    if (desc_itf->bInterfaceClass == TUSB_CLASS_HID) {
        return hidd_open(rhport, desc_itf, max_len);
    }

    if (desc_itf->bInterfaceClass != TUSB_CLASS_AUDIO) {
        return 0;
    }

    // Consume this interface number's whole span: its class-specific
    // descriptors plus every alternate setting and their endpoints. The audio
    // endpoints are deliberately left unopened -- the console only needs the
    // interfaces to exist and answer control requests, and the RP2040 has no
    // audio to stream.
    uint8_t const *p_desc = (uint8_t const *)desc_itf;
    uint16_t drv_len = tu_desc_len(p_desc);

    while (drv_len < max_len) {
        uint8_t const *p_next = p_desc + tu_desc_len(p_desc);
        uint8_t const type = tu_desc_type(p_next);
        if (type == TUSB_DESC_INTERFACE_ASSOCIATION) {
            break;
        }
        if (type == TUSB_DESC_INTERFACE) {
            tusb_desc_interface_t const *next_itf =
                (tusb_desc_interface_t const *)p_next;
            if (next_itf->bInterfaceNumber != desc_itf->bInterfaceNumber) {
                break; // a different interface, leave it for the next open()
            }
        }
        p_desc = p_next;
        drv_len = (uint16_t)(drv_len + tu_desc_len(p_desc));
    }

    return drv_len;
}

static void mayflashs5_reset(uint8_t rhport) {
    hidd_reset(rhport);
    if (activeMayflashS5Driver != nullptr) {
        activeMayflashS5Driver->resetAudioState();
    }
}


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
    0xff, 0xff, 0xf4, 0xff, 0xfb, 0xff, 0x92, 0x22, 
    0x6a, 0xdd, 0x8d, 0x22, 0x5d, 0xdd, 0x9b, 0x22,
    0x65, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0xd2, 0x1f,
    0xf2, 0xdf, 0xd0, 0x1f, 0xb7, 0xdf, 0x04, 0x20,
    0xfc, 0xdf, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00
};

static constexpr uint8_t output_0x09[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 0x20 (Firmware Controller Revision Jun 24 2024 11:16:21)
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


// A current DualSense firmware revision report (0x20).
//
// The console compares the reported build against the DualSense firmware
// bundled with its own system software. It judges the S5's own build out of
// date and offers a firmware update instead of proceeding, so a current report
// is substituted for it. Only the version is substituted; the pairing identity
// in report 0x09 is always the dongle's own, because the signature the S5
// produces belongs to it.
//
// This will need updating as Sony ships newer controller firmware.
static constexpr uint8_t dualsense_current_0x20[] = {
    0x4A, 0x75, 0x6C, 0x20, 0x20, 0x34, 0x20, 0x32,  // "Jul  4 2025"
    0x30, 0x32, 0x35,
    0x31, 0x30, 0x3A, 0x31, 0x30, 0x3A, 0x33, 0x32,  // "10:10:32"
    0x03, 0x00,                                       // fw_type 3
    0x04, 0x00,                                       // sw_series 4
    0x14, 0x05, 0x00, 0x00,                           // hw_info 0x0514
    0x2A, 0x00, 0x10, 0x01,                           // fw_version
    0xC1, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // deviceinfo
    0x00, 0x00, 0x00, 0x00,
    0x30, 0x06,                                       // update_version 0x0630
    0x00,
    0x00, 0x38, 0x00, 0x01,
    0x00, 0x0A, 0x00, 0x02,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
static_assert(sizeof(dualsense_current_0x20) == 63,
    "report 0x20 payload is 63 bytes once the report id is stripped");


void MayflashS5Driver::initialize() {
    P5DPRINTF_INIT();
    P5DPRINTF("P5D:initialize\n");

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    activeMayflashS5Driver = this;
    resetAudioState();

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

    // Force an update as soon as we can
    memset(&ps5Report_last, 0xFF, sizeof(ps5Report));

    class_driver = 	{
#if CFG_TUSB_DEBUG >= 2
        .name = "PS5",
#endif
        .init = hidd_init,
        .reset = mayflashs5_reset,
        .open = mayflashs5_open,
        .control_xfer_cb = mayflashs5_control_xfer_cb,
        .xfer_cb = hidd_xfer_cb,
        .sof = NULL
    };

    timeout_report_us = getMicro();
}

void MayflashS5Driver::initializeAux() {
    P5DPRINTF_INIT();
    P5DPRINTF("P5D:initializeAux\n");

    mayflashS5AuthDriver = new MayflashS5Auth();
    if ( mayflashS5AuthDriver != nullptr && mayflashS5AuthDriver->available() ) {
        mayflashS5AuthDriver->initialize();
        ps5AuthData = mayflashS5AuthDriver->getAuthData();
    }
}

bool MayflashS5Driver::getDongleAuthRequired() {
    return true;
}

void MayflashS5Driver::beforeRun() {
    // Do not expose an all-zero pair identity upstream.
    tud_disconnect();
    const uint64_t timeout = getMicro() + 5000ll * 1000ll;
    while ( ps5AuthData != nullptr && !ps5AuthData->S5_reports_ready &&
            getMicro() < timeout) {
        USBHostManager::getInstance().process();
        if (mayflashS5AuthDriver != nullptr &&
                mayflashS5AuthDriver->available()) {
            mayflashS5AuthDriver->process();
        }
    }
    tud_connect();
}

bool MayflashS5Driver::process(Gamepad * gamepad) {
    // TinyUSB host is serviced on core 0. Keep S5 control-transfer
    // submissions on that same core as well; issuing them from processAux()
    // on core 1 can leave the endpoint busy after the first signed report.
    if (mayflashS5AuthDriver != nullptr && mayflashS5AuthDriver->available()) {
        mayflashS5AuthDriver->process();
    }
    serviceDeferredControl();

    if (!ps5AuthData || !ps5AuthData->dongle_ready) {
        return false;
    }

    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    // Hash from Dongle is ready! let's try to send it onto the device
    if (ps5AuthData->hash_ready) {
        memcpy(ps5AuthData->send_hid_buffer, ps5AuthData->hash_finish_buffer, 64);
        if (tud_hid_ready() && tud_hid_report(0, ps5AuthData->send_hid_buffer, PS5_PACKET_SIZE) == true ) {
            ps5AuthData->hash_ready = false;
            if (ps5AuthData->console_report_count == 0) {
            }
            // Per the DualSense layout, USBGetStateData offset 9.0 is
            // ButtonHome, which is report byte 10 bit 0 once the report id is
            // counted. Log the transition so we can tell whether the press
            // actually reaches the console, rather than assuming it does.
            {
                const bool home_now =
                    (ps5AuthData->send_hid_buffer[10] & 0x01) != 0;
                if (home_now != home_was_pressed) {
                    home_was_pressed = home_now;
                    if (home_now) {
                    }
                }
            }
            ps5AuthData->console_report_count++;
            timeout_report_us = getMicro();
        } else if (!tud_mounted()) {
            // No upstream host: the board is on a charger or a port that never
            // enumerated it. Once the dongle has paired over bluetooth it
            // delivers reports to the console itself, so ours has nowhere to
            // go. Discard it and keep reading the stick, otherwise this returns
            // early forever and the dongle is never fed any input at all.
            ps5AuthData->hash_ready = false;
            timeout_report_us = getMicro();
        } else {
            return false;
        }
    }

    // Don't read IO while we are still waiting to send the previous IO to the dongle
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

    // New input or timeout happened
    if (memcmp(&ps5Report_last, &ps5Report, sizeof(ps5Report)) ||
        (getMicro() > (timeout_report_us + PS5_KEEPALIVE_US))) {
        memcpy(&ps5Report_last, &ps5Report, sizeof(ps5Report));
        memcpy(ps5AuthData->hash_pending_buffer, &ps5Report, sizeof(ps5Report));
        ps5AuthData->hash_pending = true;
        timeout_report_us = getMicro(); // don't immediatley send a timeout
        return true; // New input, return true
    }

    return false;
}


// USB Audio Class 1.0 request codes used by the console on this device.
#define S5_UAC_SET_CUR 0x01
#define S5_UAC_GET_CUR 0x81
#define S5_UAC_GET_MIN 0x82
#define S5_UAC_GET_MAX 0x83
#define S5_UAC_GET_RES 0x84

// Feature-unit volume ranges, as a genuine DualSense reports them.
// { entity, current, min, max, res }, all little-endian 1/256 dB.
static constexpr struct {
    uint8_t entity;
    uint8_t cur[2];
    uint8_t min[2];
    uint8_t max[2];
    uint8_t res[2];
} s5_audio_features[] = {
    { 0x02, { 0x47, 0xE6 }, { 0x80, 0x9C }, { 0x00, 0xFF }, { 0x00, 0x01 } },
    { 0x05, { 0xCC, 0xFE }, { 0xC0, 0xE8 }, { 0x00, 0x18 }, { 0xC0, 0x00 } },
};
static constexpr uint8_t S5_AUDIO_FEATURE_COUNT =
    sizeof(s5_audio_features) / sizeof(s5_audio_features[0]);

void MayflashS5Driver::resetAudioState() {
    for (uint8_t i = 0; i < 3; i++) {
        audio_alt_setting[i] = 0;
    }
    for (uint8_t i = 0; i < S5_AUDIO_FEATURE_COUNT; i++) {
        memcpy(audio_current[i], s5_audio_features[i].cur, 2);
    }
    audio_pending_index = 0xFF;
}

int8_t MayflashS5Driver::audioFeatureIndex(uint8_t entity) const {
    for (uint8_t i = 0; i < S5_AUDIO_FEATURE_COUNT; i++) {
        if (s5_audio_features[i].entity == entity) {
            return static_cast<int8_t>(i);
        }
    }
    return -1;
}

// Interfaces 0/1/2 of the composite. The console reads the feature
// unit ranges during enumeration and writes a speaker volume in the middle of
// the auth exchange, so these have to answer rather than stall.
bool MayflashS5Driver::audioControlXferCb(
        uint8_t rhport, uint8_t stage,
        tusb_control_request_t const *request) {
    if (stage != CONTROL_STAGE_SETUP) {
        if (stage == CONTROL_STAGE_ACK && audio_pending_index != 0xFF) {
            memcpy(audio_current[audio_pending_index], audio_xfer_buffer, 2);
            audio_pending_index = 0xFF;
        }
        return true;
    }

    const uint8_t itf = tu_u16_low(request->wIndex);

    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_STANDARD) {
        switch (request->bRequest) {
            case TUSB_REQ_SET_INTERFACE:
                // The audio streams are never started; only acknowledge the
                // alternate setting so the console's SET_INTERFACE succeeds.
                if (itf < 3) {
                    audio_alt_setting[itf] =
                        static_cast<uint8_t>(tu_u16_low(request->wValue));
                }
                return tud_control_status(rhport, request);
            case TUSB_REQ_GET_INTERFACE:
                audio_xfer_buffer[0] = (itf < 3) ? audio_alt_setting[itf] : 0;
                return tud_control_xfer(rhport, request, audio_xfer_buffer, 1);
            default:
                return false;
        }
    }

    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_CLASS) {
        return false;
    }

    const int8_t index = audioFeatureIndex(tu_u16_high(request->wIndex));

    if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {
        const uint8_t *value = nullptr;
        if (index >= 0) {
            switch (request->bRequest) {
                case S5_UAC_GET_CUR: value = audio_current[index]; break;
                case S5_UAC_GET_MIN: value = s5_audio_features[index].min; break;
                case S5_UAC_GET_MAX: value = s5_audio_features[index].max; break;
                case S5_UAC_GET_RES: value = s5_audio_features[index].res; break;
                default: break;
            }
        }
        if (value == nullptr) {
            memset(audio_xfer_buffer, 0, 2);
        } else {
            memcpy(audio_xfer_buffer, value, 2);
        }
        return tud_control_xfer(rhport, request, audio_xfer_buffer,
            tu_min16(request->wLength, 2));
    }

    // Host to device: accept the payload and remember it so a later GET_CUR
    // returns the console's own value.
    memset(audio_xfer_buffer, 0, sizeof(audio_xfer_buffer));
    audio_pending_index =
        (request->bRequest == S5_UAC_SET_CUR && index >= 0)
            ? static_cast<uint8_t>(index) : 0xFF;
    return tud_control_xfer(rhport, request, audio_xfer_buffer,
        tu_min16(request->wLength, sizeof(audio_xfer_buffer)));
}




bool MayflashS5Driver::controlXferCb(
        uint8_t rhport, uint8_t stage,
        tusb_control_request_t const *request) {
    // Requests aimed at the audio interfaces are answered by the audio
    // handler; everything else belongs to the HID interface.
    if (request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE &&
            tu_u16_low(request->wIndex) != S5_ITF_HID) {
        return audioControlXferCb(rhport, stage, request);
    }

    // A real DualSense stalls SET_IDLE; TinyUSB's HID class would accept it.
    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS &&
            request->bRequest == HID_REQ_CONTROL_SET_IDLE) {
        return false;
    }

    // Every feature report the console reads is answered by the dongle, live.
    //
    // The vendor documentation is explicit that the adapter processes this
    // traffic rather than answering on its own behalf: the console transmits
    // the data to the device, the device processes it, and the console reads
    // the processed result back. Nothing is cached; the reference adapter
    // forwards each request within a couple of milliseconds.
    const bool is_feature_get =
        request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS &&
        request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
        tu_u16_high(request->wValue) == HID_REPORT_TYPE_FEATURE &&
        // Report 0x81 is answered locally with a stall rather than proxied.
        // The dongle stalls it too, so answering locally is externally
        // identical and avoids a needless downstream transaction. A console
        // test proved that proxying 0x81 was not the cause of the later
        // signing failure.
        tu_u16_low(request->wValue) != PS5AuthReport::PS5_GET_TEST_PARAM;

    if (!is_feature_get) {
        return hidd_control_xfer_cb(rhport, stage, request);
    }

    if (stage == CONTROL_STAGE_SETUP) {
        // Only one proxied request can be in flight at a time. Refusing here
        // stalls the console's control request, which it retries.
        if (ps5AuthData == nullptr ||
                !ps5AuthData->dongle_ready ||
                deferred_control_pending ||
                ps5AuthData->proxy_get_pending ||
                ps5AuthData->proxy_get_inflight) {
            return false;
        }

        const uint8_t report_id = tu_u16_low(request->wValue);
        deferred_control_request = *request;
        deferred_control_pending = true;
        ps5AuthData->proxy_get_request = report_id;
        ps5AuthData->proxy_get_len =
            tu_min16(request->wLength, sizeof(ps5AuthData->proxy_response));
        ps5AuthData->proxy_get_pending = true;
        ps5AuthData->proxy_get_stalled = false;
        ps5AuthData->proxy_response_ready = false;
        ps5AuthData->proxy_response_len = 0;

        if (report_id == PS5AuthReport::PS5_GET_SIGNATURE_NONCE) {
            ps5AuthData->auth_console_f1_count++;
        } else if (report_id == PS5AuthReport::PS5_GET_SIGNING_STATE) {
            ps5AuthData->auth_console_f2_count++;
        }
    }

    // Leave EP0 without an IN transfer during SETUP. The controller NAKs
    // until serviceDeferredControl() supplies the downstream S5 response.
    return true;
}

void MayflashS5Driver::serviceDeferredControl() {
    if (!deferred_control_pending || ps5AuthData == nullptr) {
        return;
    }

    // The dongle refused the request. Pass the refusal on rather than leaving
    // the console's transfer open; report 0x81 is stalled by the real device.
    if (ps5AuthData->proxy_get_stalled) {
        ps5AuthData->proxy_get_stalled = false;
        deferred_control_pending = false;
        usbd_edpt_stall(TUD_OPT_RHPORT, 0x80);
        return;
    }

    if (!ps5AuthData->proxy_response_ready) {
        return;
    }

    const uint16_t response_len = ps5AuthData->proxy_response_len;
    if (response_len == 0 || response_len >
            sizeof(deferred_control_buffer) ||
            ps5AuthData->proxy_response[0] !=
                ps5AuthData->proxy_get_request) {
        return;
    }

    memcpy(deferred_control_buffer, ps5AuthData->proxy_response,
        response_len);

    // The console compares the reported DualSense firmware build against the
    // one bundled with its own system software, and offers a firmware update
    // instead of proceeding if it judges the controller out of date. The S5
    // reports its own, older build, so substitute a current one. Everything
    // else the dongle answers is passed through untouched.
    if (ps5AuthData->proxy_get_request == PS5AuthReport::PS5_GET_FIRWMARE &&
            response_len >= 64) {
        memcpy(&deferred_control_buffer[1], dualsense_current_0x20, 63);
    }

    if (tud_control_xfer(TUD_OPT_RHPORT, &deferred_control_request,
            deferred_control_buffer, response_len)) {
        ps5AuthData->proxy_response_ready = false;
        deferred_control_pending = false;
    }
}

void MayflashS5Driver::processAux() {
    // S5 USB host work is intentionally performed by process() on core 0.
}

USBListener * MayflashS5Driver::get_usb_auth_listener() {
    if ( mayflashS5AuthDriver != nullptr ) {
        return mayflashS5AuthDriver->getListener();
    }
    return nullptr;
}

uint16_t MayflashS5Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    //P5DPRINTF("P5D:get_report id:%02x type:%d reqlen:%d\n", report_id, report_type, reqlen);
    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        return -1;
    }
    const uint16_t traced = getReportTraced(report_id, buffer, reqlen);
    return traced;
}

uint16_t MayflashS5Driver::getReportTraced(uint8_t report_id, uint8_t *buffer, uint16_t reqlen) {

    uint16_t responseLen = 0;
    uint8_t reportBuffer[60];
    uint32_t crc32;
    switch(report_id) {
        case PS5AuthReport::PS5_DEFINITION:
            if (reqlen != sizeof(output_0x03)) return -1;
            responseLen = MAX(reqlen, sizeof(output_0x03));
            memcpy(buffer, output_0x03, responseLen);
            return responseLen;
        case PS5AuthReport::PS5_GET_CALIBRATION:
        {
            if (reqlen != sizeof(output_0x05)) return -1;
            if ( ps5AuthData->S5_reports_ready == false ) {
                memcpy(buffer, output_0x05, reqlen);
            } else {
                memcpy(buffer, ps5AuthData->calibration_report, reqlen);
            }
            return 40;
        }
        case PS5AuthReport::PS5_GET_PAIRINFO:
        {
            if (reqlen != sizeof(output_0x09)) return -1;
            if ( ps5AuthData->S5_reports_ready == false ) {
                // Never let the console cache a zero identity. A failed
                // request can be retried after the downstream report arrives.
                return -1;
            } else {
                memcpy(buffer, ps5AuthData->MAC_pair_report, reqlen);
            }
            return 15;
        }
        case PS5AuthReport::PS5_GET_FIRWMARE:
        {
            if (reqlen != sizeof(output_0x20)) return -1;
            // Substitute a current DualSense build; see serviceDeferredControl.
            memcpy(buffer, dualsense_current_0x20, reqlen);
            return 63;
        }
        case PS5AuthReport::PS5_GET_TEST_PARAM:
            // https://controllers.fandom.com/wiki/Sony_DualSense
            //P5DPRINTF("P5D:DualSense Get test command <STALL>\n");
            return -1;
        case PS5AuthReport::PS5_GET_SIGNATURE_NONCE:
            ps5AuthData->auth_console_f1_count++;
            // Preserve the S5's response type, frame/index fields and CRC
            // exactly. Reconstructing this packet causes PS5 auth rejection
            // even when all four payload chunks are present.
            memcpy(buffer, &ps5AuthData->auth_f1_raw[ps5AuthData->auth_f1_get_index][1], 63);

            P5DPRINTF("GP2040->Console F0[%i]: %02x, %02x, %02x, %02x, %02x\n", ps5AuthData->auth_f1_get_index, buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

            // in 4-block mode, we send 4 index blocks
            if ( ps5AuthData->console_f0_type == 0x01 ) {
                //P5DPRINTF("P5D: PS5_GET_SIGNATURE_NONCE retrieved index %02x\n", ps5AuthData->auth_f1_get_index);
                ps5AuthData->auth_f1_get_index++;
                if ( ps5AuthData->auth_f1_get_index == 0x04 ) { // we sent all 4 blocks
                    //P5DPRINTF("P5D: PS5_GET_SIGNATURE_NONCE DONE! PS5 will ask for status if this is good\n");
                    ps5AuthData->auth_f1_get_index = 0;
                }
            } else if ( ps5AuthData->console_f0_type == 0x02 ) {
                ps5AuthData->auth_f1_get_index++;
                if ( ps5AuthData->auth_f1_get_index == 0x04 ) { // we only send 1 block I think?
                    //P5DPRINTF("P5D: PS5_GET_SIGNATURE_NONCE DONE! PS5 will ask for status if this is good\n");
                    ps5AuthData->auth_f1_get_index = 0;
                }
            } else if ( ps5AuthData->console_f0_type == 0x03 ) {
                //P5DPRINTF("P5D: PS5_GET_SIGNATURE_NONCE Refresh done! PS5 will ask for status if this is good\n");
                ps5AuthData->auth_f1_get_index++;
                if ( ps5AuthData->auth_f1_get_index == 0x04 ) {
                    ps5AuthData->auth_f1_get_index = 0;
                }
            }

            //timeout_report_us = getMicro(); // don't need to keep timing out while we're doing this
            return 63;
        case PS5AuthReport::PS5_GET_SIGNING_STATE:
            ps5AuthData->auth_console_f2_count++;
            // We need to add a signing state for "renewing / cycling to the next packet"
            if ( ps5AuthData->auth_f1_done ) {
                //P5DPRINTF("P5D: Dongle auth is all done, ready for the next auth (F2 0x40)\n");
                if (ps5AuthData->auth_f2_valid) {
                    memcpy(buffer, &ps5AuthData->auth_f2_raw[1], 15);
                } else {
                    return -1;
                }

                // reset everything (ready for next encryption!)
                ps5AuthData->console_f0_get_index = 0;
                ps5AuthData->console_f0_recv_count = 0;
                ps5AuthData->auth_f1_get_index = 0;
                ps5AuthData->auth_f1_ready = false;
                ps5AuthData->auth_f1_done = false;
                ps5AuthData->auth_f2_valid = false;
            } else if ( ps5AuthData->auth_f1_ready ) {
                //P5DPRINTF("P5D: Dongle auth is ready! (F2 0x12)\n");
                if (ps5AuthData->auth_f2_valid) {
                    memcpy(buffer, &ps5AuthData->auth_f2_raw[1], 15);
                } else {
                    return -1;
                }

                // sneak this in
                ps5AuthData->auth_f1_get_index = 0; // start at 0
            } else {
                //P5DPRINTF("P5D: Dongle auth NOT is ready! (F2 0x11)\n");
                memset(buffer, 0, 15);
                buffer[0] = ps5AuthData->console_f0_type == 0x03
                    ? 0x01 : ps5AuthData->console_f0_type;
                buffer[1] = ps5AuthData->auth_frame_id;
                if ( ps5AuthData->console_f0_type == 0x01 ) {
                    buffer[2] = PS5AuthResponse::PS5_AUTH_NOT_READY;
                } else if ( ps5AuthData->console_f0_type == 0x02 ) {
                    buffer[2] = PS5AuthResponse::PS5_AUTH_NOT_READY;
                } else if ( ps5AuthData->console_f0_type == 0x03 ) {
                    buffer[2] = PS5AuthResponse::PS5_AUTH_REFRESH_NOT_READY; // ?? odd
                }

                reportBuffer[0] = PS5AuthReport::PS5_GET_SIGNING_STATE;
                memcpy(&reportBuffer[1], buffer, 11);
                crc32 = CRC32::calculate(reportBuffer, 12);
                memcpy(&buffer[11], &crc32, sizeof(uint32_t));
            }
            //timeout_report_us = getMicro(); // don't need to keep timing out while we're doing this
            return 15;
        default:
            //P5DPRINTF("P5D: Missed get report: %02x length: %02x\n", report_id, reqlen);
            return -1;
    }
    return -1;
}

// Copy a console feature write verbatim, report id included, for the listener
// to hand to the S5 unchanged.
void MayflashS5Driver::queueConsoleSet(uint8_t report_id,
        uint8_t const *buffer, uint16_t bufsize) {
    if (ps5AuthData == nullptr || bufsize > 63) {
        return;
    }

    // Only forward writes this console path is known to use. The PS5 now
    // offers a DualSense firmware update for this device, and a blanket
    // passthrough would carry an update payload straight to the S5. Anything
    // unexpected is recorded and dropped rather than handed to the dongle.
    switch (report_id) {
        case PS5AuthReport::PS5_SET_BLUETOOTH:    // 0x08
        case PS5AuthReport::PS5_SET_TEST_PARAM:   // 0x80
        // Pairing data is allowed through even in wired mode. The console
        // appears to treat pairing as mandatory: with 0x0A withheld it asks
        // for bluetooth ON six times, never registers the controller, never
        // challenges, and grants no input, even while receiving 2,496 valid
        // signed reports. Letting it pair while keeping bluetooth switched off
        // is the state a real DualSense is in when wired.
        case 0x0A:                                // bluetooth pairing data
            break;
        default:
            return;
    }
    const uint8_t next =
        static_cast<uint8_t>((ps5AuthData->set_queue_head + 1) % PS5_SET_QUEUE_DEPTH);
    if (next == ps5AuthData->set_queue_tail) {
        ps5AuthData->set_queue_dropped++;
        return;
    }
    PS5PendingSet &slot = ps5AuthData->set_queue[ps5AuthData->set_queue_head];
    slot.report_id = report_id;
    slot.len = static_cast<uint16_t>(bufsize + 1);
    slot.data[0] = report_id;
    memcpy(&slot.data[1], buffer, bufsize);
    ps5AuthData->set_queue_head = next;
}

void MayflashS5Driver::setFirstConsoleF0(ConsolePS5AuthBuffer * authBuffer) { // helper
    ps5AuthData->console_f0_get_index = 0;
    ps5AuthData->console_f0_recv_count = 1;
    ps5AuthData->auth_frame_id = authBuffer->frame_id;
    ps5AuthData->console_f0_type = authBuffer->auth_type; // 4-chunk block type
    ps5AuthData->auth_f2_valid = false;
    ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_send_f0_from_console;
}

void MayflashS5Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    //P5DPRINTF("P5D:set_report id:%02x type:%d reqlen:%d\n", report_id, report_type, bufsize);
    if ( report_type != HID_REPORT_TYPE_FEATURE ) {
        //P5DPRINTF("P5D:not a feature, nothing to do...\n");
        return;
    }


    ConsolePS5AuthBuffer * authBuffer = nullptr;

    switch( report_id ) {
        case PS5AuthReport::PS5_SET_BLUETOOTH:
            ps5AuthData->set_bluetooth_mode = bufsize > 0 ? buffer[0] : 0;
            // Report 0x08 is a bluetooth on/off switch:
            //
            //     struct ReportFeatureOutBluetooth {
            //         uint8_t ReportID;   // 0x08
            //         uint8_t State;      // 1 = ON, 2 = OFF
            //     };
            //
            // Forwarded verbatim. With the console's radio off it asks for OFF
            // and the dongle signs over USB; with the radio on it asks for ON,
            // the dongle pairs, and it delivers to the console over the air.
            // Both are working modes, so the console's request is honoured
            // rather than second-guessed.
            queueConsoleSet(report_id, buffer, bufsize);
            break;
        case PS5AuthReport::PS5_SET_TEST_PARAM:
            if (bufsize >= 2) {
                ps5AuthData->set_testcommand[0] = buffer[0];
                ps5AuthData->set_testcommand[1] = buffer[1];
            }
            queueConsoleSet(report_id, buffer, bufsize);
            break;
        case PS5AuthReport::PS5_SET_AUTH_PAYLOAD:
            // Count every F0 callback before validating it. This distinguishes
            // "the console never sent completion" from "we rejected it".
            ps5AuthData->auth_console_f0_raw_count++;
            if (bufsize > 0) {
                ps5AuthData->auth_console_f0_last_type = buffer[0];
            }
            if (bufsize != 63) {
                //P5DPRINTF("P5D:Wrong size for PS5_SET_AUTH_PAYLOAD\n");
                break;
            }

            if ( ps5AuthData->hash_pending == true &&
                    ((PS5Report*)(&ps5AuthData->hash_pending_buffer[0]))->button_home) {
               P5DPRINTF("P5D:Skip auth while PS home is held\n");
               break;
            }

            authBuffer = (ConsolePS5AuthBuffer*)buffer;
            if (authBuffer->auth_index >= 4 ||
                    (authBuffer->auth_type != 0x01 &&
                     authBuffer->auth_type != 0x02 &&
                     authBuffer->auth_type != 0x03)) {
                P5DPRINTF("P5D:Invalid auth type/index\n");
                break;
            }
            ps5AuthData->auth_console_f0_count++;
            ps5AuthData->auth_console_f0_accept_mask |=
                static_cast<uint8_t>(1u << authBuffer->auth_index);
            uint8_t f0_with_id[64];
            f0_with_id[0] = PS5AuthReport::PS5_SET_AUTH_PAYLOAD;
            memcpy(&f0_with_id[1], buffer, 63);
            uint32_t received_crc;
            memcpy(&received_crc, &f0_with_id[60], sizeof(received_crc));
            if (CRC32::calculate(f0_with_id, 60) == received_crc) {
                ps5AuthData->auth_console_f0_crc_valid_count++;
            }
            ps5AuthData->auth_f0_raw[authBuffer->auth_index][0] =
                PS5AuthReport::PS5_SET_AUTH_PAYLOAD;
            memcpy(&ps5AuthData->auth_f0_raw[authBuffer->auth_index][1],
                buffer, 63);
            //P5DPRINTF("P5D:Getting PS5 F0 with 4-blocks (frame_id %02x auth_type %02x auth_index %02x)\n", authBuffer->frame_id, authBuffer->auth_type, authBuffer->auth_index);

            // Copy the data to our console F0 buffer
            memcpy(&ps5AuthData->console_f0_buffer[authBuffer->auth_index*PS5_AUTH_DATALEN], authBuffer->auth_data, PS5_AUTH_DATALEN);

            P5DPRINTF("Console->GP2040 F0[%i]: \n    F0 ", authBuffer->auth_index);
            for(int i = 0 ; i < 63; i++ ) {
                P5DPRINTF("%02x ", buffer[i]);
            }
            P5DPRINTF("\n");

            if ( authBuffer->auth_type == 0x01 ) { // Setup initial Auth
                if ( authBuffer->auth_index == 0 ) { // START (4 blocks incoming)
                    ps5AuthData->auth_f1_ready = false;
                    ps5AuthData->auth_f1_done = false; // set our F1 auth to not done!
                    ps5AuthData->console_f0_get_index = 0;
                    ps5AuthData->console_f0_recv_count = 1;
                    ps5AuthData->auth_frame_id = authBuffer->frame_id;
                    ps5AuthData->console_f0_type = authBuffer->auth_type;
                    ps5AuthData->auth_f2_valid = false;
                    // Each authentication block is forwarded as it arrives.
                    ps5AuthData->ps5_auth_state =
                        PS5AuthState::ps5_auth_send_f0_from_console;
                } else {
                    ps5AuthData->console_f0_recv_count =
                        authBuffer->auth_index + 1;
                    if (ps5AuthData->console_f0_get_index <
                            ps5AuthData->console_f0_recv_count) {
                        ps5AuthData->ps5_auth_state =
                            PS5AuthState::ps5_auth_send_f0_from_console;
                    }
                }        
            } else if ( authBuffer->auth_type == 0x02) {
                // Wait for the S5's F2 0x40 response before reporting done.
                ps5AuthData->auth_f1_done = false;
                setFirstConsoleF0(authBuffer);
            } else if ( authBuffer->auth_type == 0x03) {
                ps5AuthData->auth_f1_ready = false;
                ps5AuthData->auth_f1_done = false; // don't send done until we're done?
                setFirstConsoleF0(authBuffer);
            }
            break;
        default:
            // Anything else the console writes is forwarded untouched,
            // including report 0x0A, which carries bluetooth pairing material.
            queueConsoleSet(report_id, buffer, bufsize);
            break;
    };
}


const uint16_t * MayflashS5Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const char *value = (const char*)mayflashs5_string_descriptors[index];
    return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * MayflashS5Driver::get_descriptor_device_cb() {
    return mayflashs5_device_descriptor;
}

const uint8_t * MayflashS5Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    return mayflashs5_custom_report_descriptor;
}

const uint8_t * MayflashS5Driver::get_descriptor_configuration_cb(uint8_t index) {
    return mayflashs5_full_configuration_descriptor;
}


uint16_t MayflashS5Driver::GetJoystickMidValue() {
    return PS5_JOYSTICK_MID << 8;
}
