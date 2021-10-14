/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 *
 * Custom config for a modded Fightboard / Fightboard MX using an ItsyBitsy RP2040.
 *
 */

#ifndef FIGHTBOARD_CONFIG_H_
#define FIGHTBOARD_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Pixel.hpp"

#define PIN_DPAD_UP     11
#define PIN_DPAD_DOWN   9
#define PIN_DPAD_LEFT   10
#define PIN_DPAD_RIGHT  8
#define PIN_BUTTON_B1   20
#define PIN_BUTTON_B2   19
#define PIN_BUTTON_B3   24
#define PIN_BUTTON_B4   29
#define PIN_BUTTON_L1   27
#define PIN_BUTTON_R1   28
#define PIN_BUTTON_L2   25
#define PIN_BUTTON_R2   18
#define PIN_BUTTON_S1   6
#define PIN_BUTTON_S2   3
#define PIN_BUTTON_L3   7
#define PIN_BUTTON_R3   2
#define PIN_BUTTON_A1   14
#define PIN_BUTTON_A2   1

#define BOARD_LEDS_PIN 26
#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRBW

#define LEDS_BUTTON_03   0
#define LEDS_BUTTON_04   1
#define LEDS_BUTTON_06   2
#define LEDS_BUTTON_05   3
#define LEDS_BUTTON_07   4
#define LEDS_BUTTON_08   5
#define LEDS_BUTTON_02   6
#define LEDS_BUTTON_01   7
#define LEDS_DPAD_RIGHT  8
#define LEDS_DPAD_DOWN   9
#define LEDS_DPAD_LEFT   10
#define LEDS_DPAD_UP     11

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

static std::vector<std::vector<Pixel>> pixels =
{
		{
			NO_PIXEL,
			Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL),
		},
		{
			Pixel(LEDS_DPAD_UP,   GAMEPAD_MASK_DU),
			Pixel(LEDS_DPAD_DOWN, GAMEPAD_MASK_DD),
		},
		{
			NO_PIXEL,
			Pixel(LEDS_DPAD_RIGHT, GAMEPAD_MASK_DR),
		},
		{
			Pixel(LEDS_BUTTON_03, GAMEPAD_MASK_B3),
			Pixel(LEDS_BUTTON_01, GAMEPAD_MASK_B1),
		},
		{
			Pixel(LEDS_BUTTON_04, GAMEPAD_MASK_B4),
			Pixel(LEDS_BUTTON_02, GAMEPAD_MASK_B2),
		},
		{
			Pixel(LEDS_BUTTON_06, GAMEPAD_MASK_R1),
			Pixel(LEDS_BUTTON_08, GAMEPAD_MASK_R2),
		},
		{
			Pixel(LEDS_BUTTON_05, GAMEPAD_MASK_L1),
			Pixel(LEDS_BUTTON_07, GAMEPAD_MASK_L2),
		},
};

static PixelMatrix matrix(pixels, 1);

#endif
