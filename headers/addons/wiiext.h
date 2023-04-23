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
    bool buttonL = true;
    bool buttonZL = true;
    bool buttonR = true;
    bool buttonZR = true;

    bool buttonSelect = true;
    bool buttonStart = true;
    bool buttonHome = true;

    bool dpadUp     = false;
    bool dpadDown   = false;
    bool dpadLeft   = false;
    bool dpadRight  = false;

    uint16_t leftX;
    uint16_t leftY;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif  // _WIIExtensionAddon_H