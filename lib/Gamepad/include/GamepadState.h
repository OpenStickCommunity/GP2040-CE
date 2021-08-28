/* 
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_STATE_H_
#define GAMEPAD_STATE_H_

#include <stdint.h>

/*
	Gamepad button mapping table:

	+---------+--------+---------+--------+
	| Generic | XInput | Switch  | Arcade |
	+---------+--------+---------+--------+
	| 01      | A      | B       | K1     |
	| 02      | B      | A       | K2     |
	| 03      | X      | Y       | P1     |
	| 04      | Y      | X       | P2     |
	| 05      | LB     | L       | P4     |
	| 06      | RB     | R       | P3     |
	| 07      | LT     | ZL      | K4     |
	| 08      | RT     | ZR      | K3     |
	| 09      | Back   | -       | Coin   |
	| 10      | Start  | +       | Start  |
	| 11      | LS     | LS      | LS     |
	| 12      | RS     | RS      | RS     |
	| 13      | Guide  | Home    | -      |
	| 14      | RS     | Capture | -      |
	+---------+--------+---------+--------+
*/

#define GAMEPAD_MASK_UP    (1U << 0)
#define GAMEPAD_MASK_DOWN  (1U << 1)
#define GAMEPAD_MASK_LEFT  (1U << 2)
#define GAMEPAD_MASK_RIGHT (1U << 3)

#define GAMEPAD_MASK_01    (1U << 0)
#define GAMEPAD_MASK_02    (1U << 1)
#define GAMEPAD_MASK_03    (1U << 2)
#define GAMEPAD_MASK_04    (1U << 3)
#define GAMEPAD_MASK_05    (1U << 4)
#define GAMEPAD_MASK_06    (1U << 5)
#define GAMEPAD_MASK_07    (1U << 6)
#define GAMEPAD_MASK_08    (1U << 7)
#define GAMEPAD_MASK_09    (1U << 8)
#define GAMEPAD_MASK_10    (1U << 9)
#define GAMEPAD_MASK_11    (1U << 10)
#define GAMEPAD_MASK_12    (1U << 11)
#define GAMEPAD_MASK_13    (1U << 12)
#define GAMEPAD_MASK_14    (1U << 13)

#define GAMEPAD_MASK_DPAD (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)

#define GAMEPAD_JOYSTICK_MIN 0
#define GAMEPAD_JOYSTICK_MID 0x7FFF
#define GAMEPAD_JOYSTICK_MAX 0xFFFF

struct __attribute__((packed)) GamepadState
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
