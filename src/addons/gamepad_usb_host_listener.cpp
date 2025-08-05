#include "addons/gamepad_usb_host_listener.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"

void GamepadUSBHostListener::setup() {
    _controller_host_enabled = false;
#if GAMEPAD_HOST_DEBUG
    stdio_init_all();
#endif
}

void GamepadUSBHostListener::process() {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = true;
    gamepad->hasLeftAnalogStick = true;
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

void GamepadUSBHostListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    _controller_host_enabled = true;
    _controller_dev_addr = dev_addr;
    _controller_instance = instance;
    tuh_vid_pid_get(dev_addr, &controller_vid, &controller_pid);

#if GAMEPAD_HOST_DEBUG
    //printf("Mount: VID_%04x PID_%04x\n", controller_vid, controller_pid);
#endif

    uint16_t joystick_mid = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.buttons = 0;
    _controller_host_state.dpad = 0;
    _controller_host_state.lx = joystick_mid;
    _controller_host_state.ly = joystick_mid;
    _controller_host_state.rx = joystick_mid;
    _controller_host_state.ry = joystick_mid;

    switch(controller_pid)
    {
        /* PS4/5 */
        // these require initialization
        case PS4_PRODUCT_ID:       // Razer Panthera
        case 0x00EE:               // Hori Minipad
        case PS4_WHEEL_PRODUCT_ID: // G29
        case 0xB67B:               // T-Flight
            init_ds4(desc_report, desc_len);
            break;
        // while these do not
        case DS4_ORG_PRODUCT_ID:   // Sony Dualshock 4 controller
        case DS4_PRODUCT_ID:       // Sony Dualshock 4 controller
            isDS4Identified = true;
            setup_ds4();
            break;
        case 0x0CE6:               // DualSense
            break;

        case 0xC294:               // Driving Force or similar
            isDFInit = false;
            setup_df_wheel();
            break;
        case 0xC29A:
            isDFInit = true;
            break;

        /* Other */
        // these types do not have an identification step, at least for PS4
        case 0x9400:               // Google Stadia controller
        case 0x0510:               // pre-2015 Ultrakstik 360
        case 0x0511:               // Ultrakstik 360
        default:
            break;
    }
}

void GamepadUSBHostListener::unmount(uint8_t dev_addr) {
    _controller_host_enabled = false;
    controller_pid = 0x00;
    controller_vid = 0x00;
    _controller_dev_addr = 0;
    _controller_instance = 0;
    isDS4Identified = false;
    hasDS4DefReport = false;
}

void GamepadUSBHostListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // if a hid device hasn't been mounted
    if ( _controller_host_enabled == false ) return;

    // Interface protocol (hid_interface_protocol_enum_t)
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    // stop execution if a keyboard or mouse is mounted
    if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) return;

    process_ctrlr_report(dev_addr, report, len);
}

void GamepadUSBHostListener::process_ctrlr_report(uint8_t dev_addr, uint8_t const* report, uint16_t len) {
#if GAMEPAD_HOST_DEBUG
    //printf("\033[1;0H\nHost (%d):\n", len);
    //for (uint8_t i = 0; i < len; i++) {
    //    printf("%02x ", report[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("----\n");
#endif

    switch(controller_pid)
    {
        case DS4_ORG_PRODUCT_ID:   // Sony Dualshock 4 controller
        case DS4_PRODUCT_ID:       // Sony Dualshock 4 controller
        case PS4_PRODUCT_ID:       // Razer Panthera
        case PS4_WHEEL_PRODUCT_ID: // G29
        case 0xB67B:               // T-Flight
        case 0x00EE:               // Hori Minipad
            if (isDS4Identified) {
                update_ds4();
                process_ds4(report, len);
            }
            break;
        case 0x0CE6:               // DualSense
            process_ds(report, len);
            break;
        case 0x9400:               // Google Stadia controller
            process_stadia(report, len);
            break;

        case 0xC294:               // Driving Force
            if (!isDFInit) setup_df_wheel();
            break;

        case 0xC29A:
            process_dfgt(report, len);
            break;

        case 0x0510:               // pre-2015 Ultrakstik 360
        case 0x0511:               // Ultrakstik 360
            process_ultrastik360(report, len);
            break;
        default:
            break;
    }
}

