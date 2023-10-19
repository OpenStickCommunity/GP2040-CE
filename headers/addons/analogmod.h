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
#define TILT1_FACTOR_LEFT_X 35
#endif

#ifndef TILT1_FACTOR_LEFT_Y
#define TILT1_FACTOR_LEFT_Y 35
#endif

#ifndef TILT1_FACTOR_RIGHT_X
#define TILT1_FACTOR_RIGHT_X 35
#endif

#ifndef TILT1_FACTOR_RIGHT_Y
#define TILT1_FACTOR_RIGHT_Y 35
#endif

#ifndef PIN_TILT_2
#define PIN_TILT_2 -1
#endif

#ifndef TILT2_FACTOR_LEFT_X
#define TILT2_FACTOR_LEFT_X 65
#endif

#ifndef TILT2_FACTOR_LEFT_Y
#define TILT2_FACTOR_LEFT_Y 65
#endif

#ifndef TILT2_FACTOR_RIGHT_X
#define TILT2_FACTOR_RIGHT_X 65
#endif

#ifndef TILT2_FACTOR_RIGHT_Y
#define TILT2_FACTOR_RIGHT_Y 65
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

#ifndef PIN_ROTATE_1
#define PIN_ROTATE_1 -1
#endif

#ifndef PIN_ROTATE_2
#define PIN_ROTATE_2 -1
#endif

#ifndef ROTATE1_FACTOR_LEFT
#define ROTATE1_FACTOR_LEFT 15
#endif

#ifndef ROTATE2_FACTOR_LEFT
#define ROTATE2_FACTOR_LEFT 345
#endif

#ifndef ROTATE3_FACTOR_LEFT
#define ROTATE3_FACTOR_LEFT 0
#endif

#ifndef ROTATE4_FACTOR_LEFT
#define ROTATE4_FACTOR_LEFT 0
#endif

#ifndef ROTATE5_FACTOR_LEFT
#define ROTATE5_FACTOR_LEFT 0
#endif

#ifndef ROTATE6_FACTOR_LEFT
#define ROTATE6_FACTOR_LEFT 0
#endif

#ifndef ROTATE1_FACTOR_RIGHT
#define ROTATE1_FACTOR_RIGHT 15
#endif

#ifndef ROTATE2_FACTOR_RIGHT
#define ROTATE2_FACTOR_RIGHT 345
#endif

#ifndef ROTATE3_FACTOR_RIGHT
#define ROTATE3_FACTOR_RIGHT 0
#endif

#ifndef ROTATE4_FACTOR_RIGHT
#define ROTATE4_FACTOR_RIGHT 0
#endif

#ifndef ROTATE5_FACTOR_RIGHT
#define ROTATE5_FACTOR_RIGHT 0
#endif

#ifndef ROTATE6_FACTOR_RIGHT
#define ROTATE6_FACTOR_RIGHT 0
#endif

#ifndef ANALOGMOD_LEFT_SOCD_MODE
#define ANALOGMOD_LEFT_SOCD_MODE SOCD_MODE_NEUTRAL
#endif

#ifndef ANALOGMOD_RIGHT_SOCD_MODE
#define ANALOGMOD_RIGHT_SOCD_MODE SOCD_MODE_NEUTRAL
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
	void SOCDAnalogModClean(SOCDMode, SOCDMode);
	void OverrideGamepad(Gamepad*, uint8_t, uint8_t);
	uint8_t dDebLeftState;           // Debounce State (stored)
	uint8_t dDebRightState;          // Debounce State (stored)
	uint8_t analogmodLeftState;           // AnalogMod Left Analog State
	uint8_t analogmodRightState;          // AnalogMod Right Analog State
	DpadDirection leftLastAnalogModUD;
	DpadDirection leftLastAnalogModLR;
	DpadDirection rightLastAnalogModUD;
	DpadDirection rightLastAnalogModLR;
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
	uint8_t pinRotate2;
	uint16_t rotate1FactorLeft;
    uint16_t rotate2FactorLeft;
	uint16_t rotate3FactorLeft;
	uint16_t rotate4FactorLeft;
	uint16_t rotate5FactorLeft;
	uint16_t rotate6FactorLeft;
    uint16_t rotate1FactorRight;
	uint16_t rotate2FactorRight;
	uint16_t rotate3FactorRight;
	uint16_t rotate4FactorRight;
	uint16_t rotate5FactorRight;
	uint16_t rotate6FactorRight;
	SOCDMode analogmodLeftSOCDMode;
	SOCDMode analogmodRightSOCDMode;
};

#endif  // _AnalogMod_H
