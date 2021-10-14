/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef OSFRD_BOARD_CONFIG_H_
#define OSFRD_BOARD_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Pixel.hpp"

#define PIN_DPAD_UP     13
#define PIN_DPAD_DOWN   11
#define PIN_DPAD_LEFT   10
#define PIN_DPAD_RIGHT  12
#define PIN_BUTTON_B1   4
#define PIN_BUTTON_B2   5
#define PIN_BUTTON_B3   0
#define PIN_BUTTON_B4   1
#define PIN_BUTTON_L1   3
#define PIN_BUTTON_R1   2
#define PIN_BUTTON_L2   7
#define PIN_BUTTON_R2   6
#define PIN_BUTTON_S1   8
#define PIN_BUTTON_S2   9
#define PIN_BUTTON_L3   17
#define PIN_BUTTON_R3   16
#define PIN_BUTTON_A1   28
#define PIN_BUTTON_A2   18

#define BOARD_LEDS_PIN 14
#define LED_BRIGHTNESS_MAXIMUM 100
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

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

static std::vector<std::vector<Pixel>> pixels =
{
		{
			Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_DOWN, GAMEPAD_MASK_DD),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_RIGHT, GAMEPAD_MASK_DR),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			NO_PIXEL,
			NO_PIXEL,
			Pixel(LEDS_DPAD_UP,   GAMEPAD_MASK_DU),
		},
		{
			Pixel(LEDS_BUTTON_03, GAMEPAD_MASK_B3),
			Pixel(LEDS_BUTTON_01, GAMEPAD_MASK_B1),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_04, GAMEPAD_MASK_B4),
			Pixel(LEDS_BUTTON_02, GAMEPAD_MASK_B2),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_06, GAMEPAD_MASK_R1),
			Pixel(LEDS_BUTTON_08, GAMEPAD_MASK_R2),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_05, GAMEPAD_MASK_L1),
			Pixel(LEDS_BUTTON_07, GAMEPAD_MASK_L2),
			NO_PIXEL,
		},
};

static PixelMatrix matrix(pixels, 1);

#endif
