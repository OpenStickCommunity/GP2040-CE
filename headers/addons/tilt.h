#ifndef _Tilt_H
#define _Tilt_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef TILT_ENABLED
#define TILT_ENABLED 1
#endif

#ifndef PIN_TILT_1
#define PIN_TILT_1 -1
#endif

#ifndef PIN_TILT_2
#define PIN_TILT_2 -1
#endif

#ifndef PIN_TILT_FUNCTION
#define PIN_TILT_FUNCTION -1
#endif

#ifndef PIN_TILT_LEFT_ANALOG_UP
#define PIN_TILT_LEFT_ANALOG_UP -1
#endif

#ifndef PIN_TILT_LEFT_ANALOG_DOWN
#define PIN_TILT_LEFT_ANALOG_DOWN -1
#endif

#ifndef PIN_TILT_LEFT_ANALOG_LEFT
#define PIN_TILT_LEFT_ANALOG_LEFT -1
#endif

#ifndef PIN_TILT_LEFT_ANALOG_RIGHT
#define PIN_TILT_LEFT_ANALOG_RIGHT -1
#endif

#ifndef PIN_TILT_RIGHT_ANALOG_UP
#define PIN_TILT_RIGHT_ANALOG_UP -1
#endif

#ifndef PIN_TILT_RIGHT_ANALOG_DOWN
#define PIN_TILT_RIGHT_ANALOG_DOWN -1
#endif

#ifndef PIN_TILT_RIGHT_ANALOG_LEFT
#define PIN_TILT_RIGHT_ANALOG_LEFT -1
#endif

#ifndef PIN_TILT_RIGHT_ANALOG_RIGHT
#define PIN_TILT_RIGHT_ANALOG_RIGHT -1
#endif

#ifndef TILT_SOCD_MODE
#define TILT_SOCD_MODE SOCD_MODE_NEUTRAL
#endif

// Tilt Module Name
#define TiltName "Tilt"

// These constants define the adjustment factors for gamepad analog inputs under different tilt states.
// Defines the behavior of the left analog stick when the Tilt1 and Tilt2 buttons are pressed.
// The main purpose of the left analog stick is to move the character.
// Pressing it simultaneously with Tilt 1 will make the character walk.
// Pressing it simultaneously with Tilt 2 will make the character walk more slowly.
#define TILT1_FACTOR_LEFT_X 0.35  // Adjustment factor for left analog stick X direction
#define TILT1_FACTOR_LEFT_Y 0.45  // Adjustment factor for left analog stick Y direction
#define TILT2_FACTOR_LEFT_X 0.65  // Adjustment factor for left analog stick X direction
#define TILT2_FACTOR_LEFT_Y 0.35  // Adjustment factor for left analog stick Y direction

// The Right analog stick has 8 directions, which can be handled by pressing up, down, left, right, and simultaneously.
// This function adds to that the ability to tilt it at an angle closer to horizontal than diagonal.
#define TILT1_FACTOR_RIGHT_X 0.3  // Adjustment factor for right analog stick X direction
#define TILT1_FACTOR_RIGHT_Y 1.7  // Adjustment factor for right analog stick Y direction
#define TILT2_FACTOR_RIGHT_X 0.3  // Adjustment factor for right analog stick X direction
#define TILT2_FACTOR_RIGHT_Y 0.3  // Adjustment factor for right analog stick Y direction


class TiltInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Tilt Setup
	virtual void process();     // Tilt Process
	virtual void preprocess();  // Tilt Pre-Process (Cheat)
	virtual std::string name() { return TiltName; }
private:
	void debounce();
	uint8_t gpadToBinary(DpadMode, GamepadState);
	void SOCDTiltClean(SOCDMode);
	uint8_t SOCDCombine(SOCDMode, uint8_t);
	uint8_t SOCDGamepadClean(uint8_t);
	void OverrideGamepad(Gamepad*, uint8_t, uint8_t);
	uint8_t dDebLeftState;          // Debounce State (stored)
	uint8_t dDebRightState;          // Debounce State (stored)
	uint8_t tiltLeftState;          // Tilt State
	uint8_t tiltRightState;          // Tilt Righjt Analog State
	DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
	DpadDirection lastTiltUD; // Tilt Last Up-Down
	DpadDirection lastTiltLR; // Gamepad Last Left-Right
	uint32_t dpadTime[4];
	uint8_t pinTilt1;
	uint8_t pinTilt2;
	uint8_t pinTiltFunction;
	uint8_t pinTiltLeftAnalogDown;
	uint8_t pinTiltLeftAnalogUp;
	uint8_t pinTiltLeftAnalogLeft;
	uint8_t pinTiltLeftAnalogRight;
	uint8_t pinTiltRightAnalogDown;
	uint8_t pinTiltRightAnalogUp;
	uint8_t pinTiltRightAnalogLeft;
	uint8_t pinTiltRightAnalogRight;
	SOCDMode tiltSOCDMode;
};

#endif  // _Tilt_H
