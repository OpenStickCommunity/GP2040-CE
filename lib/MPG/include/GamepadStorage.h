/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_STORAGE_H_
#define GAMEPAD_STORAGE_H_

#include "GamepadEnums.h"
#include "GamepadDescriptors.h"

#define STORAGE_INPUT_MODE_INDEX 0 // 1 byte
#define STORAGE_DPAD_MODE_INDEX  1 // 1 byte
#define STORAGE_SOCD_MODE_INDEX  2 // 1 byte

class GamepadStorage
{
	public:
		GamepadStorage();

		void save();

		DpadMode getDpadMode();
		void setDpadMode(DpadMode mode);

		InputMode getInputMode();
		void setInputMode(InputMode mode);

		SOCDMode getSOCDMode();
		void setSOCDMode(SOCDMode mode);
};

static GamepadStorage Storage;

#endif
