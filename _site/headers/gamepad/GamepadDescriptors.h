/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <string.h>
#include "GamepadEnums.h"
#include "descriptors/HIDDescriptors.h"
#include "descriptors/SwitchDescriptors.h"
#include "descriptors/XInputDescriptors.h"
#include "descriptors/KeyboardDescriptors.h"
#include "descriptors/PS4Descriptors.h"

#include "enums.pb.h"

// Default value used for networking, override if necessary
static uint8_t macAddress[6] = { 0x02, 0x02, 0x84, 0x6A, 0x96, 0x00 };

static const uint8_t *getConfigurationDescriptor(uint16_t *size, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_XINPUT:
			*size = sizeof(xinput_configuration_descriptor);
			return xinput_configuration_descriptor;

		case INPUT_MODE_SWITCH:
			*size = sizeof(switch_configuration_descriptor);
			return switch_configuration_descriptor;

		case INPUT_MODE_KEYBOARD:
			*size = sizeof(keyboard_configuration_descriptor);
			return keyboard_configuration_descriptor;

		case INPUT_MODE_PS4:
			*size = sizeof(ps4_configuration_descriptor);
			return ps4_configuration_descriptor;

		default:
			*size = sizeof(hid_configuration_descriptor);
			return hid_configuration_descriptor;
	}
}

static const uint8_t *getDeviceDescriptor(uint16_t *size, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_XINPUT:
			*size = sizeof(xinput_device_descriptor);
			return xinput_device_descriptor;

		case INPUT_MODE_SWITCH:
			*size = sizeof(switch_device_descriptor);
			return switch_device_descriptor;

		case INPUT_MODE_KEYBOARD:
			*size = sizeof(keyboard_device_descriptor);
			return keyboard_device_descriptor;

		case INPUT_MODE_PS4:
			*size = sizeof(ps4_device_descriptor);
			return ps4_device_descriptor;

		default:
			*size = sizeof(hid_device_descriptor);
			return hid_device_descriptor;
	}
}

static const uint8_t *getHIDDescriptor(uint16_t *size, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_SWITCH:
			*size = sizeof(switch_hid_descriptor);
			return switch_hid_descriptor;

		case INPUT_MODE_KEYBOARD:
			*size = sizeof(keyboard_hid_descriptor);
			return keyboard_hid_descriptor;

		case INPUT_MODE_PS4:
			*size = sizeof(ps4_hid_descriptor);
			return ps4_hid_descriptor;

		default:
			*size = sizeof(hid_hid_descriptor);
			return hid_hid_descriptor;
	}
}

static const uint8_t *getHIDReport(uint16_t *size, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_SWITCH:
			*size = sizeof(switch_report_descriptor);
			return switch_report_descriptor;

		case INPUT_MODE_KEYBOARD:
			*size = sizeof(keyboard_report_descriptor);
			return keyboard_report_descriptor;

		case INPUT_MODE_PS4:
			*size = sizeof(ps4_report_descriptor);
			return ps4_report_descriptor;

		default:
			*size = sizeof(hid_report_descriptor);
			return hid_report_descriptor;
	}
}

// Convert ASCII string into UTF-16
static const uint16_t *convertStringDescriptor(uint16_t *payloadSize, const char *str, int charCount)
{
	static uint16_t payload[32];

	// Cap at max char
	if (charCount > 31)
		charCount = 31;

	for (uint8_t i = 0; i < charCount; i++)
		payload[1 + i] = str[i];

	// first byte is length (including header), second byte is string type
	*payloadSize = (2 * charCount + 2);
	payload[0] = (0x03 << 8) | *payloadSize;
	return payload;
}

static const uint16_t *getStringDescriptor(uint16_t *size, InputMode mode, uint8_t index)
{
	uint8_t charCount = 0;
	char *str = 0;

	if (index == 0)
	{
		str = (char *)xinput_string_descriptors[0];
		charCount = 1;
	}
	else if (index == 5)
	{
		// Convert MAC address into UTF-16
		for (int i = 0; i < 6; i++)
		{
			str[1 + charCount++] = "0123456789ABCDEF"[(macAddress[i] >> 4) & 0xf];
			str[1 + charCount++] = "0123456789ABCDEF"[(macAddress[i] >> 0) & 0xf];
		}
	}
	else
	{
		switch (mode)
		{
			case INPUT_MODE_XINPUT:
				str = (char *)xinput_string_descriptors[index];
				break;

			case INPUT_MODE_SWITCH:
				str = (char *)switch_string_descriptors[index];
				break;

			case INPUT_MODE_KEYBOARD:
				str = (char *)keyboard_string_descriptors[index];
				break;

			case INPUT_MODE_PS4:
				str = (char *)ps4_string_descriptors[index];
				break;

			default:
				str = (char *)hid_string_descriptors[index];
				break;
		}

		charCount = strlen(str);
	}

	return convertStringDescriptor(size, str, charCount);
}