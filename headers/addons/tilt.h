#ifndef _Tilt_H
#define _Tilt_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef TILT_ENABLED
#define TILT_ENABLED 0
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

class TiltInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Tilt Setup
	virtual void process();     // Tilt Process
	virtual void preprocess();  // Tilt Pre-Process (Cheat)
	virtual std::string name() { return TiltName; }
private:
	void debounce();
	void SOCDTiltClean(SOCDMode);
	uint8_t SOCDCombine(SOCDMode, uint8_t);
	uint8_t SOCDGamepadClean(uint8_t);
	void OverrideGamepad(Gamepad*, uint8_t, uint8_t);
	uint8_t dDebLeftState;          // Debounce State (stored)
	uint8_t dDebRightState;          // Debounce State (stored)
	uint8_t tiltLeftState;          // Tilt State
	uint8_t tiltRightState;          // Tilt Right Analog State
	DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
	DpadDirection leftLastTiltUD; // Tilt Last Up-Down
	DpadDirection leftLastTiltLR; // Gamepad Last Left-Right
	DpadDirection rightLastTiltUD; // Tilt Last Up-Down
	DpadDirection rightLastTiltLR; // Gamepad Last Left-Right
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
