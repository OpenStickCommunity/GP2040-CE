/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "GamepadEnums.h"
#include "enums.pb.h"

#define GAMEPAD_BUTTON_COUNT 14

/*
	Gamepad button mapping table:

	+--------+--------+---------+----------+----------+--------+
	| GP2040 | XInput | Switch  | PS3      | DInput   | Arcade |
	+--------+--------+---------+----------|----------+--------+
	| B1     | A      | B       | Cross    | 2        | K1     |
	| B2     | B      | A       | Circle   | 3        | K2     |
	| B3     | X      | Y       | Square   | 1        | P1     |
	| B4     | Y      | X       | Triangle | 4        | P2     |
	| L1     | LB     | L       | L1       | 5        | P4     |
	| R1     | RB     | R       | R1       | 6        | P3     |
	| L2     | LT     | ZL      | L2       | 7        | K4     |
	| R2     | RT     | ZR      | R2       | 8        | K3     |
	| S1     | Back   | -       | Select   | 9        | Coin   |
	| S2     | Start  | +       | Start    | 10       | Start  |
	| L3     | LS     | LS      | L3       | 11       | LS     |
	| R3     | RS     | RS      | R3       | 12       | RS     |
	| A1     | Guide  | Home    | -        | 13       | -      |
	| A2     | -      | Capture | -        | 14       | -      |
	+--------+--------+---------+----------+----------+--------+
*/

#define GAMEPAD_MASK_UP    (1U << 0)
#define GAMEPAD_MASK_DOWN  (1U << 1)
#define GAMEPAD_MASK_LEFT  (1U << 2)
#define GAMEPAD_MASK_RIGHT (1U << 3)

#define GAMEPAD_MASK_B1    (1U << 0)
#define GAMEPAD_MASK_B2    (1U << 1)
#define GAMEPAD_MASK_B3    (1U << 2)
#define GAMEPAD_MASK_B4    (1U << 3)
#define GAMEPAD_MASK_L1    (1U << 4)
#define GAMEPAD_MASK_R1    (1U << 5)
#define GAMEPAD_MASK_L2    (1U << 6)
#define GAMEPAD_MASK_R2    (1U << 7)
#define GAMEPAD_MASK_S1    (1U << 8)
#define GAMEPAD_MASK_S2    (1U << 9)
#define GAMEPAD_MASK_L3    (1U << 10)
#define GAMEPAD_MASK_R3    (1U << 11)
#define GAMEPAD_MASK_A1    (1U << 12)
#define GAMEPAD_MASK_A2    (1U << 13)

// For detecting dpad as buttons

#define GAMEPAD_MASK_DU    (1UL << 16)
#define GAMEPAD_MASK_DD    (1UL << 17)
#define GAMEPAD_MASK_DL    (1UL << 18)
#define GAMEPAD_MASK_DR    (1UL << 19)

// For detecting analog sticks as buttons

#define GAMEPAD_MASK_LX    (1UL << 20)
#define GAMEPAD_MASK_LY    (1UL << 21)
#define GAMEPAD_MASK_RX    (1UL << 22)
#define GAMEPAD_MASK_RY    (1UL << 23)

#define GAMEPAD_MASK_DPAD (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)

#define GAMEPAD_JOYSTICK_MIN 0
#define GAMEPAD_JOYSTICK_MID 0x7FFF
#define GAMEPAD_JOYSTICK_MAX 0xFFFF

/**
 * @brief AUX defines --- gamepad state that doesn't translate to an output button/dpad/etc.
 */
#define AUX_MASK_FUNCTION	(1U << 15)

const uint8_t dpadMasks[] =
{
	GAMEPAD_MASK_UP,
	GAMEPAD_MASK_DOWN,
	GAMEPAD_MASK_LEFT,
	GAMEPAD_MASK_RIGHT,
};

const uint16_t buttonMasks[] =
{
	GAMEPAD_MASK_B1,
	GAMEPAD_MASK_B2,
	GAMEPAD_MASK_B3,
	GAMEPAD_MASK_B4,
	GAMEPAD_MASK_L1,
	GAMEPAD_MASK_R1,
	GAMEPAD_MASK_L2,
	GAMEPAD_MASK_R2,
	GAMEPAD_MASK_S1,
	GAMEPAD_MASK_S2,
	GAMEPAD_MASK_L3,
	GAMEPAD_MASK_R3,
	GAMEPAD_MASK_A1,
	GAMEPAD_MASK_A2,
};

struct GamepadState
{
	uint8_t dpad {0};
	uint16_t buttons {0};
	uint16_t aux {0};
	uint16_t lx {GAMEPAD_JOYSTICK_MID};
	uint16_t ly {GAMEPAD_JOYSTICK_MID};
	uint16_t rx {GAMEPAD_JOYSTICK_MID};
	uint16_t ry {GAMEPAD_JOYSTICK_MID};
	uint8_t lt {0};
	uint8_t rt {0};
};

// Convert the horizontal GamepadState dpad axis value into an analog value
inline uint16_t dpadToAnalogX(uint8_t dpad)
{
	switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case GAMEPAD_MASK_LEFT:
			return GAMEPAD_JOYSTICK_MIN;

		case GAMEPAD_MASK_RIGHT:
			return GAMEPAD_JOYSTICK_MAX;

		default:
			return GAMEPAD_JOYSTICK_MID;
	}
}

