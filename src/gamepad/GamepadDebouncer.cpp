/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "gamepad/GamepadDebouncer.h"
#include "storagemanager.h"

void GamepadDebouncer::debounce(GamepadState *state)
{
	/*
	// Return if dpad and button states haven't changed
	if ((debounceState.dpad == state->dpad) && (debounceState.buttons == state->buttons)) {
		return;
	}

	// Debounce the Dpad
	if (debounceState.dpad != state->dpad) {
		uint32_t changedDpad = debounceState.dpad ^ state->dpad;
		state->dpad = debounceDpad(debounceState.dpad, changedDpad);
		debounceState.dpad = state->dpad;
	}

	// Debounce the buttons
	if (debounceState.buttons != state->buttons) {
		uint32_t changedButtons = debounceState.buttons ^ state->buttons;
		state->buttons = debounceButtons(debounceState.buttons, changedButtons);
		debounceState.buttons = state->buttons;
	}*/
	const uint8_t debounceMS = 5;
	uint32_t now = getMillis();

	for (int i = 0; i < 4; i++)
	{
		if ((debounceState.dpad & dpadMasks[i]) != (state->dpad & dpadMasks[i]) && (now - dpadTime[i]) > debounceMS)
		{
			debounceState.dpad ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}

	for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++)
	{
		if ((debounceState.buttons & buttonMasks[i]) != (state->buttons & buttonMasks[i]) && (now - buttonTime[i]) > debounceMS)
		{
			debounceState.buttons ^= buttonMasks[i];
			buttonTime[i] = now;
		}
	}

	state->dpad = debounceState.dpad;
	state->buttons = debounceState.buttons;
}

uint8_t GamepadDebouncer::debounceDpad(uint8_t dpadState, uint32_t changedDpad)
{
	uint32_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;
	uint32_t now = getMillis();

	for (int i = 0; i < 4; i++)
	{
		// If Dpad is a press, don't wait
		if ((changedDpad & dpadMasks[i]) && !(dpadState & dpadMasks[i]))
		{
			dpadState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
		// If Dpad is a release, wait for the debounce timer
		else if ((changedDpad & dpadMasks[i]) && ((now - dpadTime[i]) > debounceDelay))
		{
			dpadState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}

	return dpadState;
}

uint16_t GamepadDebouncer::debounceButtons(uint16_t buttonState, uint32_t changedButtons)
{
	uint32_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;
	uint32_t now = getMillis();
	
	for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++)
	{
		// If button is a press, don't wait
		if (changedButtons & buttonMasks[i]) {
			if ( !(buttonState & buttonMasks[i]) || 
				((now - buttonTime[i]) > debounceDelay)) {
				buttonState ^= buttonMasks[i];
				buttonTime[i] = now;
			}
		}
	}

	return buttonState;
}