/*
* SPDX-License-Identifier: MIT
* SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
*/

#include <stdio.h>

#include "usb_driver.h"
#include "switch_interface.h"
#include "xinput_interface.h"

#include "StateConverter.h"
#include "GamepadState.h"

SwitchReport switch_report =
{
	.buttons = 0,
	.hat = 0,
	.lx = SWITCH_JOYSTICK_MID,
	.ly = SWITCH_JOYSTICK_MID,
	.rx = SWITCH_JOYSTICK_MID,
	.ry = SWITCH_JOYSTICK_MID,
	.vendor = 0,
};

XInputReport xinput_report =
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

static SwitchReport *fill_switch_report(GamepadState *state)
{
	// Handle HAT switch (D-Pad)
	switch (state->dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case GAMEPAD_MASK_UP:                        switch_report.hat = SWITCH_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   switch_report.hat = SWITCH_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     switch_report.hat = SWITCH_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: switch_report.hat = SWITCH_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      switch_report.hat = SWITCH_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  switch_report.hat = SWITCH_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      switch_report.hat = SWITCH_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    switch_report.hat = SWITCH_HAT_UPLEFT;    break;
		default:                                     switch_report.hat = SWITCH_HAT_NOTHING;   break;
	}

	// Convert button states
	switch_report.buttons = 0
		| (state->pressedB3() ? SWITCH_MASK_Y       : 0)
		| (state->pressedB1() ? SWITCH_MASK_B       : 0)
		| (state->pressedB2() ? SWITCH_MASK_A       : 0)
		| (state->pressedB4() ? SWITCH_MASK_X       : 0)
		| (state->pressedL1() ? SWITCH_MASK_L       : 0)
		| (state->pressedR1() ? SWITCH_MASK_R       : 0)
		| (state->pressedL2() ? SWITCH_MASK_ZL      : 0)
		| (state->pressedR2() ? SWITCH_MASK_ZR      : 0)
		| (state->pressedS1() ? SWITCH_MASK_MINUS   : 0)
		| (state->pressedS2() ? SWITCH_MASK_PLUS    : 0)
		| (state->pressedL3() ? SWITCH_MASK_L3      : 0)
		| (state->pressedR3() ? SWITCH_MASK_R3      : 0)
		| (state->pressedA1() ? SWITCH_MASK_HOME    : 0)
		| (state->pressedA2() ? SWITCH_MASK_CAPTURE : 0)
	;

	// Direct assignments
	switch_report.lx = state->lx >> 8;
	switch_report.ly = state->ly >> 8;
	switch_report.rx = state->rx >> 8;
	switch_report.ry = state->ry >> 8;

	return &switch_report;
}

static XInputReport *fill_xinput_report(GamepadState *state, bool has_analog_triggers)
{
	// Convert button states
	xinput_report.buttons1 = state->dpad
		| (state->pressedS2() ? XBOX_MASK_START : 0)
		| (state->pressedS1() ? XBOX_MASK_BACK  : 0)
		| (state->pressedL3() ? XBOX_MASK_LS    : 0)
		| (state->pressedR3() ? XBOX_MASK_RS    : 0)
	;

	xinput_report.buttons2 = 0
		| (state->pressedL1() ? XBOX_MASK_LB   : 0)
		| (state->pressedR1() ? XBOX_MASK_RB   : 0)
		| (state->pressedA1() ? XBOX_MASK_HOME : 0)
		| (state->pressedB1() ? XBOX_MASK_A    : 0)
		| (state->pressedB2() ? XBOX_MASK_B    : 0)
		| (state->pressedB3() ? XBOX_MASK_X    : 0)
		| (state->pressedB4() ? XBOX_MASK_Y    : 0)
	;

	// Direct assignments
	xinput_report.lx = state->lx + -32768;
	xinput_report.ly = ~(state->ly) + -32768;
	xinput_report.rx = state->rx + -32768;
	xinput_report.ry = ~(state->ry) + -32768;

	// Handle trigger values
	if (has_analog_triggers)
	{
		xinput_report.lt = state->lt;
		xinput_report.rt = state->rt;
	}
	else
	{
		xinput_report.lt = (state->buttons & GAMEPAD_MASK_L2) ? 0xFF : 0;
		xinput_report.rt = (state->buttons & GAMEPAD_MASK_R2) ? 0xFF : 0;
	}

	return &xinput_report;
}

void *fill_report(GamepadState *state, bool has_analog_triggers)
{
	switch (current_input_mode)
	{
		case XINPUT:
			return fill_xinput_report(state, has_analog_triggers);

		case SWITCH:
		default:
			return fill_switch_report(state);
	}

	return NULL;
}

void *select_report(uint8_t *report_size, InputMode mode)
{
	switch (current_input_mode)
	{
		case XINPUT:
			*report_size = sizeof(xinput_report);
			return &xinput_report;

		case SWITCH:
		default:
			*report_size = sizeof(switch_report);
			return &switch_report;
	}
}
