#ifndef _BootselButton_H
#define _BootselButton_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef BOOTSEL_BUTTON_ENABLED
#define BOOTSEL_BUTTON_ENABLED 0
#endif

#ifndef BOOTSEL_BUTTON_MASK
#define BOOTSEL_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h
#endif

// BootselButton Module Name
#define BootselButtonName "BootselButton"

class BootselButtonAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // BootselButton Setup
	virtual void process() {}     // BootselButton Process
	virtual void preprocess();
	virtual std::string name() { return BootselButtonName; }
private:	
	bool isBootselPressed();
	uint32_t bootselButtonMap;
};

#endif  // _BootselButton_H_