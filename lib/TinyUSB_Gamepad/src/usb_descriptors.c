/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 * SPDX-FileCopyrightText: Copyright (c) 2019 Ha Thach (tinyusb.org)
 */

#include "tusb.h"
#include "usb_driver.h"
#include "HIDDescriptors.h"
#include "SwitchDescriptors.h"
#include "XInputDescriptors.h"

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	(void)langid;

	uint8_t chr_count;

	static const char *string_descriptors[4];
	switch (get_input_mode())
	{
		case INPUT_MODE_XINPUT:
			for (int i = 0; i < 4; i++)
				string_descriptors[i] = xinput_string_descriptors[i];
			break;

		case INPUT_MODE_SWITCH:
			for (int i = 0; i < 4; i++)
				string_descriptors[i] = switch_string_descriptors[i];
			break;

		default:
			for (int i = 0; i < 4; i++)
				string_descriptors[i] = hid_string_descriptors[i];
			break;
	}

	if (index == 0)
	{
		memcpy(&_desc_str[1], string_descriptors[0], 2);
		chr_count = 1;
	}
	else
	{
		// Convert ASCII string into UTF-16
		if (!(index < sizeof(string_descriptors) / sizeof(string_descriptors[0])))
			return NULL;

		const char *str = string_descriptors[index];

		// Cap at max char
		chr_count = strlen(str);
		if (chr_count > 31)
			chr_count = 31;

		for (uint8_t i = 0; i < chr_count; i++)
			_desc_str[1 + i] = str[i];
	}

	// first byte is length (including header), second byte is string type
	_desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
	return _desc_str;
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
	switch (get_input_mode())
	{
		case INPUT_MODE_XINPUT:
			return (const uint8_t *)xinput_device_descriptor;

		case INPUT_MODE_SWITCH:
			return (const uint8_t *)switch_device_descriptor;

		default:
			return (const uint8_t *)hid_device_descriptor;
	}
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(void)
{
	switch (get_input_mode())
	{
		case INPUT_MODE_SWITCH:
			return switch_report_descriptor;

		default:
			return hid_report_descriptor;
	}
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
	(void)index; // for multiple configurations
	switch (get_input_mode())
	{
		case INPUT_MODE_XINPUT:
			return xinput_configuration_descriptor;

		case INPUT_MODE_SWITCH:
			return switch_configuration_descriptor;

		default:
			return hid_configuration_descriptor;
	}
}
