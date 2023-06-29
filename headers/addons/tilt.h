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

#ifndef PIN_TILT_2
#define PIN_TILT_2 -1
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
	uint8_t tiltRightState;          // Tilt Righjt Analog State
	DpadDirection lastGPUD; // Gamepad Last Up-Down
	DpadDirection lastGPLR; // Gamepad Last Left-Right
	DpadDirection lastTiltUD; // Tilt Last Up-Down
	DpadDirection lastTiltLR; // Gamepad Last Left-Right
	uint32_t dpadTime[4];
	uint8_t pinTilt1;
	uint8_t pinTilt2;
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
