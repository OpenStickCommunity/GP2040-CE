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

		state->dpad = debounceDpad(debounceState.dpad, changedDpad, now, debounceMS);
	}

	else if (debounceState.buttons != state->buttons) {
		uint32_t changedButtons = debounceState.buttons ^ state->buttons;
		
		state->buttons = debounceButtons(debounceState.buttons, changedButtons, now, debounceMS);
	}
}

uint8_t GamepadDebouncer::debounceDpad(uint8_t dpadState, uint32_t changedDpad, uint32_t now, uint8_t delayMS)
{
	for (int i = 0; i < 4; i++)
	{
		// If Dpad is a press, don't wait
		if ((changedDpad & dpadMasks[i]) && !(dpadState & dpadMasks[i]))
		{
			dpadState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
		// If Dpad is a release, wait for the debounce timer
		else if ((changedDpad & dpadMasks[i]) && ((now - dpadTime[i]) > delayMS))
		{
			dpadState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}

	return dpadState;
}

uint16_t GamepadDebouncer::debounceButtons(uint16_t buttonState, uint32_t changedButtons, uint32_t now, uint8_t delayMS)
{
	for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++)
	{
		// If button is a press, don't wait
		if ((changedButtons & buttonMasks[i]) && !(buttonState & buttonMasks[i]))
		{
			buttonState ^= buttonMasks[i];
			buttonTime[i] = now;
		}
		// If button is a release, wait for the debounce timer
		else if ((changedButtons & buttonMasks[i]) && ((now - buttonTime[i]) > delayMS))
		{
			buttonState ^= buttonMasks[i];
			buttonTime[i] = now;
		}
	}

	return buttonState;
}