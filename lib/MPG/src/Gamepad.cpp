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

uint8_t *GamepadClass::getReport()
{
	switch (inputMode)
	{
		case INPUT_MODE_XINPUT:
			return (uint8_t *)getXInputReport();

		case INPUT_MODE_SWITCH:
			return (uint8_t *)getSwitchReport();

		default:
			return (uint8_t *)getHIDReport();
	}
}

uint8_t GamepadClass::getReportSize()
{
	switch (inputMode)
	{
		case INPUT_MODE_XINPUT:
			return sizeof(XInputReport);

		case INPUT_MODE_SWITCH:
			return sizeof(SwitchReport);

		default:
			return sizeof(HIDReport);
	}
}

HIDReport *GamepadClass::getHIDReport()
{
	static HIDReport hidReport =
	{
		.buttons = 0,
		.hat = HID_HAT_NOTHING,
		.lx = HID_JOYSTICK_MID,
		.ly = HID_JOYSTICK_MID,
		.rx = HID_JOYSTICK_MID,
		.ry = HID_JOYSTICK_MID,
	};

	hidReport.lx = state.lx >> 8;
	hidReport.ly = state.ly >> 8;
	hidReport.rx = state.rx >> 8;
	hidReport.ry = state.ry >> 8;

	switch (state.dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case GAMEPAD_MASK_UP:                        hidReport.hat = HID_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   hidReport.hat = HID_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     hidReport.hat = HID_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: hidReport.hat = HID_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      hidReport.hat = HID_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  hidReport.hat = HID_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      hidReport.hat = HID_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    hidReport.hat = HID_HAT_UPLEFT;    break;
		default:                                     hidReport.hat = HID_HAT_NOTHING;   break;
	}

	hidReport.buttons = 0
		| (pressedB3() ? HID_MASK_SQUARE   : 0)
		| (pressedB1() ? HID_MASK_CROSS    : 0)
		| (pressedB2() ? HID_MASK_CIRCLE   : 0)
		| (pressedB4() ? HID_MASK_TRIANGLE : 0)
		| (pressedL1() ? HID_MASK_L1       : 0)
		| (pressedR1() ? HID_MASK_R1       : 0)
		| (pressedL2() ? HID_MASK_L2       : 0)
		| (pressedR2() ? HID_MASK_R2       : 0)
		| (pressedS1() ? HID_MASK_SELECT   : 0)
		| (pressedS2() ? HID_MASK_START    : 0)
		| (pressedL3() ? HID_MASK_L3       : 0)
		| (pressedR3() ? HID_MASK_R3       : 0)
		| (pressedA1() ? HID_MASK_PS       : 0)
		| (pressedA2() ? HID_MASK_TP       : 0)
	;

	return &hidReport;
}

SwitchReport *GamepadClass::getSwitchReport()
{
	static SwitchReport switchReport =
	{
		.buttons = 0,
		.hat = 0,
		.lx = SWITCH_JOYSTICK_MID,
		.ly = SWITCH_JOYSTICK_MID,
		.rx = SWITCH_JOYSTICK_MID,
		.ry = SWITCH_JOYSTICK_MID,
		.vendor = 0,
	};

	switchReport.lx = state.lx >> 8;
	switchReport.ly = state.ly >> 8;
	switchReport.rx = state.rx >> 8;
	switchReport.ry = state.ry >> 8;

	switch (state.dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case GAMEPAD_MASK_UP:                        switchReport.hat = SWITCH_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   switchReport.hat = SWITCH_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     switchReport.hat = SWITCH_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: switchReport.hat = SWITCH_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      switchReport.hat = SWITCH_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  switchReport.hat = SWITCH_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      switchReport.hat = SWITCH_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    switchReport.hat = SWITCH_HAT_UPLEFT;    break;
		default:                                     switchReport.hat = SWITCH_HAT_NOTHING;   break;
	}

	switchReport.buttons = 0
		| (pressedB3() ? SWITCH_MASK_Y       : 0)
		| (pressedB1() ? SWITCH_MASK_B       : 0)
		| (pressedB2() ? SWITCH_MASK_A       : 0)
		| (pressedB4() ? SWITCH_MASK_X       : 0)
		| (pressedL1() ? SWITCH_MASK_L       : 0)
		| (pressedR1() ? SWITCH_MASK_R       : 0)
		| (pressedL2() ? SWITCH_MASK_ZL      : 0)
		| (pressedR2() ? SWITCH_MASK_ZR      : 0)
		| (pressedS1() ? SWITCH_MASK_MINUS   : 0)
		| (pressedS2() ? SWITCH_MASK_PLUS    : 0)
		| (pressedL3() ? SWITCH_MASK_L3      : 0)
		| (pressedR3() ? SWITCH_MASK_R3      : 0)
		| (pressedA1() ? SWITCH_MASK_HOME    : 0)
		| (pressedA2() ? SWITCH_MASK_CAPTURE : 0)
	;

	return &switchReport;
}

