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
#include "descriptors/XBOneDescriptors.h"

#include "enums.pb.h"

// Default value used for networking, override if necessary
static uint8_t macAddress[6] = { 0x02, 0x02, 0x84, 0x6A, 0x96, 0x00 };

static const uint8_t *getHIDDescriptor(uint16_t *size, InputMode mode);
static const uint8_t *getHIDReport(uint16_t *size, InputMode mode);

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

	if (index == 5)
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

			case INPUT_MODE_XBONE:
				str = (char*)xbone_get_string_descriptor(index);
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
		if ( index == 0 ) // language always has a character count of 1
			charCount = 1;
		else
			charCount = strlen(str);
	}

	return convertStringDescriptor(size, str, charCount);
}