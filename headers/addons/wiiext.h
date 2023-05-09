#ifndef _WIIExtensionAddon_H
#define _WIIExtensionAddon_H

#include <string>
#include <stdint.h>
#include <hardware/i2c.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "WiiExtension.h"

// WiiExtension Module Name
#define WiiExtensionName "WiiExtension"

#ifndef WII_EXTENSION_ENABLED
#define WII_EXTENSION_ENABLED 0
#endif

#ifndef WII_EXTENSION_I2C_ADDR
#define WII_EXTENSION_I2C_ADDR 0x52
#endif

#ifndef WII_EXTENSION_I2C_SDA_PIN
#define WII_EXTENSION_I2C_SDA_PIN -1
#endif

#ifndef WII_EXTENSION_I2C_SCL_PIN
#define WII_EXTENSION_I2C_SCL_PIN -1
#endif

#ifndef WII_EXTENSION_I2C_BLOCK
#define WII_EXTENSION_I2C_BLOCK i2c0
#endif

#ifndef WII_EXTENSION_I2C_SPEED
#define WII_EXTENSION_I2C_SPEED 400000
#endif

class WiiExtensionInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // WiiExtension Setup
	virtual void process();     // WiiExtension Process
	virtual void preprocess() {}
	virtual std::string name() { return WiiExtensionName; }
private:
    WiiExtension * wii;
    uint32_t uIntervalMS;
    uint32_t nextTimer;

    bool buttonC = false;
    bool buttonZ = false;

    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool buttonL = false;
    bool buttonZL = false;
    bool buttonR = false;
    bool buttonZR = false;

    bool buttonSelect = false;
    bool buttonStart = false;
    bool buttonHome = false;

    bool dpadUp     = false;
    bool dpadDown   = false;
    bool dpadLeft   = false;
    bool dpadRight  = false;

    uint16_t triggerLeft  = 0;
    uint16_t triggerRight = 0;
    uint16_t whammyBar    = 0;

    uint16_t leftX = 0;
    uint16_t leftY = 0;
    uint16_t rightX = 0;
    uint16_t rightY = 0;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif  // _WIIExtensionAddon_H