XInputReport *GamepadClass::getXInputReport()
{
	static XInputReport xinputReport =
	{
		.report_id = 0,
		.report_size = XINPUT_REPORT_SIZE,
		.buttons1 = 0,
		.buttons2 = 0,
		.lt = 0,
		.rt = 0,
		.lx = GAMEPAD_JOYSTICK_MID,
		.ly = GAMEPAD_JOYSTICK_MID,
		.rx = GAMEPAD_JOYSTICK_MID,
		.ry = GAMEPAD_JOYSTICK_MID,
		._reserved = { },
	};

	xinputReport.lx = state.lx + -32768;
	xinputReport.ly = ~(state.ly) + -32768;
	xinputReport.rx = state.rx + -32768;
	xinputReport.ry = ~(state.ry) + -32768;

	// Convert button states
	xinputReport.buttons1 = state.dpad
		| (pressedS2() ? XBOX_MASK_START : 0)
		| (pressedS1() ? XBOX_MASK_BACK  : 0)
		| (pressedL3() ? XBOX_MASK_LS    : 0)
		| (pressedR3() ? XBOX_MASK_RS    : 0)
	;

	xinputReport.buttons2 = 0
		| (pressedL1() ? XBOX_MASK_LB   : 0)
		| (pressedR1() ? XBOX_MASK_RB   : 0)
		| (pressedA1() ? XBOX_MASK_HOME : 0)
		| (pressedB1() ? XBOX_MASK_A    : 0)
		| (pressedB2() ? XBOX_MASK_B    : 0)
		| (pressedB3() ? XBOX_MASK_X    : 0)
		| (pressedB4() ? XBOX_MASK_Y    : 0)
	;

	// Handle trigger values
	if (hasAnalogTriggers)
	{
		xinputReport.lt = state.lt;
		xinputReport.rt = state.rt;
	}
	else
	{
		xinputReport.lt = (state.buttons & GAMEPAD_MASK_L2) ? 0xFF : 0;
		xinputReport.rt = (state.buttons & GAMEPAD_MASK_R2) ? 0xFF : 0;
	}

	return &xinputReport;
}

GamepadHotkey GamepadClass::hotkey()
{
	GamepadHotkey action = HOTKEY_NONE;
	if (isDpadHotkeyPressed())
	{
		DpadMode lastDpadMode = dpadMode;
		if (pressedLeft())
		{
			action = HOTKEY_DPAD_LEFT_ANALOG;
			dpadMode = DPAD_MODE_LEFT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (pressedRight())
		{
			action = HOTKEY_DPAD_RIGHT_ANALOG;
			dpadMode = DPAD_MODE_RIGHT_ANALOG;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (pressedDown())
		{
			action = HOTKEY_DPAD_DIGITAL;
			dpadMode = DPAD_MODE_DIGITAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (pressedUp())
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
		if (pressedUp())
		{
			action = HOTKEY_SOCD_UP_PRIORITY;
			socdMode = SOCD_MODE_UP_PRIORITY;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
		}
		else if (pressedDown())
		{
			action = HOTKEY_SOCD_NEUTRAL;
			socdMode = SOCD_MODE_NEUTRAL;
			state.dpad = 0;
			state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3);
		}
		else if (pressedLeft())
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

void GamepadClass::load()
{
	inputMode = Storage.getInputMode();
	if (inputMode > INPUT_MODE_HID)
		inputMode = DEFAULT_INPUT_MODE;

	dpadMode = Storage.getDpadMode();
	if (dpadMode > DPAD_MODE_RIGHT_ANALOG)
		dpadMode = DEFAULT_DPAD_MODE;

	socdMode = Storage.getSOCDMode();;
	if (socdMode > SOCD_MODE_SECOND_INPUT_PRIORITY)
		socdMode = DEFAULT_SOCD_MODE;
}

void GamepadClass::save()
{
	Storage.setInputMode(inputMode);
	Storage.setDpadMode(dpadMode);
	Storage.setSOCDMode(socdMode);
	Storage.save();
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
