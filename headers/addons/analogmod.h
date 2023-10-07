#ifndef _AnalogMod_H
#define _AnalogMod_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef ANALOGMOD_ENABLED
#define ANALOGMOD_ENABLED 0
#endif

#ifndef PIN_TILT_1
#define PIN_TILT_1 -1
#endif

#ifndef TILT1_FACTOR_LEFT_X
#define TILT1_FACTOR_LEFT_X -1
#endif

#ifndef TILT1_FACTOR_LEFT_Y
#define TILT1_FACTOR_LEFT_Y -1
#endif

#ifndef TILT1_FACTOR_RIGHT_X
#define TILT1_FACTOR_RIGHT_X -1
#endif

#ifndef TILT1_FACTOR_RIGHT_Y
#define TILT1_FACTOR_RIGHT_Y -1
#endif

#ifndef PIN_TILT_2
#define PIN_TILT_2 -1
#endif

#ifndef TILT2_FACTOR_LEFT_X
#define TILT2_FACTOR_LEFT_X -1
#endif

#ifndef TILT2_FACTOR_LEFT_Y
#define TILT2_FACTOR_LEFT_Y -1
#endif

#ifndef TILT2_FACTOR_RIGHT_X
#define TILT2_FACTOR_RIGHT_X -1
#endif

#ifndef TILT2_FACTOR_RIGHT_Y
#define TILT2_FACTOR_RIGHT_Y -1
#endif


#ifndef PIN_ANALOGMOD_LEFT_ANALOG_UP
#define PIN_ANALOGMOD_LEFT_ANALOG_UP -1
#endif

#ifndef PIN_ANALOGMOD_LEFT_ANALOG_DOWN
#define PIN_ANALOGMOD_LEFT_ANALOG_DOWN -1
#endif

#ifndef PIN_ANALOGMOD_LEFT_ANALOG_LEFT
#define PIN_ANALOGMOD_LEFT_ANALOG_LEFT -1
#endif

#ifndef PIN_ANALOGMOD_LEFT_ANALOG_RIGHT
#define PIN_ANALOGMOD_LEFT_ANALOG_RIGHT -1
#endif

#ifndef PIN_ANALOGMOD_RIGHT_ANALOG_UP
#define PIN_ANALOGMOD_RIGHT_ANALOG_UP -1
#endif

#ifndef PIN_ANALOGMOD_RIGHT_ANALOG_DOWN
#define PIN_ANALOGMOD_RIGHT_ANALOG_DOWN -1
#endif

#ifndef PIN_ANALOGMOD_RIGHT_ANALOG_LEFT
#define PIN_ANALOGMOD_RIGHT_ANALOG_LEFT -1
#endif

#ifndef PIN_ANALOGMOD_RIGHT_ANALOG_RIGHT
#define PIN_ANALOGMOD_RIGHT_ANALOG_RIGHT -1
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

#ifndef ANALOGMOD_SOCD_MODE
#define ANALOGMOD_SOCD_MODE SOCD_MODE_NEUTRAL
#endif

// AnalogMod Module Name
#define AnalogModName "AnalogMod"

class AnalogModInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // AnalogMod Setup
	virtual void process();     // AnalogMod Process
	virtual void preprocess();  // AnalogMod Pre-Process (Cheat)
	virtual std::string name() { return AnalogModName; }
private:
	void debounce();
	void SOCDAnalogModClean(SOCDMode);
	uint8_t SOCDCombine(SOCDMode, uint8_t);
	uint8_t SOCDGamepadClean(uint8_t);
	void OverrideGamepad(Gamepad*, uint8_t, uint8_t);
	uint8_t dDebLeftState;           // Debounce State (stored)
	uint8_t dDebRightState;          // Debounce State (stored)
	uint8_t analogmodLeftState;           // AnalogMod Left Analog State
	uint8_t analogmodRightState;          // AnalogMod Right Analog State
	DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
	DpadDirection lastAnalogModUD; // Gamepad Last Up-Down
	DpadDirection lastAnalogModLR; // Gamepad Last Left-Right
	uint32_t dpadTime[4];
	uint8_t pinTilt1;
	uint8_t tilt1FactorLeftX;
    uint8_t tilt1FactorLeftY;
    uint8_t tilt1FactorRightX;
    uint8_t tilt1FactorRightY;
	uint8_t pinTilt2;
    uint8_t tilt2FactorLeftX;
    uint8_t tilt2FactorLeftY;
    uint8_t tilt2FactorRightX;
    uint8_t tilt2FactorRightY;
	uint8_t pinAnalogModLeftAnalogDown;
	uint8_t pinAnalogModLeftAnalogUp;
	uint8_t pinAnalogModLeftAnalogLeft;
	uint8_t pinAnalogModLeftAnalogRight;
	uint8_t pinAnalogModRightAnalogDown;
	uint8_t pinAnalogModRightAnalogUp;
	uint8_t pinAnalogModRightAnalogLeft;
	uint8_t pinAnalogModRightAnalogRight;
	uint8_t pinRotate1;
	uint8_t rotate1FactorLeft;
    uint8_t rotate1FactorRight;
	uint8_t pinRotate2;
    uint8_t rotate2FactorLeft;
    uint8_t rotate2FactorRight;
	SOCDMode analogmodSOCDMode;
};

#endif  // _AnalogMod_H
