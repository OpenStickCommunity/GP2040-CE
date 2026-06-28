#include "hosts/DualshockPS4Host.h"

#include "storagemanager.h"

// Static match function
bool DualshockPS4Host::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Victrix Pro FS
    
    // Nacon Revolution Pro 3
    
    // Hori (PS4)
    if ( vendor_id == 0x0F0D ) {
        switch(product_id) {
            // Horipad FPS Plus?
            // Hori Fighting Commander
            case 0x00EE:
                return true;
        }
    }

    // Razer (PS4)
    if ( vendor_id == 0x1532 ) {
        switch(product_id) {
            case 0x0401: // Razer Panthera
            case 0x1008: // Razer Panthera Evo (PS4 mode)
                return true;
        }
    }

    // Sony (Dualshock4 variations)
    if ( vendor_id == 0x054C) {
        switch(product_id) {
            case 0x05C4: // [CUH-ZCT1x]
            case 0x09CC: // [CUH-ZCT2x]
                return true;
        }
    }

    // Logitech G29
    if ( vendor_id == 0x46d) {
        switch(product_id) {
            case 0xC260:
                return true;
        }
    }

    // Thrustmaster T.Flight Racing Wheel
    if ( vendor_id == 0x044F) {
        switch(product_id) {
            case 0xB67B:
                return true;
        }
    }

    return false;
}

// We can just force it to assume its a dualshock, why do we care about definition?
void DualshockPS4Host::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
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

    memset(&prevReport, 0, sizeof(PS4Report));

    validPS4Definition = false; // check the report def

    // Find the report for PS4 definition if this controller has one
    tuh_hid_report_info_t report_info[4];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, 4, desc_report, desc_len);
    for(uint8_t i = 0; i < report_count; i++) {
#ifdef GAMEPAD_HOST_DEBUG
        //printf("Report: %02x, Usage: %04x, Usage Page: %04x\n", report_info[i].report_id, report_info[i].usage_page, report_info[i].usage);
#endif
        if (report_info[i].report_id == PS4AuthReport::PS4_DEFINITION) {
            // controller is some other type that's not a DS4, so parse the config
            memset(report_buffer, 0, PS4_ENDPOINT_SIZE);
            report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
            tuh_hid_get_report(dev_addr, instance, PS4AuthReport::PS4_DEFINITION, HID_REPORT_TYPE_FEATURE, report_buffer, 48);
            break;
        }
    }
}

void DualshockPS4Host::update() {
    if ( validPS4Definition == false ) return;

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    PS4FeatureOutputReport controller_output; // this might have to be static
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

    tuh_hid_send_report(_dev_addr, _instance, PS4AuthReport::PS4_SET_FEATURE_STATE, (uint8_t*)report+1, report_size);
}

void DualshockPS4Host::process(uint8_t const* report, uint16_t len) {
    PS4Report controller_report;

    // If its an input report
    if (report[0] == 1) {
        memcpy(&controller_report, report, sizeof(controller_report));

        if ( diff_report(&prevReport, &controller_report) ) {
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
        
        // Save report to previous
        memcpy(&prevReport, &controller_report, sizeof(PS4Report));
    }
}

void DualshockPS4Host::gamepad(Gamepad * gamepad) {
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

void DualshockPS4Host::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    memcpy(&ds4Config, report_buffer+1, sizeof(PS4ControllerConfig));
    if ((ds4Config.hidUsage == 0x2721) || (ds4Config.hidUsage == 0x2127)) {
        validPS4Definition = true;
        #ifdef GAMEPAD_HOST_DEBUG
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

uint32_t DualshockPS4Host::map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//check if different than 2
bool DualshockPS4Host::diff_than_2(uint8_t x, uint8_t y) {
    return (x - y > 2) || (y - x > 2);
}

// check if 2 reports are different enough
bool DualshockPS4Host::diff_report(PS4Report const* rpt1, PS4Report const* rpt2) {
    bool result;

    // x, y, z, rz must different than 2 to be counted
    result = diff_than_2(rpt1->leftStickX, rpt2->leftStickX) || diff_than_2(rpt1->leftStickY, rpt2->leftStickY) ||
            diff_than_2(rpt1->rightStickX, rpt2->rightStickX) || diff_than_2(rpt1->rightStickY, rpt2->rightStickY);

    // check the rest with mem compare
    result |= memcmp(&rpt1->rightStickY + 1, &rpt2->rightStickY + 1, sizeof(PS4Report)-6);

    return result;
}
