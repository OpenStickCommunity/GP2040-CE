/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps3/PS3Driver.h"
#include "drivers/ps3/PS3Descriptors.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"
#include "pico/rand.h"

void PS3Driver::initialize() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const GamepadOptions & options = gamepad->getOptions();

    // set up device descriptor IDs depending on mode
    uint8_t descSize = sizeof(ps3_device_descriptor);
    memcpy(deviceDescriptor, &ps3_device_descriptor, descSize);

    deviceType = options.inputDeviceType;

    if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_WHEEL) {
        // wheel
        deviceDescriptor[8] = LSB(PS3_WHEEL_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS3_WHEEL_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS3_WHEEL_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS3_WHEEL_PRODUCT_ID);

        buttonShiftUp = new GamepadButtonMapping(0);
        buttonShiftDown = new GamepadButtonMapping(0);
        buttonGas = new GamepadButtonMapping(0);
        buttonBrake = new GamepadButtonMapping(0);
        buttonSteerLeft = new GamepadButtonMapping(0);
        buttonSteerRight = new GamepadButtonMapping(0);
        buttonPlus = new GamepadButtonMapping(0);
        buttonMinus = new GamepadButtonMapping(0);
        buttonDialDown = new GamepadButtonMapping(0);
        buttonDialUp = new GamepadButtonMapping(0);
        buttonDialEnter = new GamepadButtonMapping(0);
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GUITAR) {
        // guitar
        deviceDescriptor[8] = LSB(PS3_GUITAR_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS3_GUITAR_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS3_GUITAR_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS3_GUITAR_PRODUCT_ID);

        buttonFretGreen = new GamepadButtonMapping(0);
        buttonFretRed = new GamepadButtonMapping(0);
        buttonFretYellow = new GamepadButtonMapping(0);
        buttonFretBlue = new GamepadButtonMapping(0);
        buttonFretOrange = new GamepadButtonMapping(0);
        buttonWhammy = new GamepadButtonMapping(0);
        buttonPickup = new GamepadButtonMapping(0);
        buttonTilt = new GamepadButtonMapping(0);
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_DRUM) {
        // drum
        deviceDescriptor[8] = LSB(PS3_DRUM_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS3_DRUM_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS3_DRUM_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS3_DRUM_PRODUCT_ID);

        buttonDrumPadRed = new GamepadButtonMapping(0);
        buttonDrumPadBlue = new GamepadButtonMapping(0);
        buttonDrumPadYellow = new GamepadButtonMapping(0);
        buttonDrumPadGreen = new GamepadButtonMapping(0);
        buttonCymbalYellow = new GamepadButtonMapping(0);
        buttonCymbalBlue = new GamepadButtonMapping(0);
        buttonCymbalGreen = new GamepadButtonMapping(0);
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD_ALT) {
        deviceDescriptor[8] = LSB(PS3_ALT_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS3_ALT_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS3_ALT_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS3_ALT_PRODUCT_ID);
    } else {
        // assume gamepad if not special cased
        deviceDescriptor[8] = LSB(PS3_VENDOR_ID);
        deviceDescriptor[9] = MSB(PS3_VENDOR_ID);
        deviceDescriptor[10] = LSB(PS3_PRODUCT_ID);
        deviceDescriptor[11] = MSB(PS3_PRODUCT_ID);
    }

    ps3Report = {
        .reportID = 1,
        .reserved = 0,
        .buttonSelect = 0, .buttonL3 = 0, .buttonR3 = 0, .buttonStart = 0,
        .dpadUp = 0, .dpadRight = 0, .dpadDown = 0, .dpadLeft = 0,
        .buttonL2 = 0, .buttonR2 = 0, .buttonL1 = 0, .buttonR1 = 0,
        .buttonNorth = 0, .buttonEast = 0, .buttonSouth = 0, .buttonWest = 0,
        .buttonPS = 0, .buttonTP = 0,
        .leftStickX = PS3_JOYSTICK_MID,
        .leftStickY = PS3_JOYSTICK_MID,
        .rightStickX = PS3_JOYSTICK_MID,
        .rightStickY = PS3_JOYSTICK_MID,
        .movePowerStatus = 0,
        .dpadUpAnalog = 0x00, .dpadRightAnalog = 0x00, .dpadDownAnalog = 0x00, .dpadLeftAnalog = 0x00,
        .buttonL2Analog = 0x00, .buttonR2Analog = 0x00, .buttonL1Analog = 0x00, .buttonR1Analog = 0x00,
        .buttonNorthAnalog = 0x00, .buttonEastAnalog = 0x00, .buttonSouthAnalog = 0x00, .buttonWestAnalog = 0x00,
        .reserved2 = {0x00,0x00,0x00},
        .plugged = PS3PluggedInState::PS3_PLUGGED,
        .powerStatus = PS3PowerState::PS3_POWER_FULL,
        .rumbleStatus = PS3WiredState::PS3_WIRED_RUMBLE,
        .reserved3 = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        .accelerometerX = PS3_CENTER_SIXAXIS, .accelerometerY = PS3_CENTER_SIXAXIS, .accelerometerZ = PS3_CENTER_SIXAXIS,
        .gyroscopeZ = PS3_CENTER_SIXAXIS,
        .reserved4 = PS3_CENTER_SIXAXIS
    };

    // generate addresses
    ps3BTInfo = {
        .reserved = {0xFF,0xFF},
        .deviceAddress = { 0x00, 0x20, 0x40, 0xCE, 0x00, 0x00, 0x00 },
        .hostAddress = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    };

    for (uint8_t addr = 0; addr < 3; addr++) {
        ps3BTInfo.deviceAddress[4+addr] = (uint8_t)(get_rand_32() % 0xff);
    }

    for (uint8_t addr = 0; addr < 6; addr++) {
        ps3BTInfo.hostAddress[1+addr] = (uint8_t)(get_rand_32() % 0xff);
    }

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        switch (pinMappings[pin].action) {
            case GpioAction::MODE_GUITAR_FRET_GREEN: buttonFretGreen->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_RED: buttonFretRed->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_YELLOW: buttonFretYellow->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_BLUE: buttonFretBlue->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_ORANGE: buttonFretOrange->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_WHAMMY: buttonWhammy->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_PICKUP: buttonPickup->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_TILT: buttonTilt->pinMask |= 1 << pin; break;

            case GpioAction::MODE_DRUM_RED_DRUMPAD: buttonDrumPadRed->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_BLUE_DRUMPAD: buttonDrumPadBlue->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_YELLOW_DRUMPAD: buttonDrumPadYellow->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_GREEN_DRUMPAD: buttonDrumPadGreen->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_YELLOW_CYMBAL: buttonCymbalYellow->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_BLUE_CYMBAL: buttonCymbalBlue->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_GREEN_CYMBAL: buttonCymbalGreen->pinMask |= 1 << pin; break;

            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_UP: buttonShiftUp->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_DOWN: buttonShiftDown->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_STEERING_LEFT: buttonSteerLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_STEERING_RIGHT: buttonSteerRight->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_BUTTON_PLUS: buttonPlus->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_BUTTON_MINUS: buttonMinus->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_DIAL_UP: buttonDialUp->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_DIAL_DOWN: buttonDialDown->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_DIAL_ENTER: buttonDialEnter->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_PEDAL_GAS: buttonGas->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_PEDAL_BRAKE: buttonBrake->pinMask |= 1 << pin; break;

            default:    break;
        }
    }

    class_driver = {
    #if CFG_TUSB_DEBUG >= 2
        .name = "PS3",
    #endif
        .init = hidd_init,
        .reset = hidd_reset,
        .open = hidd_open,
        .control_xfer_cb = hidd_control_xfer_cb,
        .xfer_cb = hidd_xfer_cb,
        .sof = NULL
    };
}

