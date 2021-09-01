/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "hid_driver.h"
#include "usb_driver.h"

#include "device/usbd_pvt.h"
#include "class/hid/hid_device.h"

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size)
{
	if (tud_hid_ready())
		return tud_hid_report(report_id, report, report_size);

	return false;
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
