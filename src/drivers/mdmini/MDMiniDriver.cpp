#include "drivers/mdmini/MDMiniDriver.h"
#include "drivers/shared/driverhelper.h"

void MDMiniDriver::initialize() {
	mdminiReport = {
		.id = 0x01,
		.notuse1 = 0x7f,
		.notuse2 = 0x7f,
		.lx = 0x7f,
		.ly = 0x7f,
		.buttons = 0x0f,
		.notuse3 = 0x00,
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "MDMINI",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool MDMiniDriver::process(Gamepad * gamepad) {
	mdminiReport.lx = 0x7f;
	mdminiReport.ly = 0x7f;

	if (gamepad->pressedLeft()) { mdminiReport.lx = MDMINI_MASK_LEFT; }
	if (gamepad->pressedRight()) { mdminiReport.lx = MDMINI_MASK_RIGHT; }
	if (gamepad->pressedUp()) { mdminiReport.ly = MDMINI_MASK_UP; }
	if (gamepad->pressedDown()) { mdminiReport.ly = MDMINI_MASK_DOWN; }

	mdminiReport.buttons = 0x0F
		| (gamepad->pressedB1()    ? MDMINI_MASK_A     : 0)
		| (gamepad->pressedB2()    ? MDMINI_MASK_B     : 0)
		| (gamepad->pressedB3()    ? MDMINI_MASK_X     : 0)
		| (gamepad->pressedB4()    ? MDMINI_MASK_Y     : 0)
		| (gamepad->pressedR1()    ? MDMINI_MASK_Z     : 0)
		| (gamepad->pressedR2()    ? MDMINI_MASK_C     : 0)
		| (gamepad->pressedS2()    ? MDMINI_MASK_START : 0)
		| (gamepad->pressedS1()    ? MDMINI_MASK_MODE  : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &mdminiReport;
	uint16_t report_size = sizeof(mdminiReport);
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
uint16_t MDMiniDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &mdminiReport, sizeof(MDMiniReport));
	return sizeof(MDMiniReport);
}

// Only PS4 does anything with set report
void MDMiniDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool MDMiniDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * MDMiniDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)mdmini_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * MDMiniDriver::get_descriptor_device_cb() {
    return mdmini_device_descriptor;
}

const uint8_t * MDMiniDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return mdmini_report_descriptor;
}

const uint8_t * MDMiniDriver::get_descriptor_configuration_cb(uint8_t index) {
    return mdmini_configuration_descriptor;
}

const uint8_t * MDMiniDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t MDMiniDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
