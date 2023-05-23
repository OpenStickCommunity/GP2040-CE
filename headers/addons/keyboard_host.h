#ifndef _KeyboardHost_H
#define _KeyboardHost_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef KEYBOARD_HOST_ENABLED
#define KEYBOARD_HOST_ENABLED 0
#endif

// KeyboardHost Module Name
#define KeyboardHostName "KeyboardHost"

struct KeyboardButtonMapping
{
	KeyboardButtonMapping(uint8_t p, uint16_t bm) : 
		key(p > HID_KEY_NONE && p <=HID_KEY_GUI_RIGHT ? p : 0xff),
		buttonMask(bm)
	{}

	uint8_t key;
	const uint16_t buttonMask;

	inline void setKey(uint8_t p)
	{
		if (p <= HID_KEY_NONE && p <=HID_KEY_GUI_RIGHT)
		{
			key = p;
		}
		else
		{
			key = 0xff;
		}
	}

	bool isAssigned() const { return key != 0xff; }
};

class KeyboardHostAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // KeyboardHost Setup
	virtual void process() {}     // KeyboardHost Process
	virtual void preprocess();
	virtual std::string name() { return KeyboardHostName; }
private:	
};

#endif  // _KeyboardHost_H_