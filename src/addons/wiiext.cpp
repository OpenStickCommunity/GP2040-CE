#include "addons/wiiext.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"
#include "config.pb.h"

bool WiiExtensionInput::available() {
    const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
    const WiiOptions& options = Storage::getInstance().getAddonOptions().wiiOptions;

    return (!displayOptions.enabled && (options.enabled && isValidPin(options.i2cSDAPin) && isValidPin(options.i2cSCLPin)));
}

void WiiExtensionInput::setup() {
    const WiiOptions& options = Storage::getInstance().getAddonOptions().wiiOptions;
    nextTimer = getMillis();

#if WII_EXTENSION_DEBUG==true
    stdio_init_all();
#endif

    uIntervalMS = 0;

    currentConfig = NULL;
    
    wii = new WiiExtension(
        options.i2cSDAPin,
        options.i2cSCLPin,
        options.i2cBlock == 0 ? i2c0 : i2c1,
        options.i2cSpeed,
        WII_EXTENSION_I2C_ADDR);
    wii->begin();
    wii->start();

    // Run during setup to catch boot selection mode
    wii->poll();

    if (wii->extensionType == WII_EXTENSION_NUNCHUCK) {
        buttonZ = wii->buttonZ;
        buttonC = wii->buttonC;

        leftX = map(wii->joy1X,0,1023,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        leftY = map(wii->joy1Y,1023,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        rightX = GAMEPAD_JOYSTICK_MID;
        rightY = GAMEPAD_JOYSTICK_MID;

        triggerLeft = 0;
        triggerRight = 0;
    } else if ((wii->extensionType == WII_EXTENSION_CLASSIC) || (wii->extensionType == WII_EXTENSION_CLASSIC_PRO)) {
        buttonA = wii->buttonA;
        buttonB = wii->buttonB;
        buttonX = wii->buttonX;
        buttonY = wii->buttonY;
        buttonL = wii->buttonZL;
        buttonZL = wii->buttonLT;
        buttonR = wii->buttonZR;
        buttonZR = wii->buttonRT;
        dpadUp = wii->directionUp;
        dpadDown = wii->directionDown;
        dpadLeft = wii->directionLeft;
        dpadRight = wii->directionRight;
        buttonSelect = wii->buttonMinus;
        buttonStart = wii->buttonPlus;
        buttonHome = wii->buttonHome;

        if (wii->extensionType == WII_EXTENSION_CLASSIC) {
            triggerLeft  = wii->triggerLeft;
            triggerRight = wii->triggerRight;
        }

        leftX = map(wii->joy1X,0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        leftY = map(wii->joy1Y,WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        rightX = map(wii->joy2X,0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        rightY = map(wii->joy2Y,WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    } else if (wii->extensionType == WII_EXTENSION_GUITAR) {
        buttonSelect = wii->buttonMinus;
        buttonStart = wii->buttonPlus;

        dpadUp = wii->directionUp;
        dpadDown = wii->directionDown;

        buttonB = wii->fretGreen;
        buttonA = wii->fretRed;
        buttonX = wii->fretYellow;
        buttonY = wii->fretBlue;
        buttonL = wii->fretOrange;

        // whammy currently maps to Joy2X in addition to the raw whammy value
        whammyBar = wii->whammyBar;
        buttonR = wii->pedalButton;

        leftX = map(wii->joy1X,0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        leftY = map(wii->joy1Y,WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        rightX = map(wii->joy2X,0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        rightY = GAMEPAD_JOYSTICK_MID;

        triggerLeft = 0;
        triggerRight = 0;
    } else if (wii->extensionType == WII_EXTENSION_TAIKO) {
        buttonL = wii->rimLeft;
        buttonR = wii->rimRight;

        dpadLeft = wii->drumLeft;
        buttonA = wii->drumRight;
    }
}

void WiiExtensionInput::process() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    if (nextTimer < getMillis()) {
        wii->poll();
        if (wii->extensionType != WII_EXTENSION_NONE) {
            currentConfig = &extensionConfigs[wii->extensionType];

            //for (const auto& [extensionButton, value] : currentConfig->buttonMap) {
            //    WII_SET_MASK(buttonState, wii->getController()->buttons[extensionButton], value);
            //}

            if (wii->extensionType == WII_EXTENSION_NUNCHUCK) {
                buttonZ = wii->getController()->buttons[WiiButtons::WII_BUTTON_Z];
                buttonC = wii->getController()->buttons[WiiButtons::WII_BUTTON_C];

                leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,1023,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],1023,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightX = GAMEPAD_JOYSTICK_MID;
                rightY = GAMEPAD_JOYSTICK_MID;

                triggerLeft = 0;
                triggerRight = 0;
            } else if ((wii->extensionType == WII_EXTENSION_CLASSIC) || (wii->extensionType == WII_EXTENSION_CLASSIC_PRO)) {
                buttonA = wii->getController()->buttons[WiiButtons::WII_BUTTON_A];
                buttonB = wii->getController()->buttons[WiiButtons::WII_BUTTON_B];
                buttonX = wii->getController()->buttons[WiiButtons::WII_BUTTON_X];
                buttonY = wii->getController()->buttons[WiiButtons::WII_BUTTON_Y];
                buttonL = wii->getController()->buttons[WiiButtons::WII_BUTTON_L];
                buttonZL = wii->getController()->buttons[WiiButtons::WII_BUTTON_ZL];
                buttonR = wii->getController()->buttons[WiiButtons::WII_BUTTON_R];
                buttonZR = wii->getController()->buttons[WiiButtons::WII_BUTTON_ZR];
                buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
                buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];
                buttonHome = wii->getController()->buttons[WiiButtons::WII_BUTTON_HOME];
                dpadUp = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_UP];
                dpadDown = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN];
                dpadLeft = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT];
                dpadRight = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT];

                if (wii->extensionType == WII_EXTENSION_CLASSIC) {
                    triggerLeft  = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT];
                    triggerRight = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT];
                }

                leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            } else if (wii->extensionType == WII_EXTENSION_GUITAR) {
                buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
                buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

                dpadUp = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_UP];
                dpadDown = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN];

                buttonB = wii->getController()->buttons[GuitarButtons::GUITAR_GREEN];
                buttonA = wii->getController()->buttons[GuitarButtons::GUITAR_RED];
                buttonX = wii->getController()->buttons[GuitarButtons::GUITAR_YELLOW];
                buttonY = wii->getController()->buttons[GuitarButtons::GUITAR_BLUE];
                buttonL = wii->getController()->buttons[GuitarButtons::GUITAR_ORANGE];

                // whammy currently maps to Joy2X in addition to the raw whammy value
                whammyBar = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X];
                buttonR = wii->getController()->buttons[GuitarButtons::GUITAR_PEDAL];

                leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightY = GAMEPAD_JOYSTICK_MID;

                triggerLeft = 0;
                triggerRight = 0;
            } else if (wii->extensionType == WII_EXTENSION_TAIKO) {
                buttonL = wii->getController()->buttons[TaikoButtons::TATA_KAT_LEFT];
                buttonR = wii->getController()->buttons[TaikoButtons::TATA_KAT_RIGHT];

                dpadLeft = wii->getController()->buttons[TaikoButtons::TATA_DON_LEFT];
                buttonA = wii->getController()->buttons[TaikoButtons::TATA_DON_RIGHT];
            } else if (wii->extensionType == WII_EXTENSION_DRUMS) {
                buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
                buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

                buttonB = wii->getController()->buttons[DrumButtons::DRUM_RED];
                buttonA = wii->getController()->buttons[DrumButtons::DRUM_GREEN];
                buttonX = wii->getController()->buttons[DrumButtons::DRUM_YELLOW];
                buttonY = wii->getController()->buttons[DrumButtons::DRUM_BLUE];
                buttonL = wii->getController()->buttons[DrumButtons::DRUM_ORANGE];
                buttonZR = wii->getController()->buttons[DrumButtons::DRUM_PEDAL];

                leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightX = GAMEPAD_JOYSTICK_MID;
                rightY = GAMEPAD_JOYSTICK_MID;

                triggerLeft = 0;
                triggerRight = 0;
            } else if (wii->extensionType == WII_EXTENSION_TURNTABLE) {
                buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
                buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

                dpadLeft = wii->getController()->directionalPad[TurntableDirectionalPad::TURNTABLE_LEFT_GREEN];
                dpadUp = wii->getController()->directionalPad[TurntableDirectionalPad::TURNTABLE_LEFT_RED];
                dpadRight = wii->getController()->directionalPad[TurntableDirectionalPad::TURNTABLE_LEFT_BLUE];

                buttonY = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN];
                buttonX = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_RED];
                buttonA = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE];

                buttonZR = wii->getController()->buttons[TurntableButtons::TURNTABLE_EUPHORIA];

                leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightX = map(wii->getController()->analogState[TurntableAnalogs::TURNTABLE_RIGHT],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
                rightY = map(wii->getController()->analogState[TurntableAnalogs::TURNTABLE_LEFT],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);

                triggerLeft  = wii->getController()->analogState[TurntableAnalogs::TURNTABLE_EFFECTS];
                triggerRight = wii->getController()->analogState[TurntableAnalogs::TURNTABLE_CROSSFADE];
            }
        } else {
            currentConfig = NULL;
        }
              
        nextTimer = getMillis() + uIntervalMS;
    }

    if (currentConfig != NULL) {
        gamepad->state.lx = bounds(leftX,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        gamepad->state.ly = bounds(leftY,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        gamepad->state.rx = bounds(rightX,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        gamepad->state.ry = bounds(rightY,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);

        if ((wii->extensionType == WII_EXTENSION_CLASSIC) || (wii->extensionType == WII_EXTENSION_TURNTABLE)) {
            gamepad->hasAnalogTriggers = true;
            gamepad->state.lt = triggerLeft;
            gamepad->state.rt = triggerRight;
        } else {
            gamepad->hasAnalogTriggers = false;
        }

        if (buttonC) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_C];
        if (buttonZ) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_Z];

        if (buttonA) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_A];
        if (buttonB) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_B];
        if (buttonX) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_X];
        if (buttonY) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_Y];
        if (buttonL) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_L];
        if (buttonZL) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_ZL];
        if (buttonR) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_R];
        if (buttonZR) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_ZR];
        if (buttonSelect) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_MINUS];
        if (buttonStart) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_PLUS];
        if (buttonHome) gamepad->state.buttons |= currentConfig->buttonMap[WiiButtons::WII_BUTTON_HOME];
        if (dpadUp) gamepad->state.dpad |= currentConfig->dpadMap[WiiDirectionalPad::WII_DIRECTION_UP];
        if (dpadDown) gamepad->state.dpad |= currentConfig->dpadMap[WiiDirectionalPad::WII_DIRECTION_DOWN];
        if (dpadLeft) gamepad->state.dpad |= currentConfig->dpadMap[WiiDirectionalPad::WII_DIRECTION_LEFT];
        if (dpadRight) gamepad->state.dpad |= currentConfig->dpadMap[WiiDirectionalPad::WII_DIRECTION_RIGHT];
    }
}

uint16_t WiiExtensionInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t WiiExtensionInput::bounds(uint16_t x, uint16_t out_min, uint16_t out_max) {
    if (x > out_max) x = out_max;
    if (x < out_min) x = out_min;
    return x;
}
