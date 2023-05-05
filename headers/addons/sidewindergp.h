#ifndef _SidewinderGP_H
#define _SidewinderGP_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#include "BoardConfig.h"

#ifndef SIDEWINDERGP_INPUT_ENABLED
#define SIDEWINDERGP_INPUT_ENABLED 0
#endif

#ifndef SIDEWINDERGP_PIN_TRIGGER
#define SIDEWINDERGP_PIN_TRIGGER    -1
#endif

#ifndef SIDEWINDERGP_PIN_CLOCK
#define SIDEWINDERGP_PIN_CLOCK    -1
#endif

#ifndef SIDEWINDERGP_PIN_DATA
#define SIDEWINDERGP_PIN_DATA    -1
#endif


// SidewinderGP Module Name
#define SideWinderGPName "Sidewinder Gamepad"

class SidewinderGPInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Setup
	virtual void process();     // Process
	virtual void preprocess() {}
    virtual std::string name() { return SideWinderGPName; }
private:
	uint8_t sidewinderGPPinTrigger;
	uint8_t sidewinderGPPinClock;
	uint8_t sidewinderGPPinData;
};

#endif  // _SidewinderGP_H_