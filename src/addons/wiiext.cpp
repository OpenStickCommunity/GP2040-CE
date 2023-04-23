#include "addons/wiiext.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool WiiExtensionInput::available() {
	AddonOptions options = Storage::getInstance().getAddonOptions();
    return true;
//	extraButtonMap = options.extraButtonMap;
//	extraButtonPin = options.extraButtonPin;
//	return options.ExtraButtonAddonEnabled &&
//		extraButtonMap != 0 &&
//		extraButtonPin != (uint8_t)-1;
}

void WiiExtensionInput::setup() {
    nextTimer = getMillis();

#if WII_EXTENSION_DEBUG==true
    stdio_init_all();
#endif

    uIntervalMS = 1;
    
    wii = new WiiExtension(1,
        WII_EXTENSION_I2C_SDA_PIN,
        WII_EXTENSION_I2C_SCL_PIN,
        WII_EXTENSION_I2C_BLOCK,
        WII_EXTENSION_I2C_SPEED);
    wii->begin();
    wii->start();
}

void WiiExtensionInput::process() {
    if (nextTimer < getMillis()) {
        wii->poll();
        
        buttonZ = wii->buttonZ;
        buttonC = wii->buttonC;
        buttonA = wii->buttonA;
        buttonB = wii->buttonB;
        buttonX = wii->buttonX;
        buttonY = wii->buttonY;

        leftX = map(wii->joy1X,35,228,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        leftY = map(wii->joy1Y,27,220,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
               
        nextTimer = getMillis() + uIntervalMS;
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    if (buttonC) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (buttonZ) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (buttonA) gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (buttonB) gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (buttonX) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (buttonY) gamepad->state.buttons |= GAMEPAD_MASK_R1;

    gamepad->state.lx = leftX;
    gamepad->state.ly = leftY;
}

int16_t WiiExtensionInput::map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
