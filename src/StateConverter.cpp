/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <stdio.h>

#include "usb_driver.h"
#include "switch_device.h"
#include "xinput_device.h"

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
		| ((state->buttons & GAMEPAD_MASK_03) ? SWITCH_MASK_Y       : 0)
		| ((state->buttons & GAMEPAD_MASK_01) ? SWITCH_MASK_B       : 0)
		| ((state->buttons & GAMEPAD_MASK_02) ? SWITCH_MASK_A       : 0)
		| ((state->buttons & GAMEPAD_MASK_04) ? SWITCH_MASK_X       : 0)
		| ((state->buttons & GAMEPAD_MASK_05) ? SWITCH_MASK_L       : 0)
		| ((state->buttons & GAMEPAD_MASK_06) ? SWITCH_MASK_R       : 0)
		| ((state->buttons & GAMEPAD_MASK_07) ? SWITCH_MASK_ZL      : 0)
		| ((state->buttons & GAMEPAD_MASK_08) ? SWITCH_MASK_ZR      : 0)
		| ((state->buttons & GAMEPAD_MASK_09) ? SWITCH_MASK_MINUS   : 0)
		| ((state->buttons & GAMEPAD_MASK_10) ? SWITCH_MASK_PLUS    : 0)
		| ((state->buttons & GAMEPAD_MASK_11) ? SWITCH_MASK_L3      : 0)
		| ((state->buttons & GAMEPAD_MASK_12) ? SWITCH_MASK_R3      : 0)
		| ((state->buttons & GAMEPAD_MASK_13) ? SWITCH_MASK_HOME    : 0)
		| ((state->buttons & GAMEPAD_MASK_14) ? SWITCH_MASK_CAPTURE : 0)
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
		| ((state->buttons & GAMEPAD_MASK_10) ? XBOX_MASK_START : 0)
		| ((state->buttons & GAMEPAD_MASK_09) ? XBOX_MASK_BACK  : 0)
		| ((state->buttons & GAMEPAD_MASK_11) ? XBOX_MASK_LS    : 0)
		| ((state->buttons & GAMEPAD_MASK_12) ? XBOX_MASK_RS    : 0)
	;

	xinput_report.buttons2 = 0
		| ((state->buttons & GAMEPAD_MASK_05) ? XBOX_MASK_LB   : 0)
		| ((state->buttons & GAMEPAD_MASK_06) ? XBOX_MASK_RB   : 0)
		| ((state->buttons & GAMEPAD_MASK_13) ? XBOX_MASK_HOME : 0)
		| ((state->buttons & GAMEPAD_MASK_01) ? XBOX_MASK_A    : 0)
		| ((state->buttons & GAMEPAD_MASK_02) ? XBOX_MASK_B    : 0)
		| ((state->buttons & GAMEPAD_MASK_03) ? XBOX_MASK_X    : 0)
		| ((state->buttons & GAMEPAD_MASK_04) ? XBOX_MASK_Y    : 0)
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
		xinput_report.lt = (state->buttons & GAMEPAD_MASK_07) ? 0xFF : 0;
		xinput_report.rt = (state->buttons & GAMEPAD_MASK_08) ? 0xFF : 0;
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
			*report_size = sizeof(XInputReport);
			return &xinput_report;

		case SWITCH:
		default:
			*report_size = sizeof(SwitchReport);
			return &switch_report;
	}
}
