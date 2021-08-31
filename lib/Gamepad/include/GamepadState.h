/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_STATE_H_
#define GAMEPAD_STATE_H_

#include <stdint.h>

/*
	Gamepad button mapping table:

	+---------+--------+----------+---------+--------+
	| Generic | XInput | PS3      | Switch  | Arcade |
	+---------+--------+----------+---------+--------+
	| B1      | A      | Cross    | B       | K1     |
	| B2      | B      | Circle   | A       | K2     |
	| B3      | X      | Square   | Y       | P1     |
	| B4      | Y      | Triangle | X       | P2     |
	| L1      | LB     | L1       | L       | P4     |
	| R1      | RB     | R1       | R       | P3     |
	| L2      | LT     | L2       | ZL      | K4     |
	| R2      | RT     | R2       | ZR      | K3     |
	| S1      | Back   | Select   | -       | Coin   |
	| S2      | Start  | Start    | +       | Start  |
	| L3      | LS     | L3       | LS      | LS     |
	| R3      | RS     | R3       | RS      | RS     |
	| A1      | Guide  | PS       | Home    | -      |
	| A2      | -      | -        | Capture | -      |
	+---------+--------+----------+---------+--------+
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

	inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask)    { return (dpad & mask) == mask; }
	inline bool __attribute__((always_inline)) pressedButton(const uint32_t mask) { return (buttons & mask) == mask; }

	inline bool __attribute__((always_inline)) pressedUp()    { return pressedDpad(GAMEPAD_MASK_UP); }
	inline bool __attribute__((always_inline)) pressedDown()  { return pressedDpad(GAMEPAD_MASK_DOWN); }
	inline bool __attribute__((always_inline)) pressedLeft()  { return pressedDpad(GAMEPAD_MASK_LEFT); }
	inline bool __attribute__((always_inline)) pressedRight() { return pressedDpad(GAMEPAD_MASK_RIGHT); }
	inline bool __attribute__((always_inline)) pressedB1()    { return pressedButton(GAMEPAD_MASK_B1); }
	inline bool __attribute__((always_inline)) pressedB2()    { return pressedButton(GAMEPAD_MASK_B2); }
	inline bool __attribute__((always_inline)) pressedB3()    { return pressedButton(GAMEPAD_MASK_B3); }
	inline bool __attribute__((always_inline)) pressedB4()    { return pressedButton(GAMEPAD_MASK_B4); }
	inline bool __attribute__((always_inline)) pressedL1()    { return pressedButton(GAMEPAD_MASK_L1); }
	inline bool __attribute__((always_inline)) pressedR1()    { return pressedButton(GAMEPAD_MASK_R1); }
	inline bool __attribute__((always_inline)) pressedL2()    { return pressedButton(GAMEPAD_MASK_L2); }
	inline bool __attribute__((always_inline)) pressedR2()    { return pressedButton(GAMEPAD_MASK_R2); }
	inline bool __attribute__((always_inline)) pressedS1()    { return pressedButton(GAMEPAD_MASK_S1); }
	inline bool __attribute__((always_inline)) pressedS2()    { return pressedButton(GAMEPAD_MASK_S2); }
	inline bool __attribute__((always_inline)) pressedL3()    { return pressedButton(GAMEPAD_MASK_L3); }
	inline bool __attribute__((always_inline)) pressedR3()    { return pressedButton(GAMEPAD_MASK_R3); }
	inline bool __attribute__((always_inline)) pressedA1()    { return pressedButton(GAMEPAD_MASK_A1); }
	inline bool __attribute__((always_inline)) pressedA2()    { return pressedButton(GAMEPAD_MASK_A2); }
};

#endif
