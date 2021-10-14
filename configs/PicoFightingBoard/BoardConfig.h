/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_FIGHTING_BOARD_CONFIG_H_
#define PICO_FIGHTING_BOARD_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Pixel.hpp"

#define PIN_DPAD_UP     1
#define PIN_DPAD_DOWN   2
#define PIN_DPAD_LEFT   0
#define PIN_DPAD_RIGHT  3
#define PIN_BUTTON_B1   11
#define PIN_BUTTON_B2   12
#define PIN_BUTTON_B3   7
#define PIN_BUTTON_B4   8
#define PIN_BUTTON_L1   10
#define PIN_BUTTON_R1   9
#define PIN_BUTTON_L2   14
#define PIN_BUTTON_R2   13
#define PIN_BUTTON_S1   5
#define PIN_BUTTON_S2   6
#define PIN_BUTTON_L3   21
#define PIN_BUTTON_R3   20
#define PIN_BUTTON_A1   4
#define PIN_BUTTON_A2   22

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 15
#define LED_BRIGHTNESS_MAXIMUM 200
#define LED_BRIGHTNESS_STEPS 5

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_DPAD_UP     3
#define LEDS_BUTTON_03   4
#define LEDS_BUTTON_04   5
#define LEDS_BUTTON_06   6
#define LEDS_BUTTON_05   7
#define LEDS_BUTTON_01   8
#define LEDS_BUTTON_02   9
#define LEDS_BUTTON_07   11
#define LEDS_BUTTON_08   10

// Hitbox layout

static std::vector<std::vector<Pixel>> pixels =
{
		{
			Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL, { 22, 23 }),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_DOWN, GAMEPAD_MASK_DD, { 20, 21 }),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_RIGHT, GAMEPAD_MASK_DR, { 18, 19 }),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			NO_PIXEL,
			NO_PIXEL,
			Pixel(LEDS_DPAD_UP,   GAMEPAD_MASK_DU, { 0, 1 }),
		},
		{
			Pixel(LEDS_BUTTON_03, GAMEPAD_MASK_B3, { 16, 17 }),
			Pixel(LEDS_BUTTON_01, GAMEPAD_MASK_B1, { 2, 3 }),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_04, GAMEPAD_MASK_B4, { 14, 15 }),
			Pixel(LEDS_BUTTON_02, GAMEPAD_MASK_B2, { 4, 5 }),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_06, GAMEPAD_MASK_R1, { 12, 13 }),
			Pixel(LEDS_BUTTON_08, GAMEPAD_MASK_R2, { 6, 7 }),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_05, GAMEPAD_MASK_L1, { 10, 11 }),
			Pixel(LEDS_BUTTON_07, GAMEPAD_MASK_L2, { 8, 9 }),
			NO_PIXEL,
		},
};

static PixelMatrix matrix(pixels);

#endif
