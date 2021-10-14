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
#include "enums.h"

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

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 26

#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRBW
#define LED_LAYOUT LED_LAYOUT_ARCADE_WASD
#define LEDS_PER_PIXEL 1

#define LEDS_DPAD_LEFT  10
#define LEDS_DPAD_DOWN   9
#define LEDS_DPAD_RIGHT  8
#define LEDS_DPAD_UP    11
#define LEDS_BUTTON_B3   0
#define LEDS_BUTTON_B4   1
#define LEDS_BUTTON_R1   2
#define LEDS_BUTTON_L1   3
#define LEDS_BUTTON_B1   7
#define LEDS_BUTTON_B2   6
#define LEDS_BUTTON_R2   5
#define LEDS_BUTTON_L2   4

#endif
