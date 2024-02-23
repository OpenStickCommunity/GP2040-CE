/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <string.h>
#include <stdint.h>
#include "GamepadState.h"

// Implement this wrapper function for your platform
// TODO: Make this a pure virtual member instead.
uint32_t getMillis();

class GamepadDebouncer
{
	public:
		GamepadDebouncer() { }

		void debounce(GamepadState *state);
		uint8_t debounceDpad(uint8_t oldDpad, uint8_t newDpad, uint32_t now);
		uint16_t debounceButtons(uint16_t oldButtons, uint16_t newButtons, uint32_t now);

		GamepadState debounceState;
		uint32_t dpadTime[4];
		uint32_t buttonTime[GAMEPAD_BUTTON_COUNT];
};
