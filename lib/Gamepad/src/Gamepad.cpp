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
		if (isDpadLeftPressed())
		{
			action = HOTKEY_DPAD_LEFT_ANALOG;
			dpadMode = DPAD_MODE_LEFT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
		}
		else if (isDpadRightPressed())
		{
			action = HOTKEY_DPAD_RIGHT_ANALOG;
			dpadMode = DPAD_MODE_RIGHT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
		}
		else if (isDpadDownPressed())
		{
			action = HOTKEY_DPAD_DIGITAL;
			dpadMode = DPAD_MODE_DIGITAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
		}
		else if (isDpadUpPressed())
		{
			action = HOTKEY_HOME_BUTTON;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
			state.buttons |= GAMEPAD_MASK_13; // Press the Home button
		}
		else if (is1PPressed())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_UP;
			state.buttons &= ~(GAMEPAD_MASK_03 | GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
		}
		else if (is1KPressed())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_DOWN;
			state.buttons &= ~(GAMEPAD_MASK_01 | GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
		}

		if (lastDpadMode != dpadMode)
			save();
	}
	else if (isSOCDHotkeyPressed())
	{
		SOCDMode lastSOCDMode = socdMode;
		if (isDpadUpPressed())
		{
			action = HOTKEY_SOCD_UP_PRIORITY;
			socdMode = SOCD_MODE_UP_PRIORITY;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_11 | GAMEPAD_MASK_12);
		}
		else if (isDpadDownPressed())
		{
			action = HOTKEY_SOCD_NEUTRAL;
			socdMode = SOCD_MODE_NEUTRAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_11 | GAMEPAD_MASK_12);
		}
		else if (isDpadLeftPressed())
		{
			action = HOTKEY_SOCD_LAST_INPUT;
			socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_11 | GAMEPAD_MASK_12);
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
	return (state.buttons & (GAMEPAD_MASK_09 | GAMEPAD_MASK_10)) == (GAMEPAD_MASK_09 | GAMEPAD_MASK_10);
}

bool GamepadClass::isSOCDHotkeyPressed()
{
	// LS + RS
	return (state.buttons & (GAMEPAD_MASK_11 | GAMEPAD_MASK_12)) == (GAMEPAD_MASK_11 | GAMEPAD_MASK_12);
}