bool GamepadUSBHostListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_get_report(_controller_dev_addr, _controller_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool GamepadUSBHostListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_set_report(_controller_dev_addr, _controller_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

void GamepadUSBHostListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    awaiting_cb = false;
}

void GamepadUSBHostListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
#if GAMEPAD_HOST_DEBUG
    //printf("get_report_complete Report ID: %02x\n", report_id);
#endif
    if (!isDS4Identified) {
        switch (report_id) {
            case PS4AuthReport::PS4_DEFINITION:
                setup_ds4();
                break;
            default: 
                break;
        }
    }
    //
    awaiting_cb = false;
}

uint16_t GamepadUSBHostListener::map(uint8_t x, uint8_t in_min, uint8_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


//check if different than 2
bool GamepadUSBHostListener::diff_than_2(uint8_t x, uint8_t y) {
    return (x - y > 2) || (y - x > 2);
}

// check if 2 reports are different enough
bool GamepadUSBHostListener::diff_report(PS4Report const* rpt1, PS4Report const* rpt2) {
    bool result;

    // x, y, z, rz must different than 2 to be counted
    result = diff_than_2(rpt1->leftStickX, rpt2->leftStickX) || diff_than_2(rpt1->leftStickY, rpt2->leftStickY) ||
            diff_than_2(rpt1->rightStickX, rpt2->rightStickX) || diff_than_2(rpt1->rightStickY, rpt2->rightStickY);

    // check the rest with mem compare
    result |= memcmp(&rpt1->rightStickY + 1, &rpt2->rightStickY + 1, sizeof(PS4Report)-6);

    return result;
}

void GamepadUSBHostListener::setup_ds4() {
    if (hasDS4DefReport) {
        // report came from the controller so copy the buffer
        memcpy(&ds4Config, report_buffer+1, sizeof(PS4ControllerConfig));
    }
    if ((ds4Config.hidUsage == 0x2721) || (ds4Config.hidUsage == 0x2127)) {
        isDS4Identified = true;
#if GAMEPAD_HOST_DEBUG
        //printf("PS4 controller details\n");
        //printf("----------------------\n");
        //printf("enableController: %d\n", ds4Config.features.enableController);
        //printf("enableMotion: %d\n", ds4Config.features.enableMotion);
        //printf("enableLED: %d\n", ds4Config.features.enableLED);
        //printf("enableRumble: %d\n", ds4Config.features.enableRumble);
        //printf("enableAnalog: %d\n", ds4Config.features.enableAnalog);
        //printf("enableUnknown0: %d\n", ds4Config.features.enableUnknown0);
        //printf("enableTouchpad: %d\n", ds4Config.features.enableTouchpad);
        //printf("enableUnknown1: %d\n", ds4Config.features.enableUnknown1);
#endif
    }
}

void GamepadUSBHostListener::init_ds4(const uint8_t* descReport, uint16_t descLen) {
    isDS4Identified = false;

    tuh_hid_report_info_t report_info[4];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, 4, descReport, descLen);
    for(uint8_t i = 0; i < report_count; i++) {
#if GAMEPAD_HOST_DEBUG
        //printf("Report: %02x, Usage: %04x, Usage Page: %04x\n", report_info[i].report_id, report_info[i].usage_page, report_info[i].usage);
#endif
        if (report_info[i].report_id == PS4AuthReport::PS4_DEFINITION) {
            // controller is some other type that's not a DS4, so parse the config
            memset(report_buffer, 0, PS4_ENDPOINT_SIZE);
            report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
            host_get_report(PS4AuthReport::PS4_DEFINITION, report_buffer, 48);
            hasDS4DefReport = true;
            break;
        }
    }
    
    if (!hasDS4DefReport) {
        // no report found, DS4 or clone assume. use struct default data.
        //isDS4Identified = true;
    }
}

