/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 * SPDX-FileCopyrightText: Copyright (c) 2019 Ha Thach (tinyusb.org)
 */

#include <wchar.h>
#include "tusb.h"
#include "usb_driver.h"
#include "gamepad/GamepadDescriptors.h"
#include "webserver_descriptors.h"

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	(void)langid;

	if (get_input_mode() == INPUT_MODE_CONFIG)
	{
		return web_tud_descriptor_string_cb(index, langid);
	}
	else
	{
		uint16_t size = 0;
		return getStringDescriptor(&size, get_input_mode(), index);
	}
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb()
{
	switch (get_input_mode())
	{
		case INPUT_MODE_CONFIG:
			return  web_tud_descriptor_device_cb();

		case INPUT_MODE_XINPUT:
			return xinput_device_descriptor;

		case INPUT_MODE_XBONE:
			return xbone_device_descriptor;

		case INPUT_MODE_PS4:
			return ps4_device_descriptor;

		case INPUT_MODE_SWITCH:
			return switch_device_descriptor;

		case INPUT_MODE_KEYBOARD:
			return keyboard_device_descriptor;

		case INPUT_MODE_NEOGEO:
			return neogeo_device_descriptor;

		case INPUT_MODE_MDMINI:
			return mdmini_device_descriptor;

		case INPUT_MODE_PCEMINI:
			return pcengine_device_descriptor;

		case INPUT_MODE_EGRET:
			return egret_device_descriptor;

		case INPUT_MODE_ASTRO:
			return astro_device_descriptor;

		case INPUT_MODE_PSCLASSIC:
			return psclassic_device_descriptor;

		case INPUT_MODE_XBOXORIGINAL:
			return xboxoriginal_device_descriptor;

		default:
			return hid_device_descriptor;
	}
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
	(void) itf;
	switch (get_input_mode())
	{
		case INPUT_MODE_SWITCH:
			return switch_report_descriptor;

		case INPUT_MODE_PS4:
			return ps4_report_descriptor;

		case INPUT_MODE_KEYBOARD:
			return keyboard_report_descriptor;

		case INPUT_MODE_NEOGEO:
			return neogeo_report_descriptor;

		case INPUT_MODE_MDMINI:
			return mdmini_report_descriptor;

		case INPUT_MODE_PCEMINI:
			return pcengine_report_descriptor;

		case INPUT_MODE_EGRET:
			return egret_report_descriptor;

		case INPUT_MODE_ASTRO:
			return astro_report_descriptor;

		case INPUT_MODE_PSCLASSIC:
			return psclassic_report_descriptor;

		default:
			return hid_report_descriptor;
	}
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
	switch (get_input_mode())
	{
		case INPUT_MODE_CONFIG:
			return web_tud_descriptor_configuration_cb(index);

		case INPUT_MODE_XINPUT:
			return xinput_configuration_descriptor;

		case INPUT_MODE_XBONE:
			return xbone_configuration_descriptor_cb(index);

		case INPUT_MODE_PS4:
			return ps4_configuration_descriptor;

		case INPUT_MODE_SWITCH:
			return switch_configuration_descriptor;

		case INPUT_MODE_KEYBOARD:
			return keyboard_configuration_descriptor;

		case INPUT_MODE_NEOGEO:
			return neogeo_configuration_descriptor;

		case INPUT_MODE_MDMINI:
			return mdmini_configuration_descriptor;

		case INPUT_MODE_PCEMINI:
			return pcengine_configuration_descriptor;

		case INPUT_MODE_EGRET:
			return egret_configuration_descriptor;

		case INPUT_MODE_ASTRO:
			return astro_configuration_descriptor;

		case INPUT_MODE_PSCLASSIC:
			return psclassic_configuration_descriptor;

		case INPUT_MODE_XBOXORIGINAL:
			return xboxoriginal_configuration_descriptor;

		default:
			return hid_configuration_descriptor;
	}
}

uint8_t const* tud_descriptor_device_qualifier_cb(void) {
	switch (get_input_mode())
	{
		case INPUT_MODE_XBONE:
			return xbone_device_qualifier;
		default:
			return nullptr;
	}
}