#include "drivers/keyboard/KeyboardDriver.h"
#include "storagemanager.h"
#include "drivers/shared/driverhelper.h"
#include "drivers/hid/HIDDescriptors.h"

#include "eventmanager.h"

void KeyboardDriver::initialize() {
	keyboardReport = {
		.keycode = { 0 },
		.multimedia = 0
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "KEYBOARD",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};

    // Handle Volume for Rotary Encoder
    EventManager::getInstance().registerEventHandler(GP_EVENT_ENCODER_CHANGE, GPEVENT_CALLBACK(this->handleEncoder(event)));
    volumeChange = 0; // no change
}

uint8_t KeyboardDriver::getModifier(uint8_t code) {
	switch (code) {
		case HID_KEY_CONTROL_LEFT : return KEYBOARD_MODIFIER_LEFTCTRL  ;
		case HID_KEY_SHIFT_LEFT   : return KEYBOARD_MODIFIER_LEFTSHIFT ;
		case HID_KEY_ALT_LEFT     : return KEYBOARD_MODIFIER_LEFTALT   ;
		case HID_KEY_GUI_LEFT     : return KEYBOARD_MODIFIER_LEFTGUI   ;
		case HID_KEY_CONTROL_RIGHT: return KEYBOARD_MODIFIER_RIGHTCTRL ;
		case HID_KEY_SHIFT_RIGHT  : return KEYBOARD_MODIFIER_RIGHTSHIFT;
		case HID_KEY_ALT_RIGHT    : return KEYBOARD_MODIFIER_RIGHTALT  ;
		case HID_KEY_GUI_RIGHT    : return KEYBOARD_MODIFIER_RIGHTGUI  ;
	}

	return 0;
}

uint8_t KeyboardDriver::getMultimedia(uint8_t code) {
	switch (code) {
		case KEYBOARD_MULTIMEDIA_NEXT_TRACK : return 0x01;
		case KEYBOARD_MULTIMEDIA_PREV_TRACK : return 0x02;
		case KEYBOARD_MULTIMEDIA_STOP 	    : return 0x04;
		case KEYBOARD_MULTIMEDIA_PLAY_PAUSE : return 0x08;
		case KEYBOARD_MULTIMEDIA_MUTE 	    : return 0x10;
		case KEYBOARD_MULTIMEDIA_VOLUME_UP  : return 0x20;
		case KEYBOARD_MULTIMEDIA_VOLUME_DOWN: return 0x40;
	}
	return 0;
}