void GamepadUSBHostListener::update_ds4() {
#if GAMEPAD_HOST_USE_FEATURES
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    PS4FeatureOutputReport controller_output;

    memset(&controller_output, 0, sizeof(controller_output));

    controller_output.reportID = PS4AuthReport::PS4_SET_FEATURE_STATE;

    if (ds4Config.features.enableLED && gamepad->auxState.sensors.statusLight.enabled) {
        controller_output.enableUpdateLED = gamepad->auxState.sensors.statusLight.enabled;
        controller_output.ledRed = gamepad->auxState.sensors.statusLight.color.red;
        controller_output.ledGreen = gamepad->auxState.sensors.statusLight.color.green;
        controller_output.ledBlue = gamepad->auxState.sensors.statusLight.color.blue;
        controller_output.ledBlinkOn = gamepad->auxState.playerID.ledBlinkOn;
        controller_output.ledBlinkOff = gamepad->auxState.playerID.ledBlinkOff;
    }

    if (ds4Config.features.enableRumble) {
        gamepad->auxState.haptics.leftActuator.enabled = 1;
        gamepad->auxState.haptics.rightActuator.enabled = 1;
        controller_output.enableUpdateRumble = 1;
        controller_output.rumbleLeft = gamepad->auxState.haptics.leftActuator.intensity;
        controller_output.rumbleRight = gamepad->auxState.haptics.rightActuator.intensity;
    }

    void * report = &controller_output;
    uint16_t report_size = sizeof(controller_output)-1;

    tuh_hid_send_report(_controller_dev_addr, _controller_instance, 5, report+1, report_size);
#endif
}

