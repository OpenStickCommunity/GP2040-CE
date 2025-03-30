#include "drivers/astro/AstroDriver.h"
#include "drivers/shared/driverhelper.h"

void AstroDriver::initialize() {
	astroReport = {
		.id = 1,
		.notuse1 = 0x7f,
		.notuse2 = 0x7f,
		.lx = 0x7f,
		.ly = 0x7f,
		.buttons = 0xf,
		.notuse3 = 0,	
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "ASTRO",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool AstroDriver::process(Gamepad * gamepad) {
	astroReport.lx = 0x7f;
	astroReport.ly = 0x7f;

	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_RIGHT:                     astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MID; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN:                      astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_LEFT:                      astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MID; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		default:                                     astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MID; break;
	}


	astroReport.buttons = 0x0F
		| (gamepad->pressedB1() ? ASTRO_MASK_A       : 0)
		| (gamepad->pressedB2() ? ASTRO_MASK_B       : 0)
		| (gamepad->pressedB3() ? ASTRO_MASK_D       : 0)
		| (gamepad->pressedB4() ? ASTRO_MASK_E       : 0)
		| (gamepad->pressedR1() ? ASTRO_MASK_F       : 0)
		| (gamepad->pressedR2() ? ASTRO_MASK_C       : 0)
		| (gamepad->pressedS1() ? ASTRO_MASK_CREDIT  : 0)
		| (gamepad->pressedS2() ? ASTRO_MASK_START   : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &astroReport;
	uint16_t report_size = sizeof(astroReport);
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
uint16_t AstroDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &astroReport, sizeof(AstroReport));
	return sizeof(AstroReport);
}

// Only PS4 does anything with set report
void AstroDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool AstroDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * AstroDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)astro_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * AstroDriver::get_descriptor_device_cb() {
    return astro_device_descriptor;
}

const uint8_t * AstroDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return astro_report_descriptor;
}

const uint8_t * AstroDriver::get_descriptor_configuration_cb(uint8_t index) {
    return astro_configuration_descriptor;
}

const uint8_t * AstroDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t AstroDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
