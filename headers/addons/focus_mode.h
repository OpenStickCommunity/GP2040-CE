#ifndef _FocusMode_H
#define _FocusMode_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef FOCUS_MODE_ENABLED
#define FOCUS_MODE_ENABLED 0
#endif

#ifndef FOCUS_MODE_PIN
#define FOCUS_MODE_PIN -1
#endif

#ifndef FOCUS_MODE_BUTTON_MASK
#define FOCUS_MODE_BUTTON_MASK 0
#endif

#ifndef FOCUS_MODE_OLED_LOCK_ENABLED
#define FOCUS_MODE_OLED_LOCK_ENABLED 0
#endif

#ifndef FOCUS_MODE_RGB_LOCK_ENABLED
#define FOCUS_MODE_RGB_LOCK_ENABLED 0
#endif

#ifndef FOCUS_MODE_BUTTON_LOCK_ENABLED
#define FOCUS_MODE_BUTTON_LOCK_ENABLED 0
#endif

#ifndef FOCUS_MODE_MACRO_LOCK_ENABLED
#define FOCUS_MODE_MACRO_LOCK_ENABLED 1
#endif

// FocusMode Module Name
#define FocusModeName "FocusMode"

class FocusModeAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // FocusMode Setup
	virtual void process();     // FocusMode Process
	virtual void preprocess() {}
	virtual std::string name() { return FocusModeName; }
private:
	uint32_t buttonLockMask;
	uint8_t focusModePin;
};

#endif  // _FocusMode_H_