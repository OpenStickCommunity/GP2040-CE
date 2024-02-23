/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "gamepad/GamepadDebouncer.h"
#include "storagemanager.h"

void GamepadDebouncer::debounce(GamepadState *state)
{
	const uint32_t debounceMS = Storage::getInstance().getGamepadOptions().debounceDelay;
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
