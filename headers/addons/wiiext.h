#ifndef _WIIExtensionAddon_H
#define _WIIExtensionAddon_H

#include <string>
#include <hardware/i2c.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "WiiExtension.h"

// WiiExtension Module Name
#define WiiExtensionName "WiiExtension"

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

    bool buttonC = true;
    bool buttonZ = false;
    bool buttonA = true;
    bool buttonB = true;
    bool buttonX = true;
    bool buttonY = true;

    int16_t leftX;
    int16_t leftY;

    int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max);
};

#endif  // _WIIExtensionAddon_H