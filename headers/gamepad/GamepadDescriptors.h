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
#include "descriptors/NeogeoDescriptors.h"
#include "descriptors/MDMiniDescriptors.h"
#include "descriptors/PCEngineDescriptors.h"
#include "descriptors/EgretDescriptors.h"
#include "descriptors/AstroDescriptors.h"
#include "descriptors/PSClassicDescriptors.h"
#include "descriptors/XboxOriginalDescriptors.h"

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

		case INPUT_MODE_NEOGEO:
			*size = sizeof(neogeo_configuration_descriptor);
			return neogeo_configuration_descriptor;

		case INPUT_MODE_MDMINI:
			*size = sizeof(mdmini_configuration_descriptor);
			return mdmini_configuration_descriptor;

		case INPUT_MODE_PCEMINI:
			*size = sizeof(pcengine_configuration_descriptor);
			return pcengine_configuration_descriptor;

		case INPUT_MODE_EGRET:
			*size = sizeof(egret_configuration_descriptor);
			return egret_configuration_descriptor;

		case INPUT_MODE_ASTRO:
			*size = sizeof(astro_configuration_descriptor);
			return astro_configuration_descriptor;

		case INPUT_MODE_PSCLASSIC:
			*size = sizeof(psclassic_configuration_descriptor);
			return psclassic_configuration_descriptor;

		case INPUT_MODE_XBOXORIGINAL:
			*size = sizeof(xboxoriginal_configuration_descriptor);
			return xboxoriginal_configuration_descriptor;

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

		case INPUT_MODE_NEOGEO:
			*size = sizeof(neogeo_device_descriptor);
			return neogeo_device_descriptor;

		case INPUT_MODE_MDMINI:
			*size = sizeof(mdmini_device_descriptor);
			return mdmini_device_descriptor;

		case INPUT_MODE_PCEMINI:
			*size = sizeof(pcengine_device_descriptor);
			return pcengine_device_descriptor;

		case INPUT_MODE_EGRET:
			*size = sizeof(egret_device_descriptor);
			return egret_device_descriptor;

		case INPUT_MODE_ASTRO:
			*size = sizeof(astro_device_descriptor);
			return astro_device_descriptor;

		case INPUT_MODE_PSCLASSIC:
			*size = sizeof(psclassic_device_descriptor);
			return psclassic_device_descriptor;

		case INPUT_MODE_XBOXORIGINAL:
			*size = sizeof(xboxoriginal_device_descriptor);
			return xboxoriginal_device_descriptor;

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

		case INPUT_MODE_NEOGEO:
			*size = sizeof(neogeo_hid_descriptor);
			return neogeo_hid_descriptor;

		case INPUT_MODE_MDMINI:
			*size = sizeof(mdmini_hid_descriptor);
			return mdmini_hid_descriptor;

		case INPUT_MODE_PCEMINI:
			*size = sizeof(pcengine_hid_descriptor);
			return pcengine_hid_descriptor;

		case INPUT_MODE_EGRET:
			*size = sizeof(egret_hid_descriptor);
			return egret_hid_descriptor;

		case INPUT_MODE_ASTRO:
			*size = sizeof(astro_hid_descriptor);
			return astro_hid_descriptor;

		case INPUT_MODE_PSCLASSIC:
			*size = sizeof(psclassic_hid_descriptor);
			return psclassic_hid_descriptor;

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

		case INPUT_MODE_NEOGEO:
			*size = sizeof(neogeo_report_descriptor);
			return neogeo_report_descriptor;

		case INPUT_MODE_MDMINI:
			*size = sizeof(mdmini_report_descriptor);
			return mdmini_report_descriptor;

		case INPUT_MODE_PCEMINI:
			*size = sizeof(pcengine_report_descriptor);
			return pcengine_report_descriptor;

		case INPUT_MODE_EGRET:
			*size = sizeof(egret_report_descriptor);
			return egret_report_descriptor;

		case INPUT_MODE_ASTRO:
			*size = sizeof(astro_report_descriptor);
			return astro_report_descriptor;

		case INPUT_MODE_PSCLASSIC:
			*size = sizeof(psclassic_report_descriptor);
			return psclassic_report_descriptor;

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

			case INPUT_MODE_NEOGEO:
				str = (char *)neogeo_string_descriptors[index];
				break;

			case INPUT_MODE_MDMINI:
				str = (char *)mdmini_string_descriptors[index];
				break;

			case INPUT_MODE_PCEMINI:
				str = (char *)pcengine_string_descriptors[index];
				break;

			case INPUT_MODE_EGRET:
				str = (char *)egret_string_descriptors[index];
				break;

			case INPUT_MODE_ASTRO:
				str = (char *)astro_string_descriptors[index];
				break;

			case INPUT_MODE_PSCLASSIC:
				str = (char *)psclassic_string_descriptors[index];
				break;

			case INPUT_MODE_XBOXORIGINAL:
				str = (char *)xboxoriginal_string_descriptors[index];
				break;

			default:
				str = (char *)hid_string_descriptors[index];
				break;
		}

		charCount = strlen(str);
	}

	return convertStringDescriptor(size, str, charCount);
}