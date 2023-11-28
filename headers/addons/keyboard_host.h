#ifndef _KeyboardHost_H
#define _KeyboardHost_H

#include "usbaddon.h"
#include "gamepad.h"

#ifndef KEYBOARD_HOST_ENABLED
#define KEYBOARD_HOST_ENABLED 0
#endif

#ifndef KEYBOARD_HOST_PIN_DPLUS
#define KEYBOARD_HOST_PIN_DPLUS -1
#endif

#ifndef KEYBOARD_HOST_PIN_5V
#define KEYBOARD_HOST_PIN_5V -1
#endif

// KeyboardHost Module Name
#define KeyboardHostName "KeyboardHost"

struct KeyboardButtonMapping
{
	uint8_t key;
	uint16_t buttonMask;

	inline void setMask(uint16_t m) {
		buttonMask = m;
	}

	inline void setKey(uint8_t p)
	{
		key = (p > HID_KEY_NONE && p <=HID_KEY_GUI_RIGHT) ? p : 0xff;
	}

	bool isAssigned() const { return key != 0xff; }
};

class KeyboardHostAddon : public USBAddon {
public:
	virtual bool available();
	virtual void setup();       // KeyboardHost Setup
	virtual void process() {}   // KeyboardHost Process
	virtual void preprocess();
	virtual std::string name() { return KeyboardHostName; }
// USB Add-on Features
	virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {}
	virtual void unmount(uint8_t dev_addr);
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
	bool _keyboard_host_enabled;
	uint8_t getKeycodeFromModifier(uint8_t modifier);
	void process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report);
	GamepadState _keyboard_host_state;
	KeyboardButtonMapping _keyboard_host_mapDpadUp;
	KeyboardButtonMapping _keyboard_host_mapDpadDown;
	KeyboardButtonMapping _keyboard_host_mapDpadLeft;
	KeyboardButtonMapping _keyboard_host_mapDpadRight;
	KeyboardButtonMapping _keyboard_host_mapButtonB1;
	KeyboardButtonMapping _keyboard_host_mapButtonB2;
	KeyboardButtonMapping _keyboard_host_mapButtonB3;
	KeyboardButtonMapping _keyboard_host_mapButtonB4;
	KeyboardButtonMapping _keyboard_host_mapButtonL1;
	KeyboardButtonMapping _keyboard_host_mapButtonR1;
	KeyboardButtonMapping _keyboard_host_mapButtonL2;
	KeyboardButtonMapping _keyboard_host_mapButtonR2;
	KeyboardButtonMapping _keyboard_host_mapButtonS1;
	KeyboardButtonMapping _keyboard_host_mapButtonS2;
	KeyboardButtonMapping _keyboard_host_mapButtonL3;
	KeyboardButtonMapping _keyboard_host_mapButtonR3;
	KeyboardButtonMapping _keyboard_host_mapButtonA1;
	KeyboardButtonMapping _keyboard_host_mapButtonA2;
};

#endif  // _KeyboardHost_H_