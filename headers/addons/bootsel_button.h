#ifndef _BootselButton_H
#define _BootselButton_H

#include "gpaddon.h"
#include "gamepad.h"

// BootselButton Module Name
#define BootselButtonName "BootselButton"

class BootselButtonAddon : public GPAddon {
public:
	virtual bool available();   // GPAddon available
	virtual void setup();       // BootselButton Setup
	virtual void process();     // BootselButton Process
	virtual std::string name() { return BootselButtonName; }
private:	
	virtual bool isBootselPressed();
	Gamepad* gamepad;
	uint32_t bootselButtonMap;
};

#endif  // _BootselButton_H_