// Generate PS3 report from gamepad and send to TUSB Device
bool PS3Driver::process(Gamepad * gamepad) {
    const GamepadOptions & options = gamepad->getOptions();
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;

    uint8_t * report;
    uint16_t report_size = 0;

    if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
        // reset button states to false 
        ps3Report.buttonSouth    = false;
        ps3Report.buttonEast     = false;
        ps3Report.buttonWest     = false;
        ps3Report.buttonNorth    = false;
        ps3Report.buttonL1       = false;
        ps3Report.buttonR1       = false;
        ps3Report.buttonL2       = false;
        ps3Report.buttonR2       = false;
        ps3Report.buttonSelect   = false;
        ps3Report.buttonStart    = false;
        ps3Report.buttonL3       = false;
        ps3Report.buttonR3       = false;
        ps3Report.buttonPS       = false;
        ps3Report.buttonTP       = false;

        ps3Report.dpadLeft    = gamepad->pressedLeft();
        ps3Report.dpadDown    = gamepad->pressedDown();
        ps3Report.dpadRight   = gamepad->pressedRight();
        ps3Report.dpadUp      = gamepad->pressedUp();

        ps3Report.buttonSouth    = gamepad->pressedB1();
        ps3Report.buttonEast     = gamepad->pressedB2();
        ps3Report.buttonWest     = gamepad->pressedB3();
        ps3Report.buttonNorth    = gamepad->pressedB4();
        ps3Report.buttonL1       = gamepad->pressedL1();
        ps3Report.buttonR1       = gamepad->pressedR1();
        ps3Report.buttonL2       = gamepad->pressedL2();
        ps3Report.buttonR2       = gamepad->pressedR2();
        ps3Report.buttonSelect   = gamepad->pressedS1();
        ps3Report.buttonStart    = gamepad->pressedS2();
        ps3Report.buttonL3       = gamepad->pressedL3();
        ps3Report.buttonR3       = gamepad->pressedR3();
        ps3Report.buttonPS       = gamepad->pressedA1();
        ps3Report.buttonTP       = gamepad->pressedA2();

        ps3Report.leftStickX = static_cast<uint8_t>(gamepad->state.lx >> 8);
        ps3Report.leftStickY = static_cast<uint8_t>(gamepad->state.ly >> 8);
        ps3Report.rightStickX = static_cast<uint8_t>(gamepad->state.rx >> 8);
        ps3Report.rightStickY = static_cast<uint8_t>(gamepad->state.ry >> 8);

        if (gamepad->hasAnalogTriggers)
        {
            ps3Report.buttonL2Analog = gamepad->state.lt;
            ps3Report.buttonR2Analog = gamepad->state.rt;
        } else {
            ps3Report.buttonL2Analog = gamepad->pressedL2() ? 0xFF : 0;
            ps3Report.buttonR2Analog = gamepad->pressedR2() ? 0xFF : 0;
        }

        ps3Report.buttonNorthAnalog = gamepad->pressedB4() ? 0xFF : 0;
        ps3Report.buttonEastAnalog  = gamepad->pressedB2() ? 0xFF : 0;
        ps3Report.buttonSouthAnalog = gamepad->pressedB1() ? 0xFF : 0;
        ps3Report.buttonWestAnalog  = gamepad->pressedB3() ? 0xFF : 0;
        ps3Report.buttonL1Analog    = gamepad->pressedL1() ? 0xFF : 0;
        ps3Report.buttonR1Analog    = gamepad->pressedR1() ? 0xFF : 0;
        ps3Report.dpadRightAnalog   = gamepad->state.dpad & GAMEPAD_MASK_RIGHT ? 0xFF : 0;
        ps3Report.dpadLeftAnalog    = gamepad->state.dpad & GAMEPAD_MASK_LEFT ? 0xFF : 0;
        ps3Report.dpadUpAnalog      = gamepad->state.dpad & GAMEPAD_MASK_UP ? 0xFF : 0;
        ps3Report.dpadDownAnalog    = gamepad->state.dpad & GAMEPAD_MASK_DOWN ? 0xFF : 0;

        if (gamepad->auxState.sensors.accelerometer.enabled) {
            ps3Report.accelerometerX = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
            ps3Report.accelerometerY = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
            ps3Report.accelerometerZ = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
        } else {
            ps3Report.accelerometerX = PS3_CENTER_SIXAXIS;
            ps3Report.accelerometerY = PS3_CENTER_SIXAXIS;
            ps3Report.accelerometerZ = PS3_CENTER_SIXAXIS;
        }

        if (gamepad->auxState.sensors.gyroscope.enabled) {
            ps3Report.gyroscopeZ = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
            ps3Report.reserved4 = PS3_CENTER_SIXAXIS;
        } else {
            ps3Report.gyroscopeZ = PS3_CENTER_SIXAXIS;
            ps3Report.reserved4 = PS3_CENTER_SIXAXIS;
        }

        report = (uint8_t*)&ps3Report;
        report_size = sizeof(ps3Report);
    } else if (deviceType != InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
        if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GUITAR) {
            ps3ReportAlt.guitar.padding0[0] = PS3_JOYSTICK_MID;
            ps3ReportAlt.guitar.padding0[1] = PS3_JOYSTICK_MID;
            ps3ReportAlt.guitar.pickup = PS3_JOYSTICK_MIN;
            ps3ReportAlt.guitar.whammy = PS3_JOYSTICK_MIN;
            ps3ReportAlt.guitar.tilt = false;
            ps3ReportAlt.guitar.solo = false;
            ps3ReportAlt.guitar.green = false;
            ps3ReportAlt.guitar.red = false;
            ps3ReportAlt.guitar.yellow = false;
            ps3ReportAlt.guitar.blue = false;
            ps3ReportAlt.guitar.orange = false;

            switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
            {
                case GAMEPAD_MASK_UP:                        ps3ReportAlt.guitar.dpadDirection = PS3_HAT_UP; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps3ReportAlt.guitar.dpadDirection = PS3_HAT_UPRIGHT; break;
                case GAMEPAD_MASK_RIGHT:                     ps3ReportAlt.guitar.dpadDirection = PS3_HAT_RIGHT; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps3ReportAlt.guitar.dpadDirection = PS3_HAT_DOWNRIGHT; break;
                case GAMEPAD_MASK_DOWN:                      ps3ReportAlt.guitar.dpadDirection = PS3_HAT_DOWN; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps3ReportAlt.guitar.dpadDirection = PS3_HAT_DOWNLEFT; break;
                case GAMEPAD_MASK_LEFT:                      ps3ReportAlt.guitar.dpadDirection = PS3_HAT_LEFT; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps3ReportAlt.guitar.dpadDirection = PS3_HAT_UPLEFT; break;
                default:                                     ps3ReportAlt.guitar.dpadDirection = PS3_HAT_NOTHING; break;
            }

            ps3ReportAlt.guitar.buttonSelect = gamepad->pressedS1();
            ps3ReportAlt.guitar.buttonStart  = gamepad->pressedS2();
            ps3ReportAlt.guitar.buttonPS     = gamepad->pressedA1();
            ps3ReportAlt.guitar.solo         = gamepad->pressedL2();
            
            // frets also activate their face button counterparts
            if ((values & buttonFretGreen->pinMask)  || gamepad->pressedB1()) ps3ReportAlt.guitar.green  = true;
            if ((values & buttonFretRed->pinMask)    || gamepad->pressedB2()) ps3ReportAlt.guitar.red    = true;
            if ((values & buttonFretYellow->pinMask) || gamepad->pressedB4()) ps3ReportAlt.guitar.yellow = true;
            if ((values & buttonFretBlue->pinMask)   || gamepad->pressedB3()) ps3ReportAlt.guitar.blue   = true;
            if ((values & buttonFretOrange->pinMask) || gamepad->pressedL1()) ps3ReportAlt.guitar.orange = true;
            if ((values & buttonTilt->pinMask)       || gamepad->pressedR2()) ps3ReportAlt.guitar.tilt   = true;

            ps3ReportAlt.guitar.whammy = static_cast<uint8_t>(gamepad->state.rx >> 8);
            ps3ReportAlt.guitar.pickup = static_cast<uint8_t>(gamepad->state.ry >> 8);

            if (values & buttonPickup->pinMask) ps3ReportAlt.guitar.pickup = PS3_JOYSTICK_MAX;
            if (values & buttonWhammy->pinMask) ps3ReportAlt.guitar.whammy = PS3_JOYSTICK_MAX;
        } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_DRUM) {
            ps3ReportAlt.drums.padding0[0] = PS3_JOYSTICK_MID;
            ps3ReportAlt.drums.padding0[1] = PS3_JOYSTICK_MID;
            ps3ReportAlt.drums.blue = false;
            ps3ReportAlt.drums.green = false;
            ps3ReportAlt.drums.red = false;
            ps3ReportAlt.drums.yellow = false;

            ps3ReportAlt.drums.pad = false;
            ps3ReportAlt.drums.cymbal = false;

            switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
            {
                case GAMEPAD_MASK_UP:                        ps3ReportAlt.drums.dpadDirection = PS3_HAT_UP; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps3ReportAlt.drums.dpadDirection = PS3_HAT_UPRIGHT; break;
                case GAMEPAD_MASK_RIGHT:                     ps3ReportAlt.drums.dpadDirection = PS3_HAT_RIGHT; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps3ReportAlt.drums.dpadDirection = PS3_HAT_DOWNRIGHT; break;
                case GAMEPAD_MASK_DOWN:                      ps3ReportAlt.drums.dpadDirection = PS3_HAT_DOWN; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps3ReportAlt.drums.dpadDirection = PS3_HAT_DOWNLEFT; break;
                case GAMEPAD_MASK_LEFT:                      ps3ReportAlt.drums.dpadDirection = PS3_HAT_LEFT; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps3ReportAlt.drums.dpadDirection = PS3_HAT_UPLEFT; break;
                default:                                     ps3ReportAlt.drums.dpadDirection = PS3_HAT_NOTHING; break;
            }

            ps3ReportAlt.drums.buttonSelect   = gamepad->pressedS1();
            ps3ReportAlt.drums.buttonStart    = gamepad->pressedS2();
            ps3ReportAlt.drums.buttonPS       = gamepad->pressedA1();
            ps3ReportAlt.drums.kickPedalLeft  = gamepad->pressedL1();
            ps3ReportAlt.drums.kickPedalRight = gamepad->pressedR1();
            
            // frets also activate their face button counterparts
            if ((values & buttonDrumPadGreen->pinMask)  || gamepad->pressedB1()) { ps3ReportAlt.drums.green  = true; ps3ReportAlt.drums.pad = true; }
            if ((values & buttonDrumPadRed->pinMask)    || gamepad->pressedB2()) { ps3ReportAlt.drums.red    = true; ps3ReportAlt.drums.pad = true; }
            if ((values & buttonDrumPadYellow->pinMask) || gamepad->pressedB4()) { ps3ReportAlt.drums.yellow = true; ps3ReportAlt.drums.pad = true; }
            if ((values & buttonDrumPadBlue->pinMask)   || gamepad->pressedB3()) { ps3ReportAlt.drums.blue   = true; ps3ReportAlt.drums.pad = true; }
            if ((values & buttonCymbalYellow->pinMask)  || gamepad->pressedL1()) { ps3ReportAlt.drums.yellow = true; ps3ReportAlt.drums.cymbal = true; ps3ReportAlt.drums.dpadDirection = PS3_HAT_UP; }
            if ((values & buttonCymbalBlue->pinMask)    || gamepad->pressedR2()) { ps3ReportAlt.drums.blue   = true; ps3ReportAlt.drums.cymbal = true; ps3ReportAlt.drums.dpadDirection = PS3_HAT_DOWN; }
            if ((values & buttonCymbalGreen->pinMask)   || gamepad->pressedR2()) { ps3ReportAlt.drums.green  = true; ps3ReportAlt.drums.cymbal = true; }
        } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD_ALT) {
            switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
            {
                case GAMEPAD_MASK_UP:                        ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_UP; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_UPRIGHT; break;
                case GAMEPAD_MASK_RIGHT:                     ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_RIGHT; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_DOWNRIGHT; break;
                case GAMEPAD_MASK_DOWN:                      ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_DOWN; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_DOWNLEFT; break;
                case GAMEPAD_MASK_LEFT:                      ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_LEFT; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_UPLEFT; break;
                default:                                     ps3ReportAlt.gamepad.dpadDirection = PS3_HAT_NOTHING; break;
            }

            ps3ReportAlt.gamepad.buttonSouth  = gamepad->pressedB1();
            ps3ReportAlt.gamepad.buttonEast   = gamepad->pressedB2();
            ps3ReportAlt.gamepad.buttonWest   = gamepad->pressedB3();
            ps3ReportAlt.gamepad.buttonNorth  = gamepad->pressedB4();
            ps3ReportAlt.gamepad.buttonL1     = gamepad->pressedL1();
            ps3ReportAlt.gamepad.buttonR1     = gamepad->pressedR1();
            ps3ReportAlt.gamepad.buttonL2     = gamepad->pressedL2();
            ps3ReportAlt.gamepad.buttonR2     = gamepad->pressedR2();
            ps3ReportAlt.gamepad.buttonSelect = gamepad->pressedS1();
            ps3ReportAlt.gamepad.buttonStart  = gamepad->pressedS2();
            ps3ReportAlt.gamepad.buttonL3     = gamepad->pressedL3();
            ps3ReportAlt.gamepad.buttonR3     = gamepad->pressedR3();
            ps3ReportAlt.gamepad.buttonPS     = gamepad->pressedA1();
            ps3ReportAlt.gamepad.buttonTP     = gamepad->pressedA2();

            ps3ReportAlt.gamepad.leftStickX = static_cast<uint8_t>(gamepad->state.lx >> 8);
            ps3ReportAlt.gamepad.leftStickY = static_cast<uint8_t>(gamepad->state.ly >> 8);
            ps3ReportAlt.gamepad.rightStickX = static_cast<uint8_t>(gamepad->state.rx >> 8);
            ps3ReportAlt.gamepad.rightStickY = static_cast<uint8_t>(gamepad->state.ry >> 8);

            if (gamepad->hasAnalogTriggers)
            {
                ps3ReportAlt.gamepad.buttonL2Analog = gamepad->state.lt;
                ps3ReportAlt.gamepad.buttonR2Analog = gamepad->state.rt;
            } else {
                ps3ReportAlt.gamepad.buttonL2Analog = gamepad->pressedL2()                    ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
                ps3ReportAlt.gamepad.buttonR2Analog = gamepad->pressedR2()                    ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            }

            ps3ReportAlt.gamepad.buttonNorthAnalog = ps3ReportAlt.gamepad.buttonNorth         ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.buttonEastAnalog  = ps3ReportAlt.gamepad.buttonEast          ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.buttonSouthAnalog = ps3ReportAlt.gamepad.buttonSouth         ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.buttonWestAnalog  = ps3ReportAlt.gamepad.buttonWest          ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.buttonL1Analog    = ps3ReportAlt.gamepad.buttonL1            ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.buttonR1Analog    = ps3ReportAlt.gamepad.buttonR1            ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.dpadRightAnalog   = gamepad->state.dpad & GAMEPAD_MASK_RIGHT ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.dpadLeftAnalog    = gamepad->state.dpad & GAMEPAD_MASK_LEFT  ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.dpadUpAnalog      = gamepad->state.dpad & GAMEPAD_MASK_UP    ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;
            ps3ReportAlt.gamepad.dpadDownAnalog    = gamepad->state.dpad & GAMEPAD_MASK_DOWN  ? PS3_JOYSTICK_MAX : PS3_JOYSTICK_MIN;

            if (gamepad->auxState.sensors.accelerometer.enabled) {
                ps3ReportAlt.gamepad.accelerometerX = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
                ps3ReportAlt.gamepad.accelerometerY = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
                ps3ReportAlt.gamepad.accelerometerZ = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
            } else {
                ps3ReportAlt.gamepad.accelerometerX = PS3_CENTER_SIXAXIS;
                ps3ReportAlt.gamepad.accelerometerY = PS3_CENTER_SIXAXIS;
                ps3ReportAlt.gamepad.accelerometerZ = PS3_CENTER_SIXAXIS;
            }

            if (gamepad->auxState.sensors.gyroscope.enabled) {
                ps3ReportAlt.gamepad.gyroscopeZ = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
            } else {
                ps3ReportAlt.gamepad.gyroscopeZ = PS3_CENTER_SIXAXIS;
            }
        } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_WHEEL) {
            ps3ReportAlt.wheel.steeringWheel = PS3_WHEEL_MID;
            ps3ReportAlt.wheel.gasPedal      = PS3_JOYSTICK_MID;
            ps3ReportAlt.wheel.brakePedal    = PS3_JOYSTICK_MID;

            switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
            {
                case GAMEPAD_MASK_UP:                        ps3ReportAlt.wheel.dpadDirection = PS3_HAT_UP; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps3ReportAlt.wheel.dpadDirection = PS3_HAT_UPRIGHT; break;
                case GAMEPAD_MASK_RIGHT:                     ps3ReportAlt.wheel.dpadDirection = PS3_HAT_RIGHT; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps3ReportAlt.wheel.dpadDirection = PS3_HAT_DOWNRIGHT; break;
                case GAMEPAD_MASK_DOWN:                      ps3ReportAlt.wheel.dpadDirection = PS3_HAT_DOWN; break;
                case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps3ReportAlt.wheel.dpadDirection = PS3_HAT_DOWNLEFT; break;
                case GAMEPAD_MASK_LEFT:                      ps3ReportAlt.wheel.dpadDirection = PS3_HAT_LEFT; break;
                case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps3ReportAlt.wheel.dpadDirection = PS3_HAT_UPLEFT; break;
                default:                                     ps3ReportAlt.wheel.dpadDirection = PS3_HAT_NOTHING; break;
            }

            ps3ReportAlt.wheel.buttonSouth  = gamepad->pressedB1();
            ps3ReportAlt.wheel.buttonEast   = gamepad->pressedB2();
            ps3ReportAlt.wheel.buttonWest   = gamepad->pressedB3();
            ps3ReportAlt.wheel.buttonNorth  = gamepad->pressedB4();
            ps3ReportAlt.wheel.buttonL1     = gamepad->pressedL1();
            ps3ReportAlt.wheel.buttonR1     = gamepad->pressedR1();
            ps3ReportAlt.wheel.buttonL2     = gamepad->pressedL2();
            ps3ReportAlt.wheel.buttonR2     = gamepad->pressedR2();
            ps3ReportAlt.wheel.buttonSelect = gamepad->pressedS1();
            ps3ReportAlt.wheel.buttonStart  = gamepad->pressedS2();
            ps3ReportAlt.wheel.buttonL3     = gamepad->pressedL3();
            ps3ReportAlt.wheel.buttonR3     = gamepad->pressedR3();
            ps3ReportAlt.wheel.buttonPS     = gamepad->pressedA1();

            if (values & buttonSteerLeft->pinMask)  { ps3ReportAlt.wheel.steeringWheel   = PS3_WHEEL_MIN; }
            if (values & buttonSteerRight->pinMask) { ps3ReportAlt.wheel.steeringWheel   = PS3_WHEEL_MAX; }
            if (values & buttonGas->pinMask)        { ps3ReportAlt.wheel.gasPedal        = PS3_JOYSTICK_MIN; }
            if (values & buttonBrake->pinMask)      { ps3ReportAlt.wheel.brakePedal      = PS3_JOYSTICK_MIN; }
        } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_HOTAS) {

        }

        report = (uint8_t*)&ps3ReportAlt;
        report_size = sizeof(ps3ReportAlt);
    }

    // Wake up TinyUSB device
    if (tud_suspended())
        tud_remote_wakeup();

    bool reportSent = false;

    if (memcmp(last_report, report, report_size) != 0)
    {
        // HID ready + report sent, copy previous report
        if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
            memcpy(last_report, report, report_size);
            reportSent = true;
        }
    }

    uint16_t featureSize = sizeof(PS3Features);
    if (memcmp(lastFeatures, &ps3Features, featureSize) != 0) {
        memcpy(lastFeatures, &ps3Features, featureSize);
        Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

        if (gamepad->auxState.haptics.leftActuator.enabled) {
            gamepad->auxState.haptics.leftActuator.active = (ps3Features.leftMotorPower > 0);
            gamepad->auxState.haptics.leftActuator.intensity = ps3Features.leftMotorPower;
        }

        if (gamepad->auxState.haptics.rightActuator.enabled) {
            gamepad->auxState.haptics.rightActuator.active = (ps3Features.rightMotorPower > 0);
            gamepad->auxState.haptics.rightActuator.intensity = ps3Features.rightMotorPower;
        }

        gamepad->auxState.playerID.active = true;
        gamepad->auxState.playerID.ledValue = ps3Features.playerLED;
        gamepad->auxState.playerID.value = (ps3Features.playerLED & 0x0F);
    }

    return reportSent;
}

