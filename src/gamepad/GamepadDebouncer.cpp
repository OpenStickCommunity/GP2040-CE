/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "gamepad/GamepadDebouncer.h"

void GamepadDebouncer::debounce(GamepadState *state)
{
	// Only run if dpad and button states have changed
	if ((debounceState.dpad == state->dpad) && (debounceState.buttons == state->buttons))
		return;

	uint32_t now = getMillis();

	if (debounceState.dpad != state->dpad) {
		uint32_t changedDpad = debounceState.dpad ^ state->dpad;

		for (int i = 0; i < 4; i++)
		{
			// If Dpad is a press, don't wait
			if ((changedDpad & dpadMasks[i]) && (state->dpad & dpadMasks[i]))
			{
				debounceState.dpad ^= dpadMasks[i];
				dpadTime[i] = now;
			}
			// If Dpad is a release, wait for the debounce timer
			else if ((changedDpad & dpadMasks[i]) && ((now - dpadTime[i]) > debounceMS))
			{
				debounceState.dpad ^= dpadMasks[i];
				dpadTime[i] = now;
			}
		}

		state->dpad = debounceState.dpad;
	}

	else if (debounceState.buttons != state->buttons) {
		uint32_t changedButtons = debounceState.buttons ^ state->buttons;

		for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++)
		{
			// If button is a press, don't wait
			if ((changedButtons & buttonMasks[i]) && (state->buttons & buttonMasks[i]))
			{
				debounceState.buttons ^= buttonMasks[i];
				buttonTime[i] = now;
			}
			// If button is a release, wait for the debounce timer
			else if ((changedButtons & buttonMasks[i]) && ((now - buttonTime[i]) > debounceMS))
			{
				debounceState.buttons ^= buttonMasks[i];
				buttonTime[i] = now;
			}
		}

		state->buttons = debounceState.buttons;
	}
}
