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
    if (nextTimer < getMillis()) {
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
               
        nextTimer = getMillis() + uIntervalMS;
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.lx = leftX;
    gamepad->state.ly = leftY;
    gamepad->state.rx = rightX;
    gamepad->state.ry = rightY;

    if (wii->extensionType == WII_EXTENSION_CLASSIC) {
        gamepad->hasAnalogTriggers = true;
        gamepad->state.lt = triggerLeft;
        gamepad->state.rt = triggerRight;
    } else {
        gamepad->hasAnalogTriggers = false;
    }

    if (buttonC) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (buttonZ) gamepad->state.buttons |= GAMEPAD_MASK_B2;

    if (buttonA) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (buttonB) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (buttonX) gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (buttonY) gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (buttonL) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (buttonZL) gamepad->state.buttons |= GAMEPAD_MASK_L2;
    if (buttonR) gamepad->state.buttons |= GAMEPAD_MASK_R1;
    if (buttonZR) gamepad->state.buttons |= GAMEPAD_MASK_R2;
    if (buttonSelect) gamepad->state.buttons |= GAMEPAD_MASK_S1;
    if (buttonStart) gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (buttonHome) gamepad->state.buttons |= GAMEPAD_MASK_A1;
    if (dpadUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t WiiExtensionInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
