#include "drivers/neogeo/NeoGeoDriver.h"
#include "drivers/shared/driverhelper.h"

void NeoGeoDriver::initialize() {
	neogeoReport = {
		.buttons = 0,
		.hat = 0xf,
		.const0 = 0x80,
		.const1 = 0x80,
		.const2 = 0x80,
		.const3 = 0x80,
		.const4 = 0,	
		.const5 = 0,	
		.const6 = 0,	
		.const7 = 0,	
		.const8 = 0,	
		.const9 = 0,	
		.const10 = 0,	
		.const11 = 0,	
		.const12 = 0,	
		.const13 = 0,	
		.const14 = 0,	
		.const15 = 0,	
		.const16 = 0,	
		.const17 = 0,	
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "NEOGEO",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool NeoGeoDriver::process(Gamepad * gamepad) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        neogeoReport.hat = NEOGEO_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   neogeoReport.hat = NEOGEO_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     neogeoReport.hat = NEOGEO_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: neogeoReport.hat = NEOGEO_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      neogeoReport.hat = NEOGEO_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  neogeoReport.hat = NEOGEO_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      neogeoReport.hat = NEOGEO_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    neogeoReport.hat = NEOGEO_HAT_UPLEFT;    break;
		default:                                     neogeoReport.hat = NEOGEO_HAT_NOTHING;   break;
	}

	neogeoReport.buttons = 0x0
		| (gamepad->pressedB3() ? NEOGEO_MASK_A       : 0)
		| (gamepad->pressedB1() ? NEOGEO_MASK_B       : 0)
		| (gamepad->pressedB4() ? NEOGEO_MASK_C       : 0)
		| (gamepad->pressedB2() ? NEOGEO_MASK_D       : 0)
		| (gamepad->pressedS1() ? NEOGEO_MASK_SELECT  : 0)
		| (gamepad->pressedS2() ? NEOGEO_MASK_START   : 0)
		| (gamepad->pressedA1() ? NEOGEO_MASK_OPTIONS : 0)
		| (gamepad->pressedL1() ? NEOGEO_MASK_L1      : 0)
		| (gamepad->pressedL2() ? NEOGEO_MASK_L2      : 0)
		| (gamepad->pressedR1() ? NEOGEO_MASK_R1      : 0)
		| (gamepad->pressedR2() ? NEOGEO_MASK_R2      : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &neogeoReport;
	uint16_t report_size = sizeof(neogeoReport);
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
uint16_t NeoGeoDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &neogeoReport, sizeof(NeogeoReport));
	return sizeof(NeogeoReport);
}

// Only PS4 does anything with set report
void NeoGeoDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool NeoGeoDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * NeoGeoDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)neogeo_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * NeoGeoDriver::get_descriptor_device_cb() {
    return neogeo_device_descriptor;
}

const uint8_t * NeoGeoDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return neogeo_report_descriptor;
}

const uint8_t * NeoGeoDriver::get_descriptor_configuration_cb(uint8_t index) {
    return neogeo_configuration_descriptor;
}

const uint8_t * NeoGeoDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t NeoGeoDriver::GetJoystickMidValue() {
	return NEOGEO_JOYSTICK_MID;
}
