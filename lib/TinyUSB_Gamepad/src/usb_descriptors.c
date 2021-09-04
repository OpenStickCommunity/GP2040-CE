/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 * SPDX-FileCopyrightText: Copyright (c) 2019 Ha Thach (tinyusb.org)
 */

#include <wchar.h>
#include "tusb.h"
#include "usb_driver.h"
#include "GamepadDescriptors.h"

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	(void)langid;

	static uint16_t descriptorStringBuffer[32];

	uint8_t charCount;
	wchar_t *value;

	switch (get_input_mode())
	{
		case INPUT_MODE_XINPUT:
			value = xinput_string_descriptors[index];
			break;

		case INPUT_MODE_SWITCH:
			value = switch_string_descriptors[index];
			break;

		default:
			value = hid_string_descriptors[index];
			break;
	}

	// Cap at max char
	charCount = wcslen(value);
	if (charCount > 31)
		charCount = 31;

	for (uint8_t i = 0; i < charCount; i++)
		descriptorStringBuffer[1 + i] = value[i];

	// first byte is length (including header), second byte is string type
	descriptorStringBuffer[0] = (TUSB_DESC_STRING << 8) | (2 * charCount + 2);
	return descriptorStringBuffer;
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
