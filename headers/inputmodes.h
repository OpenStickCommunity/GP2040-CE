/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _INPUTMODES_H_
#define _INPUTMODES_H_

#include <stdint.h>

#include "config.pb.h"
#include "enums.pb.h"

// What each InputMode is and supports, fixed for the firmware. The display
// menu and Host Lighting (page 2) read it. Every InputMode needs an
// INPUT_MODE_<mode>_ATTRIBUTES entry below, or the build fails. The driver bit
// follows DriverManager::setup(), and the authentication setting is the
// GamepadOptions field the mode's driver reads. The category, USB host,
// authentication and device type facts are those of the web configurator's
// www/src/Data/InputBootModes.ts.

// Category, as the web configurator lists the mode
#define INPUT_MODE_CATEGORY_PRIMARY 0
#define INPUT_MODE_CATEGORY_MINI    1 // console minis
#define INPUT_MODE_CATEGORY_CONFIG  2 // the config mode, which the web configurator does not list

// Capabilities
#define INPUT_MODE_DRIVER         (1 << 0) // DriverManager::setup() creates a driver
#define INPUT_MODE_PLAYER         (1 << 1) // the host's player number, which page 1 reports with the interface
#define INPUT_MODE_KEYBOARD_CODES (1 << 2) // the driver sends the keyboard mapping's codes

// Lighting interface (Host Lighting)
#define INPUT_MODE_LIGHTING_NEVER   0
#define INPUT_MODE_LIGHTING_ALWAYS  1
#define INPUT_MODE_LIGHTING_SETTING 2 // by the XInput Lighting setting

// USB host peripheral
#define INPUT_MODE_USB_HOST_UNUSED   0
#define INPUT_MODE_USB_HOST_OPTIONAL 1
#define INPUT_MODE_USB_HOST_REQUIRED 2

// Authentication methods (InputModeAuthType) and device types
// (InputModeDeviceType), one bit per value
#define INPUT_MODE_AUTH(type)   (1u << INPUT_MODE_AUTH_TYPE_##type)
#define INPUT_MODE_DEVICE(type) (1u << INPUT_MODE_DEVICE_TYPE_##type)

struct InputModeAttributes {
	InputMode mode;
	const char * name; // as the display menu shows it
	uint8_t category;
	uint8_t capabilities;
	uint8_t lighting;
	uint8_t usbHost;
	uint8_t authTypes;
	InputModeAuthType GamepadOptions::* authSetting; // nullptr for a mode with no setting
	uint16_t deviceTypes;
};

// name, category, capabilities, lighting, USB host, authentication methods,
// authentication setting, device types
#define INPUT_MODE_XINPUT_ATTRIBUTES "XInput", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER | INPUT_MODE_PLAYER, INPUT_MODE_LIGHTING_SETTING, INPUT_MODE_USB_HOST_OPTIONAL, \
	INPUT_MODE_AUTH(NONE) | INPUT_MODE_AUTH(USB), &GamepadOptions::xinputAuthType, \
	INPUT_MODE_DEVICE(GAMEPAD) | INPUT_MODE_DEVICE(WHEEL) | INPUT_MODE_DEVICE(GUITAR) | INPUT_MODE_DEVICE(DRUM)
#define INPUT_MODE_SWITCH_ATTRIBUTES "Nintendo Switch", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_PS3_ATTRIBUTES "PS3", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, \
	INPUT_MODE_DEVICE(GAMEPAD) | INPUT_MODE_DEVICE(GAMEPAD_ALT) | INPUT_MODE_DEVICE(WHEEL) | \
	INPUT_MODE_DEVICE(GUITAR) | INPUT_MODE_DEVICE(DRUM)
