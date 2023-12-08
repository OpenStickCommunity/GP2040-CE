/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "hid_driver.h"
#include "usb_driver.h"
#include "device/usbd.h"

#include "device/usbd_pvt.h"
#include "class/hid/hid_device.h"
#include "gamepad/GamepadDescriptors.h"

#include "enums.pb.h"

// Magic byte sequence to enable PS button on PS3
static const uint8_t magic_init_bytes[8] = {0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size)
{
	if (tud_hid_ready())
		return tud_hid_report(report_id, report, report_size);

	return false;
}

bool send_keyboard_report(void *report)
{
	if (tud_hid_ready()) {
		KeyboardReport *keyboard_report = ((KeyboardReport *)report);
		void *keyboard_report_payload = keyboard_report->reportId == KEYBOARD_KEY_REPORT_ID ? (void *)keyboard_report->keycode : (void *)&keyboard_report->multimedia;
		uint16_t keyboard_report_size = keyboard_report->reportId == KEYBOARD_KEY_REPORT_ID ? sizeof(KeyboardReport::keycode) : sizeof(KeyboardReport::multimedia);

		return tud_hid_report(keyboard_report->reportId, keyboard_report_payload, keyboard_report_size);
	}

	return false;
}

bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	if (
		get_input_mode() == INPUT_MODE_HID &&
		request->bmRequestType == 0xA1 &&
		request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
		request->wValue == 0x0300
	)
	{
		return tud_control_xfer(rhport, request, (void *) magic_init_bytes, sizeof(magic_init_bytes));
	}
	else
	{
		return hidd_control_xfer_cb(rhport, stage, request);
	}
}

const usbd_class_driver_t hid_driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "HID",
#endif
	.init = hidd_init,
	.reset = hidd_reset,
	.open = hidd_open,
	.control_xfer_cb = hid_control_xfer_cb,
	.xfer_cb = hidd_xfer_cb,
	.sof = NULL};