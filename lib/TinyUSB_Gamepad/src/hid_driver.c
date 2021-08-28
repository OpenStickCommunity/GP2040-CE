/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "hid_driver.h"

#include "device/usbd_pvt.h"
#include "class/hid/hid_device.h"

bool send_hid_report(void *report, uint8_t report_size)
{
	bool sent = false;
	if (tud_hid_ready())
	{
		tud_hid_report(0, report, report_size);
		sent = true;
	}

	return sent;
}

const usbd_class_driver_t hid_driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "HID",
#endif
	.init = hidd_init,
	.reset = hidd_reset,
	.open = hidd_open,
	.control_request = hidd_control_request,
	.control_complete = hidd_control_complete,
	.xfer_cb = hidd_xfer_cb,
	.sof = NULL
};