void GamepadUSBHostListener::process_ds4(uint8_t const* report, uint16_t len) {
    PS4Report controller_report;

    // previous report used to compare for changes
    static PS4Report prev_report = { 0 };

    uint8_t const report_id = report[0];

    if (report_id == 1) {
        memcpy(&controller_report, report, sizeof(controller_report));

        if ( diff_report(&prev_report, &controller_report) ) {
            _controller_host_state.lx = map(controller_report.leftStickX, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ly = map(controller_report.leftStickY, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.rx = map(controller_report.rightStickX,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ry = map(controller_report.rightStickY,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.lt = controller_report.leftTrigger;
            _controller_host_state.rt = controller_report.rightTrigger;

            _controller_host_state.buttons = 0;
            if (controller_report.buttonTouchpad) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
            if (controller_report.buttonSelect) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
            if (controller_report.buttonR3) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
            if (controller_report.buttonL3) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
            if (controller_report.buttonHome) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
            if (controller_report.buttonStart) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
            if (controller_report.buttonR1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
            if (controller_report.buttonL1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
            if (controller_report.buttonNorth) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
            if (controller_report.buttonEast) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
            if (controller_report.buttonSouth) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
            if (controller_report.buttonWest) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
            if (controller_report.buttonR2) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
            if (controller_report.buttonL2) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

            _controller_host_state.dpad = 0;
            if (controller_report.dpad == PS4_HAT_UP) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
            if (controller_report.dpad == PS4_HAT_UPRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_RIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_DOWNRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWN) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWNLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_LEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_UPLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
        }
    }

    prev_report = controller_report;
}

void GamepadUSBHostListener::process_ds(uint8_t const* report, uint16_t len) {
    DSReport controller_report;

    // previous report used to compare for changes
    static DSReport prev_ds_report = { 0 };

    uint8_t const report_id = report[0];

    if (report_id == 1) {
        memcpy(&controller_report, report, sizeof(controller_report));

        if ( prev_ds_report.reportCounter != controller_report.reportCounter ) {
            _controller_host_state.lx = map(controller_report.leftStickX, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ly = map(controller_report.leftStickY, 0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.rx = map(controller_report.rightStickX,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.ry = map(controller_report.rightStickY,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            _controller_host_state.lt = controller_report.leftTrigger;
            _controller_host_state.rt = controller_report.rightTrigger;

            _controller_host_state.buttons = 0;
            if (controller_report.buttonTouchpad) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
            if (controller_report.buttonSelect) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
            if (controller_report.buttonR3) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
            if (controller_report.buttonL3) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
            if (controller_report.buttonHome) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
            if (controller_report.buttonStart) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
            if (controller_report.buttonR1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
            if (controller_report.buttonL1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
            if (controller_report.buttonNorth) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
            if (controller_report.buttonEast) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
            if (controller_report.buttonSouth) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
            if (controller_report.buttonWest) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
            if (controller_report.buttonR2) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
            if (controller_report.buttonL2) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

            _controller_host_state.dpad = 0;
            if (controller_report.dpad == PS4_HAT_UP) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
            if (controller_report.dpad == PS4_HAT_UPRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_RIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
            if (controller_report.dpad == PS4_HAT_DOWNRIGHT) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWN) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
            if (controller_report.dpad == PS4_HAT_DOWNLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_LEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
            if (controller_report.dpad == PS4_HAT_UPLEFT) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
        }
    }

    prev_ds_report = controller_report;
}

void GamepadUSBHostListener::process_stadia(uint8_t const* report, uint16_t len) {
    google_stadia_report_t controller_report;

    memcpy(&controller_report, report, sizeof(controller_report));

    _controller_host_state.lx = map(controller_report.GD_GamePadPointerX ,1,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ly = map(controller_report.GD_GamePadPointerY,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.rx = map(controller_report.GD_GamePadPointerZ,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ry = map(controller_report.GD_GamePadPointerRz,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.lt = controller_report.SIM_GamePadBrake;
    _controller_host_state.rt = controller_report.SIM_GamePadAccelerator;

    if (controller_report.BTN_GamePadButton18 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
    if (controller_report.BTN_GamePadButton17 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A3;
    if (controller_report.BTN_GamePadButton11 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
    if (controller_report.BTN_GamePadButton15 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
    if (controller_report.BTN_GamePadButton14 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
    if (controller_report.BTN_GamePadButton13 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
    if (controller_report.BTN_GamePadButton12 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
    if (controller_report.BTN_GamePadButton8 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.BTN_GamePadButton7 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.BTN_GamePadButton5 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
    if (controller_report.BTN_GamePadButton4 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.BTN_GamePadButton2 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.BTN_GamePadButton1 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.BTN_GamePadButton19 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
    if (controller_report.BTN_GamePadButton20 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

    if (controller_report.GD_GamePadHatSwitch == 0) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
    if (controller_report.GD_GamePadHatSwitch == 1) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
    if (controller_report.GD_GamePadHatSwitch == 2) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
    if (controller_report.GD_GamePadHatSwitch == 3) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
    if (controller_report.GD_GamePadHatSwitch == 4) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
    if (controller_report.GD_GamePadHatSwitch == 5) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
    if (controller_report.GD_GamePadHatSwitch == 6) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
    if (controller_report.GD_GamePadHatSwitch == 7) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
}

void GamepadUSBHostListener::setup_df_wheel() {
    // send commands to see if can be reset to Driving Force GT mode for more compatibility
    uint8_t command[8] = {0xF8, 0x09, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00};
    uint16_t commandSize = sizeof(command);

    if (tuh_hid_send_report(_controller_dev_addr, _controller_instance, 0, command, commandSize)) {
        isDFInit = true;
    }
}

void GamepadUSBHostListener::process_dfgt(uint8_t const* report, uint16_t len) {
    PS3ReportAlt ps3Report;
    memcpy(&ps3Report, report, len);
#if GAMEPAD_HOST_DEBUG
    //printf("\033[2;0H");
    //for (uint8_t i = 0; i < len; i++) {
    //    printf("%02x ", report[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("\033[2;0H Ste:%5d Gas:%3d Brk:%3d", ps3Report.wheel.steeringWheel, ps3Report.wheel.gasPedal, ps3Report.wheel.brakePedal);
    //printf("\n-----\n");
    //printf("\033[2;0HDPad: %1d", ps3Report.wheel.dpadDirection);
    //printf("\033[3;0HWheel: %5d", ps3Report.wheel.steeringWheel);
    //printf("\033[4;0HGas: %3d", ps3Report.wheel.gasPedal);
    //printf("\033[5;0HBrake: %3d", ps3Report.wheel.brakePedal);
#endif
}

void GamepadUSBHostListener::process_ultrastik360(uint8_t const* report, uint16_t len) {

    ultrastik360_t controller_report;

    memcpy(&controller_report, report, sizeof(controller_report));

    _controller_host_state.lx = map(controller_report.GD_GamePadPointerX, 0, 255, GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ly = map(controller_report.GD_GamePadPointerY, 0, 255, GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);

    if (controller_report.BTN_GamePadButton1 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.BTN_GamePadButton2 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.BTN_GamePadButton3 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.BTN_GamePadButton4 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
    if (controller_report.BTN_GamePadButton5 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.BTN_GamePadButton6 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L2;
    if (controller_report.BTN_GamePadButton7 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.BTN_GamePadButton8 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
}
