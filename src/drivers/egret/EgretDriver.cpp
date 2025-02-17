#include "drivers/egret/EgretDriver.h"
#include "drivers/shared/driverhelper.h"

void EgretDriver::initialize() {
	egretReport = {
		.buttons = 0,
		.lx = EGRET_JOYSTICK_MID,
		.ly = EGRET_JOYSTICK_MID,
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "EGRET",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool EgretDriver::process(Gamepad * gamepad) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_RIGHT:                     egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MID; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN:                      egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_LEFT:                      egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MID; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		default:                                     egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MID; break;
	}

	egretReport.buttons = 0
		| (gamepad->pressedB1() ? EGRET_MASK_A       : 0)
		| (gamepad->pressedB2() ? EGRET_MASK_B       : 0)
		| (gamepad->pressedB3() ? EGRET_MASK_D       : 0)
		| (gamepad->pressedB4() ? EGRET_MASK_E       : 0)
		| (gamepad->pressedR1() ? EGRET_MASK_F       : 0)
		| (gamepad->pressedR2() ? EGRET_MASK_C       : 0)
		| (gamepad->pressedS1() ? EGRET_MASK_CREDIT  : 0)
		| (gamepad->pressedS2() ? EGRET_MASK_START   : 0)
		| (gamepad->pressedA1() ? EGRET_MASK_MENU    : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &egretReport;
	uint16_t report_size = sizeof(egretReport);
	if (memcmp(last_report, report, report_size) != 0)
	{
		// HID ready + report sent, copy previous report
		if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
			memcpy(last_report, report, report_size);
			return true;
		}
	}
	return false;
}

// tud_hid_get_report_cb
uint16_t EgretDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &egretReport, sizeof(EgretReport));
	return sizeof(EgretReport);
}

// Only PS4 does anything with set report
void EgretDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool EgretDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * EgretDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)egret_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * EgretDriver::get_descriptor_device_cb() {
    return egret_device_descriptor;
}

const uint8_t * EgretDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return egret_report_descriptor;
}

const uint8_t * EgretDriver::get_descriptor_configuration_cb(uint8_t index) {
    return egret_configuration_descriptor;
}

const uint8_t * EgretDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t EgretDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
