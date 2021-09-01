/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "hid_driver.h"
#include "usb_driver.h"

#include "device/usbd_pvt.h"
#include "class/hid/hid_device.h"
#include "hid_interface.h"

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size)
{
	if (tud_hid_ready())
		return tud_hid_report(report_id, report, report_size);

	return false;
}

bool hid_device_control_request(uint8_t rhport, tusb_control_request_t const * request)
{
	if (
		current_input_mode == HID &&
		request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE &&
		request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS &&
		request->bmRequestType_bit.direction == TUSB_DIR_IN &&
		request->wValue == 0x0300
	)
	{
		tud_hid_report(0, magic_init_bytes, sizeof(magic_init_bytes));
	}
	else
	{
		hidd_control_request(rhport, request);
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
