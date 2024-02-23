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
		uint8_t debounceDpad(uint8_t dpadState, uint8_t changedDpad, uint32_t now);
		uint16_t debounceButtons(uint16_t buttonState, uint16_t changedButtons, uint32_t now);

		GamepadState debounceState;
		uint32_t dpadTime[4];
		uint32_t buttonTime[GAMEPAD_BUTTON_COUNT];
};