// Convert the vertical GamepadState dpad axis value into an analog value
inline uint16_t dpadToAnalogY(uint8_t dpad)
{
	switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case GAMEPAD_MASK_UP:
			return GAMEPAD_JOYSTICK_MIN;

		case GAMEPAD_MASK_DOWN:
			return GAMEPAD_JOYSTICK_MAX;

		default:
			return GAMEPAD_JOYSTICK_MID;
	}
}

/**
 * @brief DRY method used in filtering diagonals for 4-way mode, checks two adjacent directions
 * and tracks how they toggle.
 *
 * @param dir_a Direction state A to track.
 * @param dir_mask_a Dpad mask state A to check/deselect.
 * @param dir_b Direction state B to track.
 * @param dir_mask_b Dpad mask state B to check/deselect.
 * @param state History for this diagonal (affects which cardinal to deselect).
 * @return uint8_t The modified dpad
 */
inline uint8_t diagonal_check(uint8_t dpad, DpadDirection dir_a, uint16_t dir_mask_a, DpadDirection dir_b,
		uint16_t dir_mask_b, DpadDirection* state) {
	uint8_t newDpad = ~0;
	if ((dpad & (dir_mask_a | dir_mask_b)) == (dir_mask_a | dir_mask_b)) {
		newDpad = (*state == dir_a) ? ~dir_mask_a : ~dir_mask_b;
	} else if ((dpad & (dir_mask_a | dir_mask_b)) == dir_mask_a) {
		*state = dir_a;
	} else if ((dpad & (dir_mask_a | dir_mask_b)) == dir_mask_b) {
		*state = dir_b;
	}
	return newDpad;
}

/**
 * @brief Filter diagonals out of the dpad, making the device work as a 4-way lever.
 *
 * The most recent cardinal direction wins.
 *
 * @param dpad The GameState.dpad value.
 * @return uint8_t The new dpad value.
 */
inline uint8_t filterToFourWayMode(uint8_t dpad)
{
	static DpadDirection lastUL = DIRECTION_NONE;
	static DpadDirection lastUR = DIRECTION_NONE;
	static DpadDirection lastDR = DIRECTION_NONE;
	static DpadDirection lastDL = DIRECTION_NONE;

	dpad &= diagonal_check(dpad, DIRECTION_LEFT, GAMEPAD_MASK_LEFT, DIRECTION_UP, GAMEPAD_MASK_UP, &lastUL);
	dpad &= diagonal_check(dpad, DIRECTION_UP, GAMEPAD_MASK_UP, DIRECTION_RIGHT, GAMEPAD_MASK_RIGHT, &lastUR);
	dpad &= diagonal_check(dpad, DIRECTION_RIGHT, GAMEPAD_MASK_RIGHT, DIRECTION_DOWN, GAMEPAD_MASK_DOWN, &lastDR);
	dpad &= diagonal_check(dpad, DIRECTION_DOWN, GAMEPAD_MASK_DOWN, DIRECTION_LEFT, GAMEPAD_MASK_LEFT, &lastDL);

	return dpad;
}

/**
 * @brief Run SOCD cleaning against a D-pad value.
 *
 * @param mode The SOCD cleaning mode.
 * @param dpad The GamepadState.dpad value.
 * @return uint8_t The clean D-pad value.
 */
inline uint8_t runSOCDCleaner(SOCDMode mode, uint8_t dpad)
{
	if (mode == SOCD_MODE_BYPASS) {
		return dpad;
	}

	static DpadDirection lastUD = DIRECTION_NONE;
	static DpadDirection lastLR = DIRECTION_NONE;
	uint8_t newDpad = 0;

	switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (mode == SOCD_MODE_UP_PRIORITY)
			{
				newDpad |= GAMEPAD_MASK_UP;
				lastUD = DIRECTION_UP;
			}
			else if (mode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastUD != DIRECTION_NONE)
				newDpad |= (lastUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else if (mode == SOCD_MODE_FIRST_INPUT_PRIORITY && lastUD != DIRECTION_NONE)
				newDpad |= (lastUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
			else
				lastUD = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_UP:
			newDpad |= GAMEPAD_MASK_UP;
			lastUD = DIRECTION_UP;
			break;

		case GAMEPAD_MASK_DOWN:
			newDpad |= GAMEPAD_MASK_DOWN;
			lastUD = DIRECTION_DOWN;
			break;

		default:
			lastUD = DIRECTION_NONE;
			break;
	}

	switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (mode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastLR != DIRECTION_NONE)
				newDpad |= (lastLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else if (mode == SOCD_MODE_FIRST_INPUT_PRIORITY && lastLR != DIRECTION_NONE)
				newDpad |= (lastLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
			else
				lastLR = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_LEFT:
			newDpad |= GAMEPAD_MASK_LEFT;
			lastLR = DIRECTION_LEFT;
			break;

		case GAMEPAD_MASK_RIGHT:
			newDpad |= GAMEPAD_MASK_RIGHT;
			lastLR = DIRECTION_RIGHT;
			break;

		default:
			lastLR = DIRECTION_NONE;
			break;
	}

	return newDpad;
}