#define INPUT_MODE_KEYBOARD_ATTRIBUTES "Keyboard", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER | INPUT_MODE_KEYBOARD_CODES, INPUT_MODE_LIGHTING_ALWAYS, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_PS4_ATTRIBUTES "PS4", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER | INPUT_MODE_PLAYER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_OPTIONAL, \
	INPUT_MODE_AUTH(NONE) | INPUT_MODE_AUTH(KEYS) | INPUT_MODE_AUTH(USB), &GamepadOptions::ps4AuthType, \
	INPUT_MODE_DEVICE(GAMEPAD) | INPUT_MODE_DEVICE(WHEEL) | INPUT_MODE_DEVICE(HOTAS) | \
	INPUT_MODE_DEVICE(GUITAR) | INPUT_MODE_DEVICE(DRUM)
#define INPUT_MODE_XBONE_ATTRIBUTES "Xbox One", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_REQUIRED, 0, nullptr, 0
#define INPUT_MODE_MDMINI_ATTRIBUTES "Sega Genesis Mini", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_NEOGEO_ATTRIBUTES "NEOGEO mini", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_PCEMINI_ATTRIBUTES "PC Engine Mini", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_EGRET_ATTRIBUTES "EGRET II mini", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_ASTRO_ATTRIBUTES "ASTROCITY Mini", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_PSCLASSIC_ATTRIBUTES "Playstation Classic", INPUT_MODE_CATEGORY_MINI, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_XBOXORIGINAL_ATTRIBUTES "Original Xbox", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_PS5_ATTRIBUTES "PS5", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER | INPUT_MODE_PLAYER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_OPTIONAL, \
	INPUT_MODE_AUTH(NONE) | INPUT_MODE_AUTH(USB), &GamepadOptions::ps5AuthType, \
	INPUT_MODE_DEVICE(GAMEPAD) | INPUT_MODE_DEVICE(ARCADE_STICK) | INPUT_MODE_DEVICE(WHEEL) | \
	INPUT_MODE_DEVICE(HOTAS) | INPUT_MODE_DEVICE(GUITAR) | INPUT_MODE_DEVICE(DRUM)
#define INPUT_MODE_GENERIC_ATTRIBUTES "Generic HID", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_ALWAYS, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_SWITCH_PRO_ATTRIBUTES "Nintendo Switch Pro", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_P5GENERAL_ATTRIBUTES "P5 General", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_OPTIONAL, INPUT_MODE_AUTH(USB), nullptr, 0
#define INPUT_MODE_SINPUT_ATTRIBUTES "SInput", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER | INPUT_MODE_PLAYER, INPUT_MODE_LIGHTING_ALWAYS, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_SWITCH2_PRO_ATTRIBUTES "Nintendo Switch 2 Pro", INPUT_MODE_CATEGORY_PRIMARY, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0
#define INPUT_MODE_CONFIG_ATTRIBUTES "Web Config", INPUT_MODE_CATEGORY_CONFIG, \
	INPUT_MODE_DRIVER, INPUT_MODE_LIGHTING_NEVER, INPUT_MODE_USB_HOST_UNUSED, 0, nullptr, 0

#define INPUT_MODE_ATTRIBUTES_ENTRY(name, value) { name, name##_ATTRIBUTES },
inline constexpr InputModeAttributes INPUT_MODE_ATTRIBUTES[] = { InputMode_VALUELIST(INPUT_MODE_ATTRIBUTES_ENTRY) };
#undef INPUT_MODE_ATTRIBUTES_ENTRY

// The mode's attributes; nullptr for a value InputMode does not define
constexpr const InputModeAttributes * inputModeAttributes(InputMode mode) {
	for (const InputModeAttributes & attributes : INPUT_MODE_ATTRIBUTES) {
		if (attributes.mode == mode)
			return &attributes;
	}
	return nullptr;
}

// The mode's display name; "" for a value InputMode does not define
constexpr const char * inputModeName(InputMode mode) {
	const InputModeAttributes * attributes = inputModeAttributes(mode);
	return (attributes != nullptr) ? attributes->name : "";
}

#endif
