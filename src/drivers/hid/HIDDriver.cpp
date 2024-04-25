/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/hid/HIDDriver.h"
#include "drivers/hid/HIDDescriptors.h"
#include "drivers/shared/driverhelper.h"

static bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	return hidd_control_xfer_cb(rhport, stage, request);
}

void HIDDriver::initialize() {
	hidReport = {
		.button_01 = 0, .button_02 = 0, .button_03 = 0, .button_04 = 0,
		.button_05 = 0, .button_06 = 0, .button_07 = 0, .button_08 = 0,
		.button_09 = 0, .button_10 = 0, .button_11 = 0, .button_12 = 0,
		.button_13 = 0, .button_14 = 0, .button_15 = 0, .button_16 = 0,
		.direction = HID_HAT_NOTHING,
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "HID",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hid_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

// Generate HID report from gamepad and send to TUSB Device
void HIDDriver::process(Gamepad * gamepad, uint8_t * outBuffer) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        hidReport.direction = HID_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   hidReport.direction = HID_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     hidReport.direction = HID_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: hidReport.direction = HID_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      hidReport.direction = HID_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  hidReport.direction = HID_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      hidReport.direction = HID_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    hidReport.direction = HID_HAT_UPLEFT;    break;
		default:                                     hidReport.direction = HID_HAT_NOTHING;   break;
	}

	// these first three buttons are in this unintuitive order to be compatible with
	// expectations, e.g. both PS3/4/5 modes and Switch modes map to HID as
	// B3 B4  ==  1 4
	// B1 B2  ==  2 3
	hidReport.button_01    = gamepad->pressedB3();
	hidReport.button_02    = gamepad->pressedB1();
	hidReport.button_03    = gamepad->pressedB2();
	hidReport.button_04    = gamepad->pressedB4();
	hidReport.button_05    = gamepad->pressedL1();
	hidReport.button_06    = gamepad->pressedR1();
	hidReport.button_07    = gamepad->pressedL2();
	hidReport.button_08    = gamepad->pressedR2();
	hidReport.button_09    = gamepad->pressedS1();
	hidReport.button_10    = gamepad->pressedS2();
	hidReport.button_11    = gamepad->pressedL3();
	hidReport.button_12    = gamepad->pressedR3();
	hidReport.button_13    = gamepad->pressedA1();
	hidReport.button_14    = gamepad->pressedA2();

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &hidReport;
	uint16_t report_size = sizeof(hidReport);
	if (memcmp(last_report, report, report_size) != 0)
	{
		// HID ready + report sent, copy previous report
		if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
			memcpy(last_report, report, report_size);
		}
	}
}

// tud_hid_get_report_cb
uint16_t HIDDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &hidReport, sizeof(HIDReport));
	return sizeof(HIDReport);
}

// Only PS4 does anything with set report
void HIDDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool HIDDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	return false;
}

const uint16_t * HIDDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)hid_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * HIDDriver::get_descriptor_device_cb() {
	return hid_device_descriptor;
}

const uint8_t * HIDDriver::get_hid_descriptor_report_cb(uint8_t itf) {
	return hid_report_descriptor;
}

const uint8_t * HIDDriver::get_descriptor_configuration_cb(uint8_t index) {
	return hid_configuration_descriptor;
}

const uint8_t * HIDDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t HIDDriver::GetJoystickMidValue() {
	return HID_JOYSTICK_MID << 8;
}
