/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "hid_driver.h"
#include "usb_driver.h"

#include "device/usbd_pvt.h"
#include "class/hid/hid_device.h"
#include "GamepadDescriptors.h"

// Magic byte sequence to enable PS button on PS3
static const uint8_t magic_init_bytes[8] = { 0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00 };

bool send_hid_report(uint8_t report_id, uint8_t *report, uint8_t report_size)
{
	if (tud_hid_ready())
		return tud_hid_report(report_id, report, report_size);

	return false;
}

bool hid_device_control_request(uint8_t rhport, tusb_control_request_t const * request)
{
	if (
		get_input_mode() == INPUT_MODE_HID &&
		request->bmRequestType == 0xA1 &&
		request->bRequest == HID_REQ_CONTROL_GET_REPORT &&
		request->wValue == 0x0300
	)
	{
		return tud_hid_report(0, magic_init_bytes, sizeof(magic_init_bytes));
	}
	else
	{
		return hidd_control_request(rhport, request);
	}
}

const usbd_class_driver_t hid_driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "HID",
#endif
	.init = hidd_init,
	.reset = hidd_reset,
	.open = hidd_open,
	.control_request = hid_device_control_request,//hidd_control_request,
	.control_complete = hidd_control_complete,
	.xfer_cb = hidd_xfer_cb,
	.sof = NULL
};
