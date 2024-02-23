/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "gamepad/GamepadDebouncer.h"
#include "storagemanager.h"

void GamepadDebouncer::debounce(GamepadState *state) {
	// Return if dpad and button states haven't changed
	if ((debounceState.dpad == state->dpad) && (debounceState.buttons == state->buttons)){
		return;
	}

	uint32_t now = getMillis();

	// Debounce the Dpad
	if (debounceState.dpad != state->dpad) {
		state->dpad = debounceDpad(debounceState.dpad, state->dpad, now);
		debounceState.dpad = state->dpad;
	}

	// Debounce the buttons
	if (debounceState.buttons != state->buttons) {
		state->buttons = debounceButtons(debounceState.buttons, state->buttons, now);
		debounceState.buttons = state->buttons;
	}
}

uint8_t GamepadDebouncer::debounceDpad(uint8_t oldDpad, uint8_t newDpad, uint32_t now) {
	static uint16_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;

	for (int i = 0; i < 4; i++) {
		// Allow debouncer to change state if dpad state changed and debounce delay threshold met
		if ((oldDpad & dpadMasks[i]) != (newDpad & dpadMasks[i]) && (now - dpadTime[i]) > debounceDelay) {
			newDpad ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}

	return newDpad;
}

uint16_t GamepadDebouncer::debounceButtons(uint16_t oldButtons, uint16_t newButtons, uint32_t now) {
	static uint16_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;

	for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++) {
		// Allow debouncer to change state if button state changed and debounce delay threshold met
		if ((oldButtons & buttonMasks[i]) != (newButtons & buttonMasks[i]) && ((now - buttonTime[i]) >= debounceDelay)) {
			newButtons ^= buttonMasks[i];
			buttonTime[i] = now;
		}
	}

	return newButtons;
}
