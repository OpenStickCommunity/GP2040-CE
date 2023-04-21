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
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    
    if (buttonA) gamepad->state.buttons |= GAMEPAD_MASK_B1;

    if (nextTimer < getMillis()) {
        wii->poll();
        
        //buttonA = wii->buttonA;
        buttonA = !buttonA;
        //if (wii->buttonZ) gamepad->state.buttons |= GAMEPAD_MASK_B3;

        //gamepad->state.lx = wii->joy1X;
        //gamepad->state.ly = wii->joy1Y;
        
        nextTimer = getMillis() + uIntervalMS;

        //float result;
        //uint32_t readValue;
        //if ( ads->readRegister(STATUS) & REGISTER_STATUS_DRDY ) {
        //    readValue = ads->readConversionResult();
        //    result = readValue / float(ADS_MAX); // gives us 0.0f to 1.0f (actual voltage is times voltage)
        //    pins.A[channelHop] = result;
        //    channelHop = (channelHop+1) % 4; // Loop 0-3
        //    ads->setChannel(channelHop);
        //    nextTimer = getMillis() + uIntervalMS; // interval for read (we can't be too fast)
        //}
    }
}

int16_t WiiExtensionInput::map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
