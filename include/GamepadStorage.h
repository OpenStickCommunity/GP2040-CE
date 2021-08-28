/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_STORAGE_H_
#define GAMEPAD_STORAGE_H_

#include "FlashPROM.h"
#include "Gamepad.h"
#include "usb_driver.h"

#define STORAGE_INPUT_MODE_INDEX 0 // 1 byte
#define STORAGE_DPAD_MODE_INDEX  1 // 1 byte
#define STORAGE_SOCD_MODE_INDEX  2 // 1 byte

class GamepadStorage
{
	public:
		inline void start() { EEPROM.start();  }
		inline void save()  { EEPROM.commit(); }

		DpadMode getDpadMode();
		void setDpadMode(DpadMode mode);

		InputMode getInputMode();
		void setInputMode(InputMode mode);

		SOCDMode getSOCDMode();
		void setSOCDMode(SOCDMode mode);
	
	protected:
		// Wrapper for "get" call to storage API
		template <typename T>
		T &get(int index, T &value)
		{
			return EEPROM.get(index, value);
		}

		// Wrapper for "set" call to storage API
		template <typename T>
		void set(int index, const T &value)
		{
			EEPROM.set(index, value);
		}
};

static GamepadStorage Storage;

#endif