bool KeyboardDriver::process(Gamepad * gamepad) {
	const KeyboardMapping& keyboardMapping = Storage::getInstance().getKeyboardMapping();
	releaseAllKeys();
	if(gamepad->pressedUp())     { pressKey(keyboardMapping.keyDpadUp); }
	if(gamepad->pressedDown())   { pressKey(keyboardMapping.keyDpadDown); }
	if(gamepad->pressedLeft())	{ pressKey(keyboardMapping.keyDpadLeft); }
	if(gamepad->pressedRight()) 	{ pressKey(keyboardMapping.keyDpadRight); }
	if(gamepad->pressedB1()) 	{ pressKey(keyboardMapping.keyButtonB1); }
	if(gamepad->pressedB2()) 	{ pressKey(keyboardMapping.keyButtonB2); }
	if(gamepad->pressedB3()) 	{ pressKey(keyboardMapping.keyButtonB3); }
	if(gamepad->pressedB4()) 	{ pressKey(keyboardMapping.keyButtonB4); }
	if(gamepad->pressedL1()) 	{ pressKey(keyboardMapping.keyButtonL1); }
	if(gamepad->pressedR1()) 	{ pressKey(keyboardMapping.keyButtonR1); }
	if(gamepad->pressedL2()) 	{ pressKey(keyboardMapping.keyButtonL2); }
	if(gamepad->pressedR2()) 	{ pressKey(keyboardMapping.keyButtonR2); }
	if(gamepad->pressedS1()) 	{ pressKey(keyboardMapping.keyButtonS1); }
	if(gamepad->pressedS2()) 	{ pressKey(keyboardMapping.keyButtonS2); }
	if(gamepad->pressedL3()) 	{ pressKey(keyboardMapping.keyButtonL3); }
	if(gamepad->pressedR3()) 	{ pressKey(keyboardMapping.keyButtonR3); }
	if(gamepad->pressedA1()) 	{ pressKey(keyboardMapping.keyButtonA1); }
	if(gamepad->pressedA2()) 	{ pressKey(keyboardMapping.keyButtonA2); }
	if(gamepad->pressedA3()) 	{ pressKey(keyboardMapping.keyButtonA3); }
	if(gamepad->pressedA4()) 	{ pressKey(keyboardMapping.keyButtonA4); }
	if(gamepad->pressedE1()) 	{ pressKey(keyboardMapping.keyButtonE1); }
	if(gamepad->pressedE2()) 	{ pressKey(keyboardMapping.keyButtonE2); }
	if(gamepad->pressedE3()) 	{ pressKey(keyboardMapping.keyButtonE3); }
	if(gamepad->pressedE4()) 	{ pressKey(keyboardMapping.keyButtonE4); }
	if(gamepad->pressedE5()) 	{ pressKey(keyboardMapping.keyButtonE5); }
	if(gamepad->pressedE6()) 	{ pressKey(keyboardMapping.keyButtonE6); }
	if(gamepad->pressedE7()) 	{ pressKey(keyboardMapping.keyButtonE7); }
	if(gamepad->pressedE8()) 	{ pressKey(keyboardMapping.keyButtonE8); }
	if(gamepad->pressedE9()) 	{ pressKey(keyboardMapping.keyButtonE9); }
	if(gamepad->pressedE10()) 	{ pressKey(keyboardMapping.keyButtonE10); }
	if(gamepad->pressedE11()) 	{ pressKey(keyboardMapping.keyButtonE11); }
	if(gamepad->pressedE12()) 	{ pressKey(keyboardMapping.keyButtonE12); }

    if( volumeChange > 0 ) {
        pressKey(KEYBOARD_MULTIMEDIA_VOLUME_UP);
    } else if ( volumeChange < 0 ) {
        pressKey(KEYBOARD_MULTIMEDIA_VOLUME_DOWN);
    }

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void *keyboard_report_payload;
	uint16_t keyboard_report_size;
	if ( keyboardReport.reportId == KEYBOARD_KEY_REPORT_ID ) {
		keyboard_report_payload = (void *)keyboardReport.keycode;
		keyboard_report_size = sizeof(KeyboardReport::keycode);
		
	} else {
		keyboard_report_payload = (void *)&keyboardReport.multimedia;
		keyboard_report_size = sizeof(KeyboardReport::multimedia);
	}

	// If we had a keycode but now have a multimedia key OR report is different
	if (keyboard_report_size != last_report_size || 
			memcmp(last_report, &keyboardReport, last_report_size) != 0) {
		if (tud_hid_ready()) {
			if ( tud_hid_report(keyboardReport.reportId, keyboard_report_payload, keyboard_report_size) ) {
				memcpy(last_report, keyboard_report_payload, keyboard_report_size);
				last_report_size = keyboard_report_size;

                // Adjust volume on success
                if( volumeChange > 0 ) {
                    volumeChange--;
                } else if ( volumeChange < 0 ) {
                    volumeChange++;
                }
				return true;
			}
		}
	}
	
	return false;
}

void KeyboardDriver::pressKey(uint8_t code) {
	if (code > HID_KEY_GUI_RIGHT) {
		keyboardReport.reportId = KEYBOARD_MULTIMEDIA_REPORT_ID;
		keyboardReport.multimedia = getMultimedia(code);
	}  else {
		keyboardReport.reportId = KEYBOARD_KEY_REPORT_ID;
		keyboardReport.keycode[code / 8] |= 1 << (code % 8);
	}
}

void KeyboardDriver::releaseAllKeys(void) {
	for (uint8_t i = 0; i < (sizeof(keyboardReport.keycode) / sizeof(keyboardReport.keycode[0])); i++) {
		keyboardReport.keycode[i] = 0;
	}
	keyboardReport.multimedia = 0;
}

// tud_hid_get_report_cb
uint16_t KeyboardDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	if ( report_id == KEYBOARD_KEY_REPORT_ID ) {
		memcpy(buffer, (void*) keyboardReport.keycode, sizeof(KeyboardReport::keycode));
		return sizeof(KeyboardReport::keycode);
	} else {
		memcpy(buffer, (void*) &keyboardReport.multimedia, sizeof(KeyboardReport::multimedia));
		return sizeof(KeyboardReport::multimedia);
	}
}

// Only PS4 does anything with set report
void KeyboardDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool KeyboardDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * KeyboardDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)keyboard_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * KeyboardDriver::get_descriptor_device_cb() {
    return keyboard_device_descriptor;
}

const uint8_t * KeyboardDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return keyboard_report_descriptor;
}

const uint8_t * KeyboardDriver::get_descriptor_configuration_cb(uint8_t index) {
    return keyboard_configuration_descriptor;
}

const uint8_t * KeyboardDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t KeyboardDriver::GetJoystickMidValue() {
	return HID_JOYSTICK_MID << 8;
}

void KeyboardDriver::handleEncoder(GPEvent* e) {
    GPEncoderChangeEvent * encoderEvent = (GPEncoderChangeEvent*)e;
    if ( encoderEvent->direction == 1 ) {
        // volume up
        volumeChange++;
    } else if ( encoderEvent->direction == -1 ) {
        // volume down
        volumeChange--;
    }
}