// unknown
static constexpr uint8_t output_ps3_0x01[] = {
    0x01, 0x04, 0x00, 0x0b, 0x0c, 0x01, 0x02, 0x18, 
    0x18, 0x18, 0x18, 0x09, 0x0a, 0x10, 0x11, 0x12,
    0x13, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x02,
    0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x04, 0x04,
    0x04, 0x04, 0x00, 0x00, 0x04, 0x00, 0x01, 0x02,
    0x07, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// same idea as above, but for non DS3 controllers
static constexpr uint8_t output_ps3_alt_0x01[] = {
    0x21, 0x26, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

// calibration data
static constexpr uint8_t output_ps3_0xef[] = {
    0xef, 0x04, 0x00, 0x0b, 0x03, 0x01, 0xa0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff,
    0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
};

// unknown
static constexpr uint8_t output_ps3_0xf5[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // host address - must match 0xf2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// unknown
static constexpr uint8_t output_ps3_0xf7[] = {
    0x02, 0x01, 0xf8, 0x02, 0xe2, 0x01, 0x05, 0xff,
    0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// unknown
static constexpr uint8_t output_ps3_0xf8[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// tud_hid_get_report_cb
uint16_t PS3Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    //printf("[PS3Driver::get_report] (%02x, %02x, %d)\n", report_id, report_type, reqlen);

    if ( report_type == HID_REPORT_TYPE_INPUT ) {
        memcpy(buffer, &ps3Report, sizeof(PS3Report));
        return sizeof(PS3Report);
    } else if ( report_type == HID_REPORT_TYPE_FEATURE ) {
        uint16_t responseLen = 0;
        uint8_t ctr = 0;
        switch(report_id) {
            case PS3ReportTypes::PS3_FEATURE_01:
                responseLen = reqlen;
                if (deviceType != InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
                    memcpy(buffer, output_ps3_0x01, responseLen);
                } else {
                    memcpy(buffer, output_ps3_alt_0x01, responseLen);
                }
                return responseLen;
            case PS3ReportTypes::PS3_FEATURE_EF:
                responseLen = reqlen;
                memcpy(buffer, output_ps3_0xef, responseLen);
                buffer[6] = efByte;
                return responseLen;
            case PS3ReportTypes::PS3_GET_PAIRING_INFO:
                responseLen = reqlen;
                memcpy(buffer, &ps3BTInfo, responseLen);
                return responseLen;
            case PS3ReportTypes::PS3_FEATURE_F5:
                responseLen = reqlen;
                memcpy(buffer, output_ps3_0xf5, responseLen);
                for (ctr = 0; ctr < 6; ctr++) {
                    buffer[1+ctr] = ps3BTInfo.hostAddress[ctr];
                }
                return responseLen;
            case PS3ReportTypes::PS3_FEATURE_F7:
                responseLen = reqlen;
                memcpy(buffer, output_ps3_0xf7, responseLen);
                return responseLen;
            case PS3ReportTypes::PS3_FEATURE_F8:
                responseLen = reqlen;
                memcpy(buffer, output_ps3_0xf8, responseLen);
                buffer[6] = efByte;
                return responseLen;
        }
    }
    return -1;
}

void PS3Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    //printf("[PS3Driver::set_report] (%02x, %02x, %d)\n", report_id, report_type, bufsize);
    //for(uint16_t i = 0; i < bufsize; i++) {
    //    printf("%02x ", buffer[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("\n");

    if ( report_type == HID_REPORT_TYPE_FEATURE ) {
        switch(report_id) {
            case PS3ReportTypes::PS3_FEATURE_EF:
                efByte = buffer[6];
                break;
        }
    } else if (report_type == HID_REPORT_TYPE_OUTPUT ) {
        // DS3 command
        uint8_t const *buf = buffer;
        if (report_id == 0 && bufsize > 0) {
            report_id = buffer[0];
            bufsize = bufsize - 1;
            buf = &buffer[1];
        }
        switch(report_id) {
            case PS3ReportTypes::PS3_FEATURE_01:
                memcpy(&ps3Features, buf, bufsize);
                break;
        }
    }
}

// Only XboxOG and Xbox One use vendor control xfer cb
bool PS3Driver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * PS3Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)ps3_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PS3Driver::get_descriptor_device_cb() {
    return deviceDescriptor;
}

const uint8_t * PS3Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    if (deviceType != InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
        return ps3_alt_report_descriptor;
    } else {
        return ps3_report_descriptor;
    }
}

const uint8_t * PS3Driver::get_descriptor_configuration_cb(uint8_t index) {
    if (deviceType != InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
        return ps3_alt_configuration_descriptor;
    } else {
        return ps3_configuration_descriptor;
    }
}

const uint8_t * PS3Driver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t PS3Driver::GetJoystickMidValue() {
    return GAMEPAD_JOYSTICK_MID;
}
