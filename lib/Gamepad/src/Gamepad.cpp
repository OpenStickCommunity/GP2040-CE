/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "Gamepad.h"

void GamepadClass::debounce()
{
	for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
	{
		// Did we detect a change?
		if (debouncers[i].update())
		{
			// Was it a press?
			if (debouncers[i].rose())
			{
				// Dpad or buttons?
				if (debouncers[i].isDpad)
					state.dpad |= debouncers[i].inputMask;
				else
					state.buttons |= debouncers[i].inputMask;
			}
			else
			{
				if (debouncers[i].isDpad)
					state.dpad &= ~(debouncers[i].inputMask);
				else
					state.buttons &= ~(debouncers[i].inputMask);
			}
		}
	}
}

GamepadHotkey GamepadClass::hotkey()
{
	GamepadHotkey action = HOTKEY_NONE;
	if (isDpadHotkeyPressed())
	{
		DpadMode lastDpadMode = dpadMode;
		if (state.pressedLeft())
		{
			action = HOTKEY_DPAD_LEFT_ANALOG;
			dpadMode = DPAD_MODE_LEFT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (state.pressedRight())
		{
			action = HOTKEY_DPAD_RIGHT_ANALOG;
			dpadMode = DPAD_MODE_RIGHT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (state.pressedDown())
		{
			action = HOTKEY_DPAD_DIGITAL;
			dpadMode = DPAD_MODE_DIGITAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (state.pressedUp())
		{
			action = HOTKEY_HOME_BUTTON;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
			state.buttons |= GAMEPAD_MASK_A1; // Press the Home button
		}

		if (lastDpadMode != dpadMode)
			save();
	}
	else if (isSOCDHotkeyPressed())
	{
		SOCDMode lastSOCDMode = socdMode;
		if (state.pressedUp())
		{
			action = HOTKEY_SOCD_UP_PRIORITY;
			socdMode = SOCD_MODE_UP_PRIORITY;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
		}
		else if (state.pressedDown())
		{
			action = HOTKEY_SOCD_NEUTRAL;
			socdMode = SOCD_MODE_NEUTRAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
		}
		else if (state.pressedLeft())
		{
			action = HOTKEY_SOCD_LAST_INPUT;
			socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
		}

		if (lastSOCDMode != socdMode)
			save();
	}

	return action;
}

void GamepadClass::process()
{
	state.dpad = runSOCDCleaner(socdMode, state.dpad);

	switch (dpadMode)
	{
		case DPAD_MODE_LEFT_ANALOG:
			state.lx = dpadToAnalogX(state.dpad);
			state.ly = dpadToAnalogY(state.dpad);
			state.dpad = 0;
			break;

		case DPAD_MODE_RIGHT_ANALOG:
			state.rx = dpadToAnalogX(state.dpad);
			state.ry = dpadToAnalogY(state.dpad);
			state.dpad = 0;
			break;

		default:
			break;
	}
}

bool GamepadClass::isDpadHotkeyPressed()
{
	// Select + Start
	return (state.buttons & (GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2)) == (GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
}

bool GamepadClass::isSOCDHotkeyPressed()
{
	// LS + RS
	return (state.buttons & (GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3)) == (GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
}
