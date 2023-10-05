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

#ifndef ROTATE1_FACTOR_LEFT_X
#define ROTATE1_FACTOR_LEFT_X -1
#endif

#ifndef ROTATE1_FACTOR_LEFT_Y
#define ROTATE1_FACTOR_LEFT_Y -1
#endif

#ifndef ROTATE1_FACTOR_RIGHT_X
#define ROTATE1_FACTOR_RIGHT_X -1
#endif

#ifndef ROTATE1_FACTOR_RIGHT_Y
#define ROTATE1_FACTOR_RIGHT_Y -1
#endif

#ifndef PIN_ROTATE_2
#define PIN_ROTATE_2 -1
#endif

#ifndef ROTATE2_FACTOR_LEFT_X
#define ROTATE2_FACTOR_LEFT_X -1
#endif

#ifndef ROTATE2_FACTOR_LEFT_Y
#define ROTATE2_FACTOR_LEFT_Y -1
#endif

#ifndef ROTATE2_FACTOR_RIGHT_X
#define ROTATE2_FACTOR_RIGHT_X -1
#endif

#ifndef ROTATE2_FACTOR_RIGHT_Y
#define ROTATE2_FACTOR_RIGHT_Y -1
#endif


#ifndef PIN_ROTATE_LEFT_ANALOG_UP
#define PIN_ROTATE_LEFT_ANALOG_UP -1
#endif

#ifndef PIN_ROTATE_LEFT_ANALOG_DOWN
#define PIN_ROTATE_LEFT_ANALOG_DOWN -1
#endif

#ifndef PIN_ROTATE_LEFT_ANALOG_LEFT
#define PIN_ROTATE_LEFT_ANALOG_LEFT -1
#endif

#ifndef PIN_ROTATE_LEFT_ANALOG_RIGHT
#define PIN_ROTATE_LEFT_ANALOG_RIGHT -1
#endif

#ifndef PIN_ROTATE_RIGHT_ANALOG_UP
#define PIN_ROTATE_RIGHT_ANALOG_UP -1
#endif

#ifndef PIN_ROTATE_RIGHT_ANALOG_DOWN
#define PIN_ROTATE_RIGHT_ANALOG_DOWN -1
#endif

#ifndef PIN_ROTATE_RIGHT_ANALOG_LEFT
#define PIN_ROTATE_RIGHT_ANALOG_LEFT -1
#endif

#ifndef PIN_ROTATE_RIGHT_ANALOG_RIGHT
#define PIN_ROTATE_RIGHT_ANALOG_RIGHT -1
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
	uint8_t rotate1FactorLeftX;
    uint8_t rotate1FactorLeftY;
    uint8_t rotate1FactorRightX;
    uint8_t rotate1FactorRightY;
	uint8_t pinRotate2;
    uint8_t rotate2FactorLeftX;
    uint8_t rotate2FactorLeftY;
    uint8_t rotate2FactorRightX;
    uint8_t rotate2FactorRightY;
	uint8_t pinRotateLeftAnalogDown;
	uint8_t pinRotateLeftAnalogUp;
	uint8_t pinRotateLeftAnalogLeft;
	uint8_t pinRotateLeftAnalogRight;
	uint8_t pinRotateRightAnalogDown;
	uint8_t pinRotateRightAnalogUp;
	uint8_t pinRotateRightAnalogLeft;
	uint8_t pinRotateRightAnalogRight;
	SOCDMode rotateSOCDMode;
};

#endif  // _Rotate_H
