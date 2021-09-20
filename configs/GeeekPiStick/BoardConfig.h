/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 *
 * Preset config for Pico with a GeeekPi Pico Screw Terminal Expansion Board:
 * https://www.amazon.com/gp/product/B0998594PL/
 *
 */

#ifndef GEEEK_PI_STICK_CONFIG_H_
#define GEEEK_PI_STICK_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Pixel.hpp"

#define PIN_DPAD_DOWN    4
#define PIN_DPAD_UP      5
#define PIN_DPAD_LEFT    6
#define PIN_DPAD_RIGHT   7

#define PIN_BUTTON_B1    8
#define PIN_BUTTON_B2    9
#define PIN_BUTTON_R2   10
#define PIN_BUTTON_L2   11
#define PIN_BUTTON_B3   12
#define PIN_BUTTON_B4   13
#define PIN_BUTTON_R1   14
#define PIN_BUTTON_L1   15

#define PIN_BUTTON_S1   17
#define PIN_BUTTON_S2   18
#define PIN_BUTTON_L3   19
#define PIN_BUTTON_R3   20
#define PIN_BUTTON_A1   21
#define PIN_BUTTON_A2   22

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN  28
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


const static std::vector<Pixel> pixels =
{
	{ .index = LEDS_DPAD_LEFT,  .mask = GAMEPAD_MASK_DL, .positions = { 22, 23 } },
	{ .index = LEDS_DPAD_DOWN,  .mask = GAMEPAD_MASK_DD, .positions = { 20, 21 } },
	{ .index = LEDS_DPAD_RIGHT, .mask = GAMEPAD_MASK_DR, .positions = { 18, 19 } },
	{ .index = LEDS_DPAD_UP,    .mask = GAMEPAD_MASK_DU, .positions = { 0, 1 } },
	{ .index = LEDS_BUTTON_03,  .mask = GAMEPAD_MASK_B3, .positions = { 16, 17 } },
	{ .index = LEDS_BUTTON_04,  .mask = GAMEPAD_MASK_B4, .positions = { 14, 15 } },
	{ .index = LEDS_BUTTON_06,  .mask = GAMEPAD_MASK_R1, .positions = { 12, 13 } },
	{ .index = LEDS_BUTTON_05,  .mask = GAMEPAD_MASK_L1, .positions = { 10, 11 } },
	{ .index = LEDS_BUTTON_01,  .mask = GAMEPAD_MASK_B1, .positions = { 2, 3 } },
	{ .index = LEDS_BUTTON_02,  .mask = GAMEPAD_MASK_B2, .positions = { 4, 5 } },
	{ .index = LEDS_BUTTON_08,  .mask = GAMEPAD_MASK_R2, .positions = { 6, 7 } },
	{ .index = LEDS_BUTTON_07,  .mask = GAMEPAD_MASK_L2, .positions = { 8, 9 } },
};

#endif
