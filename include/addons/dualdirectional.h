#ifndef _DualDirectional_H
#define _DualDirectional_H

#include "gpaddon.h"

#include "GamepadEnums.h"

// The available combinational methods
enum DualDirectionalCombinationMode
{
    DUAL_COMBINE_MODE_MIXED = 0,
	DUAL_COMBINE_MODE_GAMEPAD,
    DUAL_COMBINE_MODE_DUAL,
    DUAL_COMBINE_MODE_NONE
};

#ifndef PIN_DUAL_DIRECTIONAL_UP
#define PIN_DUAL_DIRECTIONAL_UP -1
#endif

#ifndef PIN_DUAL_DIRECTIONAL_DOWN
#define PIN_DUAL_DIRECTIONAL_DOWN -1
#endif

#ifndef PIN_DUAL_DIRECTIONAL_LEFT
#define PIN_DUAL_DIRECTIONAL_LEFT -1
#endif

#ifndef PIN_DUAL_DIRECTIONAL_RIGHT
#define PIN_DUAL_DIRECTIONAL_RIGHT -1
#endif

#ifndef DUAL_DIRECTIONAL_STICK_MODE
#define DUAL_DIRECTIONAL_STICK_MODE DPAD_MODE_DIGITAL
#endif

#ifndef DUAL_DIRECTIONAL_COMBINE_MODE
#define DUAL_DIRECTIONAL_COMBINE_MODE DUAL_COMBINE_MODE_MIXED
#endif

// Dual Directional Module Name
#define DualDirectionalName "DualDirectional"

class DualDirectionalInput : public GPAddon {
public:
	virtual bool available();   // GPAddon available
	virtual void setup();       // Dual Directional Setup
	virtual void process();     // Dual Directional Process
    virtual std::string name() { return DualDirectionalName; }
private:
    void debounce();
    uint8_t dDebState;          // Debounce State (stored)
    uint8_t dpadState;          // Dual Directional State
    uint32_t dpadTime[4];
};

#endif  // _DualDirectional_H
