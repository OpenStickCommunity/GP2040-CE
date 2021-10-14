/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_FIGHTING_BOARD_CONFIG_H_
#define PICO_FIGHTING_BOARD_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"

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
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT LED_LAYOUT_ARCADE_HITBOX
#define LEDS_PER_PIXEL 2

#define LEDS_DPAD_LEFT   11
#define LEDS_DPAD_DOWN   10
#define LEDS_DPAD_RIGHT  9
#define LEDS_DPAD_UP     0
#define LEDS_BUTTON_B3   8
#define LEDS_BUTTON_B4   7
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_L1   5
#define LEDS_BUTTON_B1   1
#define LEDS_BUTTON_B2   2
#define LEDS_BUTTON_R2   3
#define LEDS_BUTTON_L2   4

#endif
