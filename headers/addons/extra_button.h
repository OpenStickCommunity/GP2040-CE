#ifndef _ExtraButton_H
#define _ExtraButton_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef EXTRA_BUTTON_ENABLED
#define EXTRA_BUTTON_ENABLED 0
#endif

// ExtraButton Module Name
#define ExtraButtonName "ExtraButton"

class ExtraButtonAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // ExtraButton Setup
	virtual void process() {}     // ExtraButton Process
	virtual void preprocess();
	virtual std::string name() { return ExtraButtonName; }
private:
	uint32_t extraButtonMap;
	uint8_t extraButtonPin;
};

#endif  // _ExtraButton_H_