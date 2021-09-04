/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_DESCRIPTORS_H_
#define GAMEPAD_DESCRIPTORS_H_

#include "HIDDescriptors.h"
#include "SwitchDescriptors.h"
#include "XInputDescriptors.h"

// The available input modes
typedef enum
{
	INPUT_MODE_XINPUT,
	INPUT_MODE_SWITCH,
	INPUT_MODE_HID,
} InputMode;

#endif
