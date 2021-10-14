/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 *
 * Debug setup
 * ------------------------------------------------------------------------------------
 * 2x Raspberry Pi Pico (2nd is for picoprobe debugging)
 * 1x Waveshare 1.3inch LCD Display Module - https://www.waveshare.com/pico-lcd-1.3.htm
 * 1x Waveshare RGB 16x10 LED Matrix - https://www.waveshare.com/pico-rgb-led.htm
 * 1x Waveshare Quad GPIO Expander - https://www.waveshare.com/pico-quad-expander.htm
 *
 */

#ifndef DEBUG_BOARD_CONFIG_H_
#define DEBUG_BOARD_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Animation.hpp"

/* Mapped to pins on Waveshare IPS panel w/4 button + HAT */

#define PIN_DPAD_UP     2
#define PIN_DPAD_DOWN   18
#define PIN_DPAD_LEFT   16
#define PIN_DPAD_RIGHT  20
#define PIN_BUTTON_B1   15
#define PIN_BUTTON_B2   17
#define PIN_BUTTON_S1   19
#define PIN_BUTTON_S2   21
#define PIN_BUTTON_R3   3

/* Map other buttons to remaining open pins */

#define PIN_BUTTON_B3   4
#define PIN_BUTTON_B4   5
#define PIN_BUTTON_L1   6
#define PIN_BUTTON_R1   7
#define PIN_BUTTON_L2   14
#define PIN_BUTTON_R2   26
#define PIN_BUTTON_L3   27
#define PIN_BUTTON_A1   28
#define PIN_BUTTON_A2   0

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 22
#define LED_BRIGHTNESS_MAXIMUM 50
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
#define LEDS_BUTTON_08   10
#define LEDS_BUTTON_07   11

#define LEDS_PER_PIXEL 4

// WASD layout

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

static PixelMatrix matrix(pixels, LEDS_PER_PIXEL);

#endif
