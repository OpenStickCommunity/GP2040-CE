/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <stdint.h>

#include "tusb_config.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "device/usbd_pvt.h"

#include "gamepad/GamepadDescriptors.h"

#include "usb_driver.h"
#include "net_driver.h"
#include "hid_driver.h"
#include "xinput_driver.h"
#include "ps4_driver.h"

UsbMode usb_mode = USB_MODE_HID;
InputMode input_mode = INPUT_MODE_XINPUT;
static bool usb_mounted = false;
static bool usb_suspended = false;

InputMode get_input_mode(void)
{
	return input_mode;
}

bool get_usb_mounted(void)
{
	return usb_mounted;
}

bool get_usb_suspended(void)
{
	return usb_suspended;
}

void initialize_driver(InputMode mode)
{
	input_mode = mode;
	if (mode == INPUT_MODE_CONFIG)
		usb_mode = USB_MODE_NET;

	tud_init(TUD_OPT_RHPORT);
}

void receive_report(uint8_t *buffer)
{
	if (input_mode == INPUT_MODE_XINPUT)
	{
		receive_xinput_report();
		memcpy(buffer, xinput_out_buffer, XINPUT_OUT_SIZE);
	}
}

bool send_report(void *report, uint16_t report_size)
{
	static uint8_t previous_report[CFG_TUD_ENDPOINT0_SIZE] = { };

	bool sent = false;

	if (tud_suspended())
		tud_remote_wakeup();

	if (memcmp(previous_report, report, report_size) != 0)
	{
		switch (input_mode)
		{
			case INPUT_MODE_XINPUT:
				sent = send_xinput_report(report, report_size);
				break;
			case INPUT_MODE_KEYBOARD:
				sent = send_keyboard_report(report);
				break;

			default:
				sent = send_hid_report(0, report, report_size);
				break;
		}

		if (sent)
			memcpy(previous_report, report, report_size);
	}
	
	return sent;
}

/* USB Driver Callback (Required for XInput) */

const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count)
{
	*driver_count = 1;

	if (usb_mode == USB_MODE_NET)
	{
		return &net_driver;
	}
	else
	{
		switch (input_mode)
		{
			case INPUT_MODE_XINPUT:
				return &xinput_driver;

			case INPUT_MODE_PS4:
				return &ps4_driver;

			default:
				return &hid_driver;
		}
	}
}

/* USB HID Callbacks (Required) */

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
	// TODO: Handle the correct report type, if required
	(void)itf;

	uint8_t report_size = 0;
	SwitchReport switch_report;
	HIDReport hid_report;
	KeyboardReport keyboard_report;
	PS4Report ps4_report;
	switch (input_mode)
	{
		case INPUT_MODE_SWITCH:
			report_size = sizeof(SwitchReport);
			memcpy(buffer, &switch_report, report_size);
			break;
		case INPUT_MODE_KEYBOARD:
			report_size = report_id == KEYBOARD_KEY_REPORT_ID ? sizeof(KeyboardReport::keycode) : sizeof(KeyboardReport::multimedia);
			memcpy(buffer, report_id == KEYBOARD_KEY_REPORT_ID ?
				(void*) keyboard_report.keycode : (void*) &keyboard_report.multimedia, report_size);
			break;
		case INPUT_MODE_PS4:
			if ( report_type == HID_REPORT_TYPE_FEATURE ) {
				// Get feature report (for Auth)
				report_size = get_ps4_report(report_id, buffer, reqlen);
			} else {
				report_size = sizeof(PS4Report);
				memcpy(buffer, &ps4_report, report_size);
			}
			break;
		default:
			report_size = sizeof(HIDReport);
			memcpy(buffer, &hid_report, report_size);
			break;
	}

	return report_size;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
	(void) itf;
	switch (input_mode)
	{
		case INPUT_MODE_PS4:
			if ( report_type == HID_REPORT_TYPE_FEATURE ) {
				set_ps4_report(report_id, buffer, bufsize);
			}
			break;
	}

	// echo back anything we received from host
	tud_hid_report(report_id, buffer, bufsize);
}


/* Device callbacks (Optional) */

// Invoked when device is mounted
void tud_mount_cb(void)
{
	usb_mounted = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	usb_mounted = false;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	usb_suspended = true;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
	usb_suspended = false;
}
