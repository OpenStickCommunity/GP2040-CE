#ifndef _Rotation_H
#define _Rotation_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef ROTATE_ENABLED
#define ROTATE_ENABLED 0
#endif

#ifndef PIN_ROTATION_1
#define PIN_ROTATION_1 -1
#endif

#ifndef ROTATE1_FACTOR_LEFT
#define ROTATE1_FACTOR_LEFT -1
#endif

#ifndef ROTATE1_FACTOR_RIGHT
#define ROTATE1_FACTOR_RIGHT -1
#endif

#ifndef PIN_ROTATE_2
#define PIN_ROTATE_2 -1
#endif

#ifndef ROTATE2_FACTOR_LEFT
#define ROTATE2_FACTOR_LEFT -1
#endif

#ifndef ROTATE2_FACTOR_RIGHT
#define ROTATE2_FACTOR_RIGHT -1
#endif

#ifndef ROTATE_SOCD_MODE
#define ROTATE_SOCD_MODE SOCD_MODE_NEUTRAL
#endif

// Rotate Module Name
#define RotateName "Rotate"

class RotateInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Rotate Setup
	virtual void process();     // Rotate Process
	virtual void preprocess();  // Rotate Pre-Process (Cheat)
	virtual std::string name() { return RotateName; }
private:
	void debounce();
	void SOCDRotateClean(SOCDMode);
	uint8_t SOCDCombine(SOCDMode, uint8_t);
	uint8_t SOCDGamepadClean(uint8_t);
	void OverrideGamepad(Gamepad*, uint8_t, uint8_t);
	uint8_t dDebLeftState;          // Debounce State (stored)
	uint8_t dDebRightState;          // Debounce State (stored)
	uint8_t rotateLeftState;          // Rotate State
	uint8_t rotateRightState;          // Rotate Righjt Analog State
	DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
	DpadDirection lastRotateUD; // Rotate Last Up-Down
	DpadDirection lastRotateLR; // Gamepad Last Left-Right
	uint32_t dpadTime[4];
	uint8_t pinRotate1;
	uint8_t rotate1FactorLeft;
    uint8_t rotate1FactorRight;
	uint8_t pinRotate2;
    uint8_t rotate2FactorLeft;
    uint8_t rotate2FactorRight;
	SOCDMode rotateSOCDMode;
};

#endif  // _Rotate_H
