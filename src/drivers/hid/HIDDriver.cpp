/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/hid/HIDDriver.h"

// Magic byte sequence to enable PS button on PS3
static const uint8_t magic_init_bytes[8] = {0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

virtual void HIDDriver::initialize() {
}

virtual void HIDDriver::update() {
    // Nothing
}

static HIDReport hidReport
{
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

virtual bool HIDDriver::send_report(Gamepad * gamepad) {
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

	return &hidReport;

    if (tud_hid_ready()) {
        if ( inputMode == INPUT_MODE_KEYBOARD ) {
            KeyboardReport *keyboard_report = ((KeyboardReport *)report);
            void *keyboard_report_payload = keyboard_report->reportId == KEYBOARD_KEY_REPORT_ID ? (void *)keyboard_report->keycode : (void *)&keyboard_report->multimedia;
            uint16_t keyboard_report_size = keyboard_report->reportId == KEYBOARD_KEY_REPORT_ID ? sizeof(KeyboardReport::keycode) : sizeof(KeyboardReport::multimedia);
            return tud_hid_report(keyboard_report->reportId, keyboard_report_payload, keyboard_report_size);
        } else {
            return tud_hid_report(report_id, report, report_size);
        }
    }
    return false;
}

virtual void HIDDriver::receive_report(int8_t *buffer) {
    buffer;
}

virtual void HIDDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    report_id;
    report_type;
    buffer;
    reqlen;

    const Gamepad * gamepad = Storage::getInstance().GetGamepad();
    void * report = nullptr;
    switch(inputMode) {
        case INPUT_MODE_ASTRO:
            return getAstroReport();
        case INPUT_MODE_EGRET:
            return getEgretReport();
        case INPUT_MODE_HID:
            return getHIDReport();
        case INPUT_MODE_KEYBOARD:
            return getKeyboardReport();
        case INPUT_MODE_MDMINI:
            return getMDMiniReport();
        case INPUT_MODE_NEOGEO:
            return getNeogeoReport();
        case INPUT_MODE_PCEMINI:
            return getPCEngineReport();
        case INPUT_MODE_PSCLASSIC:
            return getPSClassicReport();
        case INPUT_MODE_SWITCH:
            return getSwitchReport();
        default:
            break;
    }  
}

virtual void HIDDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    report_id;
    report_type;
    buffer;
    bufsize;
}

virtual void HIDDriver::control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    // PS3 magic bytes
    if ( inputMode == INPUT_MODE_HID &&
		request->bmRequestType == 0xA1 &&
		request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
		request->wValue == 0x0300 ) {
		return tud_control_xfer(rhport, request, (void *) magic_init_bytes, sizeof(magic_init_bytes));
	} else { // Standard HID control xfer cb
		return hidd_control_xfer_cb(rhport, stage, request);
	}
}

virtual const uint8_t * HIDDriver::get_device_string(uint8_t index, uint16_t langid) {
    
}

virtual const uint8_t * HIDDriver::et_device_descriptor() {
    
}

virtual const uint8_t * HIDDriver::get_report_descriptor(uint8_t itf) {
    
}

virtual const uint8_t * HIDDriver::get_config_descriptor(uint8_t index) {
    
}

virtual const uint8_t * HIDDriver::get_device_qualifier() {

}

HIDReport * HIDInputDriver::getHIDReport(GamepadState * gamepad)
{
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

	return &hidReport;
}
