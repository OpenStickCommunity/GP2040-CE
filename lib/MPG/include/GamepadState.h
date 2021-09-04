/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_STATE_H_
#define GAMEPAD_STATE_H_

#include <stdint.h>

/*
	Gamepad button mapping table:

	+---------+--------+---------+----------+----------+--------+
	| Generic | XInput | Switch  | PS3      | DInput   | Arcade |
	+---------+--------+---------+----------|----------+--------+
	| B1      | A      | B       | Cross    | 2        | K1     |
	| B2      | B      | A       | Circle   | 3        | K2     |
	| B3      | X      | Y       | Square   | 1        | P1     |
	| B4      | Y      | X       | Triangle | 4        | P2     |
	| L1      | LB     | L       | L1       | 5        | P4     |
	| R1      | RB     | R       | R1       | 6        | P3     |
	| L2      | LT     | ZL      | L2       | 7        | K4     |
	| R2      | RT     | ZR      | R2       | 8        | K3     |
	| S1      | Back   | -       | Select   | 9        | Coin   |
	| S2      | Start  | +       | Start    | 10       | Start  |
	| L3      | LS     | LS      | L3       | 11       | LS     |
	| R3      | RS     | RS      | R3       | 12       | RS     |
	| A1      | Guide  | Home    | -        | 13       | -      |
	| A2      | -      | Capture | -        | 14       | -      |
	+---------+--------+---------+----------+----------+--------+
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

#define GAMEPAD_MASK_DPAD (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)

#define GAMEPAD_JOYSTICK_MIN 0
#define GAMEPAD_JOYSTICK_MID 0x7FFF
#define GAMEPAD_JOYSTICK_MAX 0xFFFF

struct GamepadState
{
	uint8_t dpad;
	uint32_t buttons;
	uint16_t lx;
	uint16_t ly;
	uint16_t rx;
	uint16_t ry;
	uint8_t lt;
	uint8_t rt;
};

#endif
