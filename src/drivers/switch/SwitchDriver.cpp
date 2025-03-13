#include "drivers/switch/SwitchDriver.h"
#include "drivers/shared/driverhelper.h"

void SwitchDriver::initialize() {
	switchReport = {
		.buttons = 0,
		.hat = SWITCH_HAT_NOTHING,
		.lx = SWITCH_JOYSTICK_MID,
		.ly = SWITCH_JOYSTICK_MID,
		.rx = SWITCH_JOYSTICK_MID,
		.ry = SWITCH_JOYSTICK_MID,
		.vendor = 0,
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "SWITCH",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool SwitchDriver::process(Gamepad * gamepad) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        switchReport.hat = SWITCH_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   switchReport.hat = SWITCH_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     switchReport.hat = SWITCH_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: switchReport.hat = SWITCH_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      switchReport.hat = SWITCH_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  switchReport.hat = SWITCH_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      switchReport.hat = SWITCH_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    switchReport.hat = SWITCH_HAT_UPLEFT;    break;
		default:                                     switchReport.hat = SWITCH_HAT_NOTHING;   break;
	}

	switchReport.buttons = 0
		| (gamepad->pressedB1() ? SWITCH_MASK_B       : 0)
		| (gamepad->pressedB2() ? SWITCH_MASK_A       : 0)
		| (gamepad->pressedB3() ? SWITCH_MASK_Y       : 0)
		| (gamepad->pressedB4() ? SWITCH_MASK_X       : 0)
		| (gamepad->pressedL1() ? SWITCH_MASK_L       : 0)
		| (gamepad->pressedR1() ? SWITCH_MASK_R       : 0)
		| (gamepad->pressedL2() ? SWITCH_MASK_ZL      : 0)
		| (gamepad->pressedR2() ? SWITCH_MASK_ZR      : 0)
		| (gamepad->pressedS1() ? SWITCH_MASK_MINUS   : 0)
		| (gamepad->pressedS2() ? SWITCH_MASK_PLUS    : 0)
		| (gamepad->pressedL3() ? SWITCH_MASK_L3      : 0)
		| (gamepad->pressedR3() ? SWITCH_MASK_R3      : 0)
		| (gamepad->pressedA1() ? SWITCH_MASK_HOME    : 0)
		| (gamepad->pressedA2() ? SWITCH_MASK_CAPTURE : 0)
	;

	switchReport.lx = static_cast<uint8_t>(gamepad->state.lx >> 8);
	switchReport.ly = static_cast<uint8_t>(gamepad->state.ly >> 8);
	switchReport.rx = static_cast<uint8_t>(gamepad->state.rx >> 8);
	switchReport.ry = static_cast<uint8_t>(gamepad->state.ry >> 8);

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &switchReport;
	uint16_t report_size = sizeof(switchReport);
	if (memcmp(last_report, report, report_size) != 0) {
		// HID ready + report sent, copy previous report
		if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
			memcpy(last_report, report, report_size);
			return true;
		}
	}
	return false;
}

// tud_hid_get_report_cb
uint16_t SwitchDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &switchReport, sizeof(SwitchReport));
	return sizeof(SwitchReport);
}

// Only PS4 does anything with set report
void SwitchDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool SwitchDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * SwitchDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)switch_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * SwitchDriver::get_descriptor_device_cb() {
    return switch_device_descriptor;
}

const uint8_t * SwitchDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return switch_report_descriptor;
}

const uint8_t * SwitchDriver::get_descriptor_configuration_cb(uint8_t index) {
    return switch_configuration_descriptor;
}

const uint8_t * SwitchDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t SwitchDriver::GetJoystickMidValue() {
	return SWITCH_JOYSTICK_MID << 8;
}
