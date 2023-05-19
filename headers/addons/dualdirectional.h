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

#ifndef DUAL_DIRECTIONAL_ENABLED
#define DUAL_DIRECTIONAL_ENABLED 0
#endif

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
    virtual bool available();
	virtual void setup();       // Dual Directional Setup
	virtual void process();     // Dual Directional Process
    virtual void preprocess();  // Dual Directional Pre-Process (Cheat)
    virtual std::string name() { return DualDirectionalName; }
private:
    void debounce();
    uint8_t gpadToBinary(DpadMode, GamepadState);
    void SOCDDualClean(SOCDMode);
    uint8_t SOCDCombine(SOCDMode, uint8_t);
    uint8_t SOCDGamepadClean(uint8_t, bool isLastWin);
    void OverrideGamepad(Gamepad *, DpadMode, uint8_t);
    const SOCDMode getSOCDMode(GamepadOptions&);
    uint8_t dDebState;          // Debounce State (stored)
    uint8_t dualState;          // Dual Directional State
    DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
    DpadDirection lastDualUD; // Dual Last Up-Down
    DpadDirection lastDualLR; // Gamepad Last Left-Right
    uint32_t dpadTime[4];
    uint8_t pinDualDirDown;
    uint8_t pinDualDirUp;
    uint8_t pinDualDirLeft;
    uint8_t pinDualDirRight;
    uint8_t combineMode;
    DpadMode dpadMode;
};

#endif  // _DualDirectional_H
