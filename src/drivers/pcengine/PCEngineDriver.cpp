#include "drivers/pcengine/PCEngineDriver.h"
#include "drivers/shared/driverhelper.h"

void PCEngineDriver::initialize() {
	pcengineReport = {
		.buttons = 0,
		.hat = 0xf,
		.const0 = 0x80,
		.const1 = 0x80,
		.const2 = 0x80,
		.const3 = 0x80,
		.const4 = 0,	
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "PCENGINE",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool PCEngineDriver::process(Gamepad * gamepad) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        pcengineReport.hat = PCENGINE_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   pcengineReport.hat = PCENGINE_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     pcengineReport.hat = PCENGINE_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: pcengineReport.hat = PCENGINE_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      pcengineReport.hat = PCENGINE_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  pcengineReport.hat = PCENGINE_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      pcengineReport.hat = PCENGINE_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    pcengineReport.hat = PCENGINE_HAT_UPLEFT;    break;
		default:                                     pcengineReport.hat = PCENGINE_HAT_NOTHING;   break;
	}

	pcengineReport.buttons = 0x0
		| (gamepad->pressedB1() ? PCENGINE_MASK_1       : 0)
		| (gamepad->pressedB2() ? PCENGINE_MASK_2       : 0)
		| (gamepad->pressedS1() ? PCENGINE_MASK_SELECT  : 0)
		| (gamepad->pressedS2() ? PCENGINE_MASK_RUN     : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &pcengineReport;
	uint16_t report_size = sizeof(pcengineReport);
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
uint16_t PCEngineDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &pcengineReport, sizeof(PCEngineReport));
	return sizeof(PCEngineReport);
}

// Only PS4 does anything with set report
void PCEngineDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool PCEngineDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * PCEngineDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)pcengine_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PCEngineDriver::get_descriptor_device_cb() {
    return pcengine_device_descriptor;
}

const uint8_t * PCEngineDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return pcengine_report_descriptor;
}

const uint8_t * PCEngineDriver::get_descriptor_configuration_cb(uint8_t index) {
    return pcengine_configuration_descriptor;
}

const uint8_t * PCEngineDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t PCEngineDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
