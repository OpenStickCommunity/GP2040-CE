/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_ENUMS_H_
#define GAMEPAD_ENUMS_H_

// The available stick emulation modes
typedef enum
{
	DPAD_MODE_DIGITAL,
	DPAD_MODE_LEFT_ANALOG,
	DPAD_MODE_RIGHT_ANALOG,
} DpadMode;

// The available SOCD cleaning methods
typedef enum
{
	SOCD_MODE_UP_PRIORITY,           // U+D=U, L+R=N
	SOCD_MODE_NEUTRAL,               // U+D=N, L+R=N
	SOCD_MODE_SECOND_INPUT_PRIORITY, // U>D=D, L>R=R (Last Input Priority, aka Last Win)
} SOCDMode;

// Enum for tracking last direction state of Second Input SOCD method
typedef enum
{
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT
} DpadDirection;

// The available hotkey actions
typedef enum
{
	HOTKEY_NONE              = 0x00,
	HOTKEY_DPAD_DIGITAL      = 0x01,
	HOTKEY_DPAD_LEFT_ANALOG  = 0x02,
	HOTKEY_DPAD_RIGHT_ANALOG = 0x04,
	HOTKEY_HOME_BUTTON       = 0x08,
	HOTKEY_CAPTURE_BUTTON    = 0x10,
	HOTKEY_SOCD_UP_PRIORITY  = 0x20,
	HOTKEY_SOCD_NEUTRAL      = 0x40,
	HOTKEY_SOCD_LAST_INPUT   = 0x80,
} GamepadHotkey;

#endif
