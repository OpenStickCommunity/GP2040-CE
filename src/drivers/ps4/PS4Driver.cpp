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
//--------------------------------------------------
// Use for normal controlller support.
//--------------------------------------------------
//    0x21, 0x27, 0x04, 0xcf, 0x00, 0x2c, 0x56,
//    0x08, 0x00, 0x3d, 0x00, 0xe8, 0x03, 0x04, 0x00,
//    0xff, 0x7f, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00,
//    0x0D, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//--------------------------------------------------
// Use for wheel support. What's different?
//--------------------------------------------------
    0x21, 0x27, 0x03, 0x11, 0x06, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0D, 0x0D, 0x00, 0x00, 0x00, 0x00,
    0x0D, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
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

    //deviceType = InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD;
    deviceType = options.inputDeviceType;

    if (!isDeviceEmulated) {
        if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_WHEEL) {
            // wheel
            deviceDescriptor[8] = LSB(PS4_WHEEL_VENDOR_ID);
            deviceDescriptor[9] = MSB(PS4_WHEEL_VENDOR_ID);
            deviceDescriptor[10] = LSB(PS4_WHEEL_PRODUCT_ID);
            deviceDescriptor[11] = MSB(PS4_WHEEL_PRODUCT_ID);
        } else {
            // assume gamepad if not special cased
            deviceDescriptor[8] = LSB(PS4_VENDOR_ID);
            deviceDescriptor[9] = MSB(PS4_VENDOR_ID);
            deviceDescriptor[10] = LSB(PS4_PRODUCT_ID);
            deviceDescriptor[11] = MSB(PS4_PRODUCT_ID);
        }
    } else {
        deviceDescriptor[8] = LSB(DS4_VENDOR_ID);
        deviceDescriptor[9] = MSB(DS4_VENDOR_ID);
        deviceDescriptor[10] = LSB(DS4_PRODUCT_ID);
        deviceDescriptor[11] = MSB(DS4_PRODUCT_ID);
    }

    // check InputModeDeviceType mode for PS4 driver and set PS4ControllerType where applicable
    // PS5 mode currently forces PS4ControllerType 7 (PS4_ARCADESTICK) for PS5 compatibility
    switch (deviceType) {
        case InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_WHEEL:
            enableController = false;
            enableMotion = false;
            enableLED = false;
            enableRumble = false;
            enableAnalog = true;
            enableUnknown0 = false;
            enableTouchpad = false;
            enableUnknown1 = false;
            shifterPosition = 0;

            controllerType = PS4ControllerType::PS4_WHEEL;

            buttonShiftUp = new GamepadButtonMapping(0);
            buttonShiftDown = new GamepadButtonMapping(0);
            buttonShift1 = new GamepadButtonMapping(0);
            buttonShift2 = new GamepadButtonMapping(0);
            buttonShift3 = new GamepadButtonMapping(0);
            buttonShift4 = new GamepadButtonMapping(0);
            buttonShift5 = new GamepadButtonMapping(0);
            buttonShift6 = new GamepadButtonMapping(0);
            buttonShiftR = new GamepadButtonMapping(0);
            buttonShiftN = new GamepadButtonMapping(0);
            buttonGas = new GamepadButtonMapping(0);
            buttonBrake = new GamepadButtonMapping(0);
            buttonClutch = new GamepadButtonMapping(0);
            buttonSteerLeft = new GamepadButtonMapping(0);
            buttonSteerRight = new GamepadButtonMapping(0);
            buttonPlus = new GamepadButtonMapping(0);
            buttonMinus = new GamepadButtonMapping(0);
            buttonDialDown = new GamepadButtonMapping(0);
            buttonDialUp = new GamepadButtonMapping(0);
            buttonDialEnter = new GamepadButtonMapping(0);
            break;
        case InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_HOTAS:
            enableController = false;
            enableMotion = false;
            enableLED = false;
            enableRumble = false;
            enableAnalog = true;
            enableUnknown0 = false;
            enableTouchpad = false;
            enableUnknown1 = false;

            controllerType = PS4ControllerType::PS4_HOTAS;

            buttonRudderLeft = new GamepadButtonMapping(0);
            buttonRudderRight = new GamepadButtonMapping(0);
            buttonThrottleForward = new GamepadButtonMapping(0);
            buttonThrottleReverse = new GamepadButtonMapping(0);
            buttonRockerLeft = new GamepadButtonMapping(0);
            buttonRockerRight = new GamepadButtonMapping(0);
            buttonPedalLeft = new GamepadButtonMapping(0);
            buttonPedalRight = new GamepadButtonMapping(0);
            buttonPedalRudderLeft = new GamepadButtonMapping(0);
            buttonPedalRudderRight = new GamepadButtonMapping(0);
            break;
        case InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GUITAR:
            enableController = true;
            enableMotion = false;
            enableLED = false;
            enableRumble = false;
            enableAnalog = true;
            enableUnknown0 = false;
            enableTouchpad = false;
            enableUnknown1 = true;

            controllerType = PS4ControllerType::PS4_GUITAR;

            buttonFretGreen = new GamepadButtonMapping(0);
            buttonFretRed = new GamepadButtonMapping(0);
            buttonFretYellow = new GamepadButtonMapping(0);
            buttonFretBlue = new GamepadButtonMapping(0);
            buttonFretOrange = new GamepadButtonMapping(0);
            buttonFretSoloGreen = new GamepadButtonMapping(0);
            buttonFretSoloRed = new GamepadButtonMapping(0);
            buttonFretSoloYellow = new GamepadButtonMapping(0);
            buttonFretSoloBlue = new GamepadButtonMapping(0);
            buttonFretSoloOrange = new GamepadButtonMapping(0);
            buttonWhammy = new GamepadButtonMapping(0);
            buttonPickup = new GamepadButtonMapping(0);
            buttonTilt = new GamepadButtonMapping(0);
            break;
        case InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_DRUM:
            enableController = true;
            enableMotion = false;
            enableLED = false;
            enableRumble = false;
            enableAnalog = true;
            enableUnknown0 = false;
            enableTouchpad = false;
            enableUnknown1 = true;

            controllerType = PS4ControllerType::PS4_DRUMS;

            buttonDrumPadRed = new GamepadButtonMapping(0);
            buttonDrumPadBlue = new GamepadButtonMapping(0);
            buttonDrumPadYellow = new GamepadButtonMapping(0);
            buttonDrumPadGreen = new GamepadButtonMapping(0);
            buttonCymbalYellow = new GamepadButtonMapping(0);
            buttonCymbalBlue = new GamepadButtonMapping(0);
            buttonCymbalGreen = new GamepadButtonMapping(0);
            buttonKickPedalLeft = new GamepadButtonMapping(0);
            buttonKickPedalRight = new GamepadButtonMapping(0);
            break;
        case InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD:
        default:
            enableController = true;
            enableMotion = true;
            enableLED = true;
            enableRumble = true;
            enableAnalog = false;
            enableUnknown0 = true;
            enableTouchpad = true;
            enableUnknown1 = true;

            // if PS4, PS4ControllerType::PS4_CONTROLLER
            // if PS5, PS4ControllerType::PS4_ARCADESTICK
            if (options.inputMode == INPUT_MODE_PS4) {
                controllerType = PS4ControllerType::PS4_CONTROLLER;
            } else {
                controllerType = PS4ControllerType::PS4_ARCADESTICK;
            }
            break;
    }

    // init feature data
    if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
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
    }

    // preseed touchpad sensors with center position values
    gamepad->auxState.sensors.touchpad[0].x = PS4_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[0].y = PS4_TP_Y_MAX/2;
    gamepad->auxState.sensors.touchpad[1].x = PS4_TP_X_MAX/2;
    gamepad->auxState.sensors.touchpad[1].y = PS4_TP_Y_MAX/2;

    touchCounter = 0;

    ps4Report = {
        .reportID = 0x01,
        .leftStickX = PS4_JOYSTICK_MID,
        .leftStickY = PS4_JOYSTICK_MID,
        .rightStickX = PS4_JOYSTICK_MID,
        .rightStickY = PS4_JOYSTICK_MID,
        .dpad = 0x08,
        .buttonWest = 0, .buttonSouth = 0, .buttonEast = 0, .buttonNorth = 0,
        .buttonL1 = 0, .buttonR1 = 0, .buttonL2 = 0, .buttonR2 = 0,
        .buttonSelect = 0, .buttonStart = 0, .buttonL3 = 0, .buttonR3 = 0, .buttonHome = 0, .buttonTouchpad = 0,
        .reportCounter = 0, .leftTrigger = 0, .rightTrigger = 0,
        .miscData = {}
    };

    // see output_0x03
    controllerConfig = {
        .hidUsage = 0x2721,
        .mystery0 = 0x04,
//        .featureValue = 0xEF,
        .features = {
            .enableController = enableController,
            .enableMotion = enableMotion,
            .enableLED = enableLED,
            .enableRumble = enableRumble,
            .enableAnalog = enableAnalog,
            .enableUnknown0 = enableUnknown0,
            .enableTouchpad = enableTouchpad,
            .enableUnknown1 = enableUnknown1,
        },
        .controllerType = (uint8_t)controllerType,
        .touchpadParam = {0x2c, 0x56},
        .imuConfig = {
            .gyroRange = 0x0008,
            .gyroResPerDegDenom = 0x003D,
            .gyroResPerDegNumer = 0x03E8,
            .accelRange = 0x0004,
            .accelResPerG = 0x7FFF,
        },
        .magicID = 0x0d0d,
        .mystery1 = {},
        .wheelParam = {0x0D, 0x84, 0x03},
        .mystery2 = {0x00}
    };

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        switch (pinMappings[pin].action) {
            case GpioAction::MODE_GUITAR_FRET_GREEN: buttonFretGreen->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_RED: buttonFretRed->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_YELLOW: buttonFretYellow->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_BLUE: buttonFretBlue->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_ORANGE: buttonFretOrange->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_SOLO_GREEN: buttonFretSoloGreen->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_SOLO_RED: buttonFretSoloRed->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_SOLO_YELLOW: buttonFretSoloYellow->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_SOLO_BLUE: buttonFretSoloBlue->pinMask |= 1 << pin; break;
            case GpioAction::MODE_GUITAR_FRET_SOLO_ORANGE: buttonFretSoloOrange->pinMask |= 1 << pin; break;
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
            case GpioAction::MODE_DRUM_KICK_PEDAL_LEFT: buttonKickPedalLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_DRUM_KICK_PEDAL_RIGHT: buttonKickPedalRight->pinMask |= 1 << pin; break;

            case GpioAction::MODE_HOTAS_RUDDER_LEFT: buttonRudderLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_RUDDER_RIGHT: buttonRudderRight->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_THROTTLE_FORWARD: buttonThrottleForward->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_THROTTLE_REVERSE: buttonThrottleReverse->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_ROCKER_LEFT: buttonRockerLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_ROCKER_RIGHT: buttonRockerRight->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_PEDAL_LEFT: buttonPedalLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_PEDAL_RIGHT: buttonPedalRight->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_PEDAL_RUDDER_LEFT: buttonPedalRudderLeft->pinMask |= 1 << pin; break;
            case GpioAction::MODE_HOTAS_PEDAL_RUDDER_RIGHT: buttonPedalRudderRight->pinMask |= 1 << pin; break;

            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_1: buttonShift1->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_2: buttonShift2->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_3: buttonShift3->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_4: buttonShift4->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_5: buttonShift5->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_6: buttonShift6->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_R: buttonShiftR->pinMask |= 1 << pin; break;
            case GpioAction::MODE_WHEEL_SHIFTER_GEAR_N: buttonShiftN->pinMask |= 1 << pin; break;
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
            case GpioAction::MODE_WHEEL_PEDAL_CLUTCH: buttonClutch->pinMask |= 1 << pin; break;
            default:    break;
        }
    }

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
    
    if ( controllerType == PS4ControllerType::PS4_ARCADESTICK ) {
        // Setup PS5 Auth System
        ps4AuthDriver = new PS4Auth(gamepadOptions.ps5AuthType);
    } else {
        ps4AuthDriver = new PS4Auth(gamepadOptions.ps4AuthType);
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
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
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

    // reset button states to false 
    ps4Report.buttonSouth    = false;
    ps4Report.buttonEast     = false;
    ps4Report.buttonWest     = false;
    ps4Report.buttonNorth    = false;
    ps4Report.buttonL1       = false;
    ps4Report.buttonR1       = false;
    ps4Report.buttonL2       = false;
    ps4Report.buttonR2       = false;
    ps4Report.buttonSelect   = false;
    ps4Report.buttonStart    = false;
    ps4Report.buttonL3       = false;
    ps4Report.buttonR3       = false;
    ps4Report.buttonHome     = false;
    ps4Report.buttonTouchpad = false;

    if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GUITAR) {
        ps4Report.guitar.pickup = PS4_JOYSTICK_MIN;
        ps4Report.guitar.whammy = PS4_JOYSTICK_MIN;
        ps4Report.guitar.tilt = PS4_JOYSTICK_MIN;
        ps4Report.guitar.frets.green = false;
        ps4Report.guitar.frets.red = false;
        ps4Report.guitar.frets.yellow = false;
        ps4Report.guitar.frets.blue = false;
        ps4Report.guitar.frets.orange = false;
        ps4Report.guitar.soloFrets.green  = false;
        ps4Report.guitar.soloFrets.red = false;
        ps4Report.guitar.soloFrets.yellow = false;
        ps4Report.guitar.soloFrets.blue = false;
        ps4Report.guitar.soloFrets.orange = false;
        
        // frets also activate their face button counterparts
        if (values & buttonFretGreen->pinMask)      { ps4Report.guitar.frets.green      = true; ps4Report.buttonSouth |= true; }
        if (values & buttonFretRed->pinMask)        { ps4Report.guitar.frets.red        = true; ps4Report.buttonEast  |= true; }
        if (values & buttonFretYellow->pinMask)     { ps4Report.guitar.frets.yellow     = true; ps4Report.buttonNorth |= true; }
        if (values & buttonFretBlue->pinMask)       { ps4Report.guitar.frets.blue       = true; ps4Report.buttonWest  |= true; }
        if (values & buttonFretOrange->pinMask)     { ps4Report.guitar.frets.orange     = true; ps4Report.buttonL1    |= true; }
        
        if (values & buttonFretSoloGreen->pinMask)  { ps4Report.guitar.soloFrets.green  = true; ps4Report.buttonSouth |= true; ps4Report.buttonL3 |= true; }
        if (values & buttonFretSoloRed->pinMask)    { ps4Report.guitar.soloFrets.red    = true; ps4Report.buttonEast  |= true; ps4Report.buttonL3 |= true; }
        if (values & buttonFretSoloYellow->pinMask) { ps4Report.guitar.soloFrets.yellow = true; ps4Report.buttonNorth |= true; ps4Report.buttonL3 |= true; }
        if (values & buttonFretSoloBlue->pinMask)   { ps4Report.guitar.soloFrets.blue   = true; ps4Report.buttonWest  |= true; ps4Report.buttonL3 |= true; }
        if (values & buttonFretSoloOrange->pinMask) { ps4Report.guitar.soloFrets.orange = true; ps4Report.buttonL1    |= true; ps4Report.buttonL3 |= true; }
        
        if (values & buttonPickup->pinMask) ps4Report.guitar.pickup = PS4_JOYSTICK_MID;
        if (values & buttonWhammy->pinMask) ps4Report.guitar.whammy = PS4_JOYSTICK_MAX;
        if (values & buttonTilt->pinMask) ps4Report.guitar.tilt = PS4_JOYSTICK_MAX;
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_DRUM) {
        ps4Report.drums.velocityDrumRed = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityDrumBlue = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityDrumYellow = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityDrumGreen = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityCymbalYellow = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityCymbalBlue = PS4_JOYSTICK_MIN;
        ps4Report.drums.velocityCymbalGreen = PS4_JOYSTICK_MIN;

        if (values & buttonDrumPadRed->pinMask)    { ps4Report.drums.velocityDrumRed      = PS4_JOYSTICK_MAX; ps4Report.buttonEast  |= true; }
        if (values & buttonDrumPadBlue->pinMask)   { ps4Report.drums.velocityDrumBlue     = PS4_JOYSTICK_MAX; ps4Report.buttonWest  |= true; }
        if (values & buttonDrumPadYellow->pinMask) { ps4Report.drums.velocityDrumYellow   = PS4_JOYSTICK_MAX; ps4Report.buttonNorth |= true; }
        if (values & buttonDrumPadGreen->pinMask)  { ps4Report.drums.velocityDrumGreen    = PS4_JOYSTICK_MAX; ps4Report.buttonSouth |= true; }
        if (values & buttonCymbalYellow->pinMask)  { ps4Report.drums.velocityCymbalYellow = PS4_JOYSTICK_MAX; ps4Report.buttonNorth |= true; }
        if (values & buttonCymbalBlue->pinMask)    { ps4Report.drums.velocityCymbalBlue   = PS4_JOYSTICK_MAX; ps4Report.buttonWest  |= true; }
        if (values & buttonCymbalGreen->pinMask)   { ps4Report.drums.velocityCymbalGreen  = PS4_JOYSTICK_MAX; ps4Report.buttonSouth |= true; }

        if (values & buttonKickPedalLeft->pinMask)  { ps4Report.buttonL1 |= true; }
        if (values & buttonKickPedalRight->pinMask) { ps4Report.buttonR1 |= true; }
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_WHEEL) {
        ps4Report.wheel.steeringWheel = PS4_NAV_JOYSTICK_MID;
        ps4Report.wheel.gasPedal = PS4_NAV_JOYSTICK_MAX;
        ps4Report.wheel.brakePedal = PS4_NAV_JOYSTICK_MAX;
        ps4Report.wheel.clutchPedal = PS4_NAV_JOYSTICK_MAX;
        ps4Report.wheel.unknownVal = 0xFFFF;
        ps4Report.wheel.buttonPlus = 0;
        ps4Report.wheel.buttonMinus = 0;
        ps4Report.wheel.buttonDialDown = 0;
        ps4Report.wheel.buttonDialUp = 0;
        ps4Report.wheel.buttonDialEnter = 0;

        if ((values & buttonShiftUp->pinMask) && idleShifter) {
            if (shifterPosition < sizeof(shifterValues)-1) {
                shifterPosition++;
                idleShifter = false;
            }
        }
        if ((values & buttonShiftDown->pinMask) && idleShifter) {
            if (shifterPosition > 0) {
                shifterPosition--;
                idleShifter = false;
            }
        }
        if (!(values & buttonShiftUp->pinMask) && !(values & buttonShiftDown->pinMask) && !idleShifter) {
            idleShifter = true;
        }
        if (values & buttonShiftN->pinMask) shifterPosition = 0;
        if (values & buttonShift1->pinMask) shifterPosition = 1;
        if (values & buttonShift2->pinMask) shifterPosition = 2;
        if (values & buttonShift3->pinMask) shifterPosition = 3;
        if (values & buttonShift4->pinMask) shifterPosition = 4;
        if (values & buttonShift5->pinMask) shifterPosition = 5;
        if (values & buttonShift6->pinMask) shifterPosition = 6;
        if (values & buttonShiftR->pinMask) shifterPosition = 7;

        ps4Report.wheel.shifterValue = shifterValues[shifterPosition];

        if (values & buttonSteerLeft->pinMask) ps4Report.wheel.steeringWheel = PS4_NAV_JOYSTICK_MIN;
        if (values & buttonSteerRight->pinMask) ps4Report.wheel.steeringWheel = PS4_NAV_JOYSTICK_MAX;
        if (values & buttonBrake->pinMask) ps4Report.wheel.brakePedal = PS4_NAV_JOYSTICK_MIN;
        if (values & buttonClutch->pinMask) ps4Report.wheel.clutchPedal = PS4_NAV_JOYSTICK_MIN;
        if (values & buttonGas->pinMask) ps4Report.wheel.gasPedal = PS4_NAV_JOYSTICK_MIN;

        if (values & buttonPlus->pinMask) ps4Report.wheel.buttonPlus = 1;
        if (values & buttonMinus->pinMask) ps4Report.wheel.buttonMinus = 1;
        if (values & buttonDialDown->pinMask) ps4Report.wheel.buttonDialDown = 1;
        if (values & buttonDialUp->pinMask) ps4Report.wheel.buttonDialUp = 1;
        if (values & buttonDialEnter->pinMask) ps4Report.wheel.buttonDialEnter = 1;
    } else if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_HOTAS) {
        ps4Report.hotas.joystickX = PS4_NAV_JOYSTICK_MID;
        ps4Report.hotas.joystickY = PS4_NAV_JOYSTICK_MID;

        ps4Report.hotas.twistRudder = PS4_JOYSTICK_MID;
        ps4Report.hotas.throttle = PS4_JOYSTICK_MID;
        ps4Report.hotas.rockerSwitch = PS4_JOYSTICK_MID;

        ps4Report.hotas.pedalRudder = PS4_JOYSTICK_MID;
        ps4Report.hotas.pedalLeft = PS4_JOYSTICK_MIN;
        ps4Report.hotas.pedalRight = PS4_JOYSTICK_MIN;

        if (values & buttonRudderLeft->pinMask) ps4Report.hotas.twistRudder = PS4_JOYSTICK_MIN;
        if (values & buttonRudderRight->pinMask) ps4Report.hotas.twistRudder = PS4_JOYSTICK_MAX;
        if (values & buttonThrottleForward->pinMask) ps4Report.hotas.throttle = PS4_JOYSTICK_MIN;
        if (values & buttonThrottleReverse->pinMask) ps4Report.hotas.throttle = PS4_JOYSTICK_MAX;
        if (values & buttonRockerLeft->pinMask) ps4Report.hotas.rockerSwitch = PS4_JOYSTICK_MIN;
        if (values & buttonRockerRight->pinMask) ps4Report.hotas.rockerSwitch = PS4_JOYSTICK_MAX;
        if (values & buttonPedalLeft->pinMask) ps4Report.hotas.pedalLeft = PS4_JOYSTICK_MAX;
        if (values & buttonPedalRight->pinMask) ps4Report.hotas.pedalRight = PS4_JOYSTICK_MAX;
        if (values & buttonPedalRudderLeft->pinMask) ps4Report.hotas.pedalRudder = PS4_JOYSTICK_MAX;
        if (values & buttonPedalRudderRight->pinMask) ps4Report.hotas.pedalRudder = PS4_JOYSTICK_MIN;

        ps4Report.hotas.joystickX = PS4_NAV_JOYSTICK_MID;
        ps4Report.hotas.joystickY = PS4_NAV_JOYSTICK_MID;
    }

    ps4Report.buttonSouth    |= gamepad->pressedB1();
    ps4Report.buttonEast     |= gamepad->pressedB2();
    ps4Report.buttonWest     |= gamepad->pressedB3();
    ps4Report.buttonNorth    |= gamepad->pressedB4();
    ps4Report.buttonL1       |= gamepad->pressedL1();
    ps4Report.buttonR1       |= gamepad->pressedR1();
    ps4Report.buttonL2       |= gamepad->pressedL2();
    ps4Report.buttonR2       |= gamepad->pressedR2();
    ps4Report.buttonSelect   |= options.switchTpShareForDs4 ? anyA2A3A4 : gamepad->pressedS1();
    ps4Report.buttonStart    |= gamepad->pressedS2();
    ps4Report.buttonL3       |= gamepad->pressedL3();
    ps4Report.buttonR3       |= gamepad->pressedR3();
    ps4Report.buttonHome     |= gamepad->pressedA1();
    ps4Report.buttonTouchpad |= options.switchTpShareForDs4 ? gamepad->pressedS1() : anyA2A3A4;

    ps4Report.leftStickX = static_cast<uint8_t>(gamepad->state.lx >> 8);
    ps4Report.leftStickY = static_cast<uint8_t>(gamepad->state.ly >> 8);
    ps4Report.rightStickX = static_cast<uint8_t>(gamepad->state.rx >> 8);
    ps4Report.rightStickY = static_cast<uint8_t>(gamepad->state.ry >> 8);

    if (gamepad->hasAnalogTriggers)
    {
        ps4Report.leftTrigger = gamepad->state.lt;
        ps4Report.rightTrigger = gamepad->state.rt;
    } else {
        ps4Report.leftTrigger = gamepad->pressedL2() ? 0xFF : 0;
        ps4Report.rightTrigger = gamepad->pressedR2() ? 0xFF : 0;
    }

    // if the touchpad is pressed (note A2 vs. S1 choice above), emulate one finger of the touchpad
    if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
        touchpadData.p1.unpressed = ps4Report.buttonTouchpad ? 0 : 1;
        ps4Report.gamepad.touchpadActive = ps4Report.buttonTouchpad ? 0x01 : 0x00;
        if (ps4Report.buttonTouchpad) {
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
                ps4Report.gamepad.touchpadActive = gamepad->auxState.sensors.touchpad[0].active;
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
        ps4Report.gamepad.touchpadData = touchpadData;
    }

    if (gamepad->auxState.sensors.accelerometer.enabled) {
        ps4Report.gamepad.sensorData.accelerometer.x = ((gamepad->auxState.sensors.accelerometer.x & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.x & 0xFF00) >> 8);
        ps4Report.gamepad.sensorData.accelerometer.y = ((gamepad->auxState.sensors.accelerometer.y & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.y & 0xFF00) >> 8);
        ps4Report.gamepad.sensorData.accelerometer.z = ((gamepad->auxState.sensors.accelerometer.z & 0xFF) << 8) | ((gamepad->auxState.sensors.accelerometer.z & 0xFF00) >> 8);
    }

    if (gamepad->auxState.sensors.gyroscope.enabled) {
        ps4Report.gamepad.sensorData.gyroscope.x = ((gamepad->auxState.sensors.gyroscope.x & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.x & 0xFF00) >> 8);
        ps4Report.gamepad.sensorData.gyroscope.y = ((gamepad->auxState.sensors.gyroscope.y & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.y & 0xFF00) >> 8);
        ps4Report.gamepad.sensorData.gyroscope.z = ((gamepad->auxState.sensors.gyroscope.z & 0xFF) << 8) | ((gamepad->auxState.sensors.gyroscope.z & 0xFF00) >> 8);
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
            ps4Report.reportCounter = last_report_counter;		// report counter is 6 bits
            if (deviceType == InputModeDeviceType::INPUT_MODE_DEVICE_TYPE_GAMEPAD) {
                ps4Report.gamepad.axisTiming = now;		 		// axis counter is 16 bits
            }
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
            if (reqlen < sizeof(controllerConfig)) {
                return -1;
            }
            controllerConfig.controllerType = (uint8_t)controllerType;
            responseLen = MAX(reqlen, sizeof(controllerConfig));
            memcpy(buffer, &controllerConfig, responseLen);
            //buffer[4] = (uint8_t)controllerType; // Change controller type in definition
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
