/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps3/PS3Driver.h"
#include "drivers/ps3/PS3Descriptors.h"
#include "drivers/shared/driverhelper.h"

// Magic byte sequence to enable PS button on PS3
static const uint8_t ps3_magic_init_bytes[8] = {0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

static bool ps3_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
    //uint32_t now = to_ms_since_boot(get_absolute_time());
    //printf("[%d] PS3Driver::ps3_control_xfer_cb Type: %02x, Req: %02x, Val: %x\n", now, request->bmRequestType, request->bRequest, request->wValue);

	if ( request->bmRequestType == 0xA1 &&
		request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
		request->wValue == 0x0300 ) {
		return tud_control_xfer(rhport, request, (void *) ps3_magic_init_bytes, sizeof(ps3_magic_init_bytes));
	} else {
		return hidd_control_xfer_cb(rhport, stage, request);
	}
}

void PS3Driver::initialize() {
    //stdio_init_all();

	ps3Report = {
        .reportID = 1,
        .reserved = 0,
		.cross_btn = 0, .circle_btn = 0, .square_btn = 0, .triangle_btn = 0,
		.l1_btn = 0, .r1_btn = 0, .l2_btn = 0, .r2_btn = 0,
        .select_btn = 0, .start_btn = 0, .l3_btn = 0, .r3_btn = 0, 
        .dpad_up = 0, .dpad_down = 0, .dpad_left = 0, .dpad_right = 0,
        .ps_btn = 0, .tp_btn = 0,
        .l_x_axis = PS3_JOYSTICK_MID,
        .l_y_axis = PS3_JOYSTICK_MID,
        .r_x_axis = PS3_JOYSTICK_MID,
		.r_y_axis = PS3_JOYSTICK_MID,
		.up_axis = 0x00, .right_axis = 0x00, .down_axis = 0x00, .left_axis = 0x00,
		.l2_axis = 0x00, .r2_axis = 0x00, .l1_axis = 0x00, .r1_axis = 0x00,
		.triangle_axis = 0x00, .circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00,
        .reserved2 = {0x00}
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "PS3",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = ps3_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

// Generate PS3 report from gamepad and send to TUSB Device
void PS3Driver::process(Gamepad * gamepad, uint8_t * outBuffer) {
    ps3Report.dpad_left    = gamepad->pressedLeft();
    ps3Report.dpad_down    = gamepad->pressedDown();
    ps3Report.dpad_right   = gamepad->pressedRight();
    ps3Report.dpad_up      = gamepad->pressedUp();

	ps3Report.cross_btn    = gamepad->pressedB1();
	ps3Report.circle_btn   = gamepad->pressedB2();
	ps3Report.square_btn   = gamepad->pressedB3();
	ps3Report.triangle_btn = gamepad->pressedB4();
	ps3Report.l1_btn       = gamepad->pressedL1();
	ps3Report.r1_btn       = gamepad->pressedR1();
	ps3Report.l2_btn       = gamepad->pressedL2();
	ps3Report.r2_btn       = gamepad->pressedR2();
	ps3Report.select_btn   = gamepad->pressedS1();
	ps3Report.start_btn    = gamepad->pressedS2();
	ps3Report.l3_btn       = gamepad->pressedL3();
	ps3Report.r3_btn       = gamepad->pressedR3();
	ps3Report.ps_btn       = gamepad->pressedA1();
	ps3Report.tp_btn       = gamepad->pressedA2();

	ps3Report.l_x_axis = static_cast<uint8_t>(gamepad->state.lx >> 8);
	ps3Report.l_y_axis = static_cast<uint8_t>(gamepad->state.ly >> 8);
	ps3Report.r_x_axis = static_cast<uint8_t>(gamepad->state.rx >> 8);
	ps3Report.r_y_axis = static_cast<uint8_t>(gamepad->state.ry >> 8);

//	if (gamepad->hasAnalogTriggers)
//	{
//		ps3Report.l2_axis = gamepad->state.lt;
//		ps3Report.r2_axis = gamepad->state.rt;
//	} else {
//		ps3Report.l2_axis = gamepad->pressedL2() ? 0xFF : 0;
//		ps3Report.r2_axis = gamepad->pressedR2() ? 0xFF : 0;
//	}
//
//	ps3Report.triangle_axis =	gamepad->pressedB4() ? 0xFF : 0;
//	ps3Report.circle_axis =		gamepad->pressedB2() ? 0xFF : 0;
//	ps3Report.cross_axis =		gamepad->pressedB1() ? 0xFF : 0;
//	ps3Report.square_axis =		gamepad->pressedB3() ? 0xFF : 0;
//	ps3Report.l1_axis =		gamepad->pressedL1() ? 0xFF : 0;
//	ps3Report.r1_axis =		gamepad->pressedR1() ? 0xFF : 0;
//	ps3Report.right_axis =		gamepad->state.dpad & GAMEPAD_MASK_RIGHT ? 0xFF : 0;
//	ps3Report.left_axis =		gamepad->state.dpad & GAMEPAD_MASK_LEFT ? 0xFF : 0;
//	ps3Report.up_axis =		gamepad->state.dpad & GAMEPAD_MASK_UP ? 0xFF : 0;
//	ps3Report.down_axis =		gamepad->state.dpad & GAMEPAD_MASK_DOWN ? 0xFF : 0;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &ps3Report;
	uint16_t report_size = sizeof(ps3Report);
	if (memcmp(last_report, report, report_size) != 0)
	{
		// HID ready + report sent, copy previous report
		if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
			memcpy(last_report, report, report_size);
		}
	}
}

// tud_hid_get_report_cb
uint16_t PS3Driver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    //uint32_t now = to_ms_since_boot(get_absolute_time());
    //printf("[%d] PS3Driver::get_report RPT: %02x, Type: %02x, Size: %d\n", now, report_id, report_type, reqlen);
    //for (uint8_t i = 0; i < reqlen; i++) {
    //    printf("%02x ", buffer[i]);
    //}
    //printf("\n");

    memcpy(buffer, &ps3Report, sizeof(PS3Report));
	return sizeof(PS3Report);
}

// Only PS4 does anything with set report
void PS3Driver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    //uint32_t now = to_ms_since_boot(get_absolute_time());
    //printf("[%d] PS3Driver::set_report RPT: %02x, Type: %02x, Size: %d\n", now, report_id, report_type, bufsize);
}

// Only XboxOG and Xbox One use vendor control xfer cb
bool PS3Driver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * PS3Driver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)ps3_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * PS3Driver::get_descriptor_device_cb() {
    return ps3_device_descriptor;
}

const uint8_t * PS3Driver::get_hid_descriptor_report_cb(uint8_t itf) {
    return ps3_report_descriptor;
}

const uint8_t * PS3Driver::get_descriptor_configuration_cb(uint8_t index) {
    return ps3_configuration_descriptor;
}

const uint8_t * PS3Driver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t PS3Driver::GetJoystickMidValue() {
	return PS3_JOYSTICK_MID << 8;
}
