#include "hosts/DualshockPS4Host.h"

#include "drivermanager.h"
#include "storagemanager.h"

// Static match function
bool DualshockPS4Host::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Qanba Obisidian
    if ( vendor_id == 0x2C22 ) {
        switch(product_id) {
            case 0x2200: // crystal
            case 0x2500: // dragon
            case 0x2000: // drone
            case 0x2302: // obsidian
            case 0x2702: // obsidian 2
                return true;
        }
    }
    
    // Nacon Daija
    if ( vendor_id == 0x146B ) {
        switch(product_id) {
            case 0x0D09: // PS4 mode
                return true;
        }
    }
    
    // Hori (PS4)
    if ( vendor_id == 0x0F0D ) {
        switch(product_id) {
            case 0x008A: // we need someone to confirm this, real arcade pro V hayabusa ps4 mode
            case 0x005E:
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
    memset(&last_controller_output, 0, sizeof(PS4FeatureOutputReport));

    tuh_hid_send_report(_dev_addr, _instance, PS4AuthReport::PS4_DEFINITION, &report_buffer, 64);
}

void DualshockPS4Host::update() {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    PS4FeatureOutputReport controller_output; // this might have to be static
    memset(&controller_output, 0, sizeof(controller_output));
    controller_output.reportID = PS4AuthReport::PS4_SET_FEATURE_STATE;

    InputMode inputMode = DriverManager::getInstance().getInputMode();
    if (inputMode == INPUT_MODE_PS4 || inputMode == INPUT_MODE_PS5) {
        controller_output.enableUpdateLED = gamepad->auxState.sensors.statusLight.enabled;
        controller_output.ledRed = gamepad->auxState.sensors.statusLight.color.red;
        controller_output.ledGreen = gamepad->auxState.sensors.statusLight.color.green;
        controller_output.ledBlue = gamepad->auxState.sensors.statusLight.color.blue;
        if ( controller_output.ledBlinkOn > 0 || controller_output.ledBlinkOff > 0 ) {
            controller_output.enableUpdateLEDBlink = true;
            controller_output.ledBlinkOn = gamepad->auxState.playerID.ledBlinkOn; // time
            controller_output.ledBlinkOff = gamepad->auxState.playerID.ledBlinkOff; // time
        }
    } else if ( inputMode == INPUT_MODE_XINPUT || inputMode == INPUT_MODE_XBONE || inputMode == INPUT_MODE_XBOXORIGINAL ) {
        controller_output.enableUpdateLED = true;
        controller_output.ledGreen = UINT8_MAX/2;
    } else if ( inputMode == INPUT_MODE_SWITCH || inputMode == INPUT_MODE_SWITCH_PRO ) {
        controller_output.enableUpdateLED = true;
        controller_output.ledRed = UINT8_MAX/2;
    }

    gamepad->auxState.haptics.leftActuator.enabled = true;
    gamepad->auxState.haptics.rightActuator.enabled = true;
    controller_output.enableUpdateRumble = true;
    controller_output.rumbleLeft = gamepad->auxState.haptics.leftActuator.intensity;
    controller_output.rumbleRight = gamepad->auxState.haptics.rightActuator.intensity;

    if ( memcmp(&controller_output, &last_controller_output, sizeof(PS4FeatureOutputReport)) != 0 ) {
        tuh_hid_send_report(_dev_addr, _instance, 0, reinterpret_cast<const uint8_t*>(&controller_output), sizeof(controller_output));
        memcpy(&last_controller_output, &controller_output, sizeof(PS4FeatureOutputReport));
    }
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
