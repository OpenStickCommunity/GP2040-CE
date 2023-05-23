#ifndef _ButtonLock_H
#define _ButtonLock_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef BUTTON_LOCK_ENABLED
#define BUTTON_LOCK_ENABLED 0
#endif

// ButtonLock Module Name
#define ButtonLockName "ButtonLock"

class ButtonLockAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // ButtonLock Setup
	virtual void process();     // ButtonLock Process
	virtual void preprocess() {}
	virtual std::string name() { return ButtonLockName; }
private:
	uint32_t buttonLockMap;
	uint8_t buttonLockPin;
};

#endif  // _ButtonLock_H_