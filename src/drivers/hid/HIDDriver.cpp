/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/hid/HIDDriver.h"
#include "drivers/hid/HIDDescriptors.h"
#include "drivers/shared/driverhelper.h"

// Magic byte sequence to enable PS button on PS3
static const uint8_t ps3_magic_init_bytes[8] = {0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

static bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	if ( request->bmRequestType == 0xA1 &&
		request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
		request->wValue == 0x0300 ) {
		return tud_control_xfer(rhport, request, (void *) ps3_magic_init_bytes, sizeof(ps3_magic_init_bytes));
	} else {
		return hidd_control_xfer_cb(rhport, stage, request);
	}
}

void HIDDriver::initialize() {
	hidReport = {
		.square_btn = 0, .cross_btn = 0, .circle_btn = 0, .triangle_btn = 0,
		.l1_btn = 0, .r1_btn = 0, .l2_btn = 0, .r2_btn = 0,
		.select_btn = 0, .start_btn = 0, .l3_btn = 0, .r3_btn = 0, .ps_btn = 0, .tp_btn = 0,
		.direction = 0x08,
		.l_x_axis = HID_JOYSTICK_MID,
		.l_y_axis = HID_JOYSTICK_MID,
		.r_x_axis = HID_JOYSTICK_MID,
		.r_y_axis = HID_JOYSTICK_MID,
		.right_axis = 0x00, .left_axis = 0x00, .up_axis = 0x00, .down_axis = 0x00,
		.triangle_axis = 0x00, .circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00,
		.l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00
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

	hidReport.cross_btn    = gamepad->pressedB1();
	hidReport.circle_btn   = gamepad->pressedB2();
	hidReport.square_btn   = gamepad->pressedB3();
	hidReport.triangle_btn = gamepad->pressedB4();
	hidReport.l1_btn       = gamepad->pressedL1();
	hidReport.r1_btn       = gamepad->pressedR1();
	hidReport.l2_btn       = gamepad->pressedL2();
	hidReport.r2_btn       = gamepad->pressedR2();
	hidReport.select_btn   = gamepad->pressedS1();
	hidReport.start_btn    = gamepad->pressedS2();
	hidReport.l3_btn       = gamepad->pressedL3();
	hidReport.r3_btn       = gamepad->pressedR3();
	hidReport.ps_btn       = gamepad->pressedA1();
	hidReport.tp_btn       = gamepad->pressedA2();

	hidReport.l_x_axis = static_cast<uint8_t>(gamepad->state.lx >> 8);
	hidReport.l_y_axis = static_cast<uint8_t>(gamepad->state.ly >> 8);
	hidReport.r_x_axis = static_cast<uint8_t>(gamepad->state.rx >> 8);
	hidReport.r_y_axis = static_cast<uint8_t>(gamepad->state.ry >> 8);

	if (gamepad->hasAnalogTriggers)
	{
		hidReport.l2_axis = gamepad->state.lt;
		hidReport.r2_axis = gamepad->state.rt;
	} else {
		hidReport.l2_axis = gamepad->pressedL2() ? 0xFF : 0;
		hidReport.r2_axis = gamepad->pressedR2() ? 0xFF : 0;
	}

	hidReport.triangle_axis =	gamepad->pressedB4() ? 0xFF : 0;
	hidReport.circle_axis =		gamepad->pressedB2() ? 0xFF : 0;
	hidReport.cross_axis =		gamepad->pressedB1() ? 0xFF : 0;
	hidReport.square_axis =		gamepad->pressedB3() ? 0xFF : 0;
	hidReport.l1_axis =		gamepad->pressedL1() ? 0xFF : 0;
	hidReport.r1_axis =		gamepad->pressedR1() ? 0xFF : 0;
	hidReport.right_axis =		gamepad->state.dpad & GAMEPAD_MASK_RIGHT ? 0xFF : 0;
	hidReport.left_axis =		gamepad->state.dpad & GAMEPAD_MASK_LEFT ? 0xFF : 0;
	hidReport.up_axis =		gamepad->state.dpad & GAMEPAD_MASK_UP ? 0xFF : 0;
	hidReport.down_axis =		gamepad->state.dpad & GAMEPAD_MASK_DOWN ? 0xFF : 0;

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
