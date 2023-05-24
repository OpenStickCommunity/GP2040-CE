#include "addons/wiiext.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool WiiExtensionInput::available() {
    const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
    AddonOptions options = Storage::getInstance().getAddonOptions();

    return (!boardOptions.hasI2CDisplay && (options.WiiExtensionAddonEnabled &&
        options.wiiExtensionSDAPin != (uint8_t)-1 &&
        options.wiiExtensionSCLPin != (uint8_t)-1));
}

void WiiExtensionInput::setup() {
    AddonOptions options = Storage::getInstance().getAddonOptions();
    nextTimer = getMillis();

#if WII_EXTENSION_DEBUG==true
    stdio_init_all();
#endif

    uIntervalMS = 0;
    
    wii = new WiiExtension(
        options.wiiExtensionSDAPin,
        options.wiiExtensionSCLPin,
        options.wiiExtensionBlock == 0 ? i2c0 : i2c1,
        options.wiiExtensionSpeed,
        WII_EXTENSION_I2C_ADDR);
    wii->begin();
    wii->start();
}

void WiiExtensionInput::process() {
    if (nextTimer < getMillis()) {
        wii->poll();
        
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
            dpadUp = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_UP];
            dpadDown = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN];
            dpadLeft = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT];
            dpadRight = wii->getController()->directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT];
            buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
            buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];
            buttonHome = wii->getController()->buttons[WiiButtons::WII_BUTTON_HOME];

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
    if (buttonL) gamepad->state.buttons |= GAMEPAD_MASK_L2;
    if (buttonZL) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (buttonR) gamepad->state.buttons |= GAMEPAD_MASK_R2;
    if (buttonZR) gamepad->state.buttons |= GAMEPAD_MASK_R1;
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
