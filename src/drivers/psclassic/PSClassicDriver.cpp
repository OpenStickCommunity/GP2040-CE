#include "drivers/psclassic/PSClassicDriver.h"
#include "drivers/shared/driverhelper.h"

void PSClassicDriver::initialize() {
	psClassicReport = {
		.buttons = 0x0014
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "PSCLASSIC",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool PSClassicDriver::process(Gamepad * gamepad) {
	psClassicReport.buttons = PSCLASSIC_MASK_CENTER;

	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        psClassicReport.buttons = PSCLASSIC_MASK_UP; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   psClassicReport.buttons = PSCLASSIC_MASK_UP_RIGHT; break;
		case GAMEPAD_MASK_RIGHT:                     psClassicReport.buttons = PSCLASSIC_MASK_RIGHT; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: psClassicReport.buttons = PSCLASSIC_MASK_DOWN_RIGHT; break;
		case GAMEPAD_MASK_DOWN:                      psClassicReport.buttons = PSCLASSIC_MASK_DOWN; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  psClassicReport.buttons = PSCLASSIC_MASK_DOWN_LEFT; break;
		case GAMEPAD_MASK_LEFT:                      psClassicReport.buttons = PSCLASSIC_MASK_LEFT; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    psClassicReport.buttons = PSCLASSIC_MASK_UP_LEFT; break;
		default:                                     psClassicReport.buttons = PSCLASSIC_MASK_CENTER; break;
	}

	psClassicReport.buttons |=
          (gamepad->pressedS2()    ? PSCLASSIC_MASK_SELECT   : 0)
		| (gamepad->pressedS1()    ? PSCLASSIC_MASK_START    : 0)
		| (gamepad->pressedB1()    ? PSCLASSIC_MASK_CROSS    : 0)
		| (gamepad->pressedB2()    ? PSCLASSIC_MASK_CIRCLE   : 0)
		| (gamepad->pressedB3()    ? PSCLASSIC_MASK_SQUARE   : 0)
		| (gamepad->pressedB4()    ? PSCLASSIC_MASK_TRIANGLE : 0)
		| (gamepad->pressedL1()    ? PSCLASSIC_MASK_L1       : 0)
		| (gamepad->pressedR1()    ? PSCLASSIC_MASK_R1       : 0)
		| (gamepad->pressedL2()    ? PSCLASSIC_MASK_L2       : 0)
		| (gamepad->pressedR2()    ? PSCLASSIC_MASK_R2       : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &psClassicReport;
	uint16_t report_size = sizeof(psClassicReport);
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
uint16_t PSClassicDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &psClassicReport, sizeof(PSClassicReport));
	return sizeof(PSClassicReport);
}

// Only PS4 does anything with set report
void PSClassicDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool PSClassicDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * PSClassicDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)psclassic_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PSClassicDriver::get_descriptor_device_cb() {
    return psclassic_device_descriptor;
}

const uint8_t * PSClassicDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return psclassic_report_descriptor;
}

const uint8_t * PSClassicDriver::get_descriptor_configuration_cb(uint8_t index) {
    return psclassic_configuration_descriptor;
}

const uint8_t * PSClassicDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t PSClassicDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
