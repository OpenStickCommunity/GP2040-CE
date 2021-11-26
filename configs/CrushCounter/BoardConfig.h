/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef CRUSH_COUNTER_BOARD_CONFIG_H_
#define CRUSH_COUNTER_BOARD_CONFIG_H_

#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"


#define PIN_DPAD_UP     20
#define PIN_DPAD_DOWN   8
#define PIN_DPAD_LEFT   1
#define PIN_DPAD_RIGHT  14
#define PIN_BUTTON_B1   18
#define PIN_BUTTON_B2   17
#define PIN_BUTTON_B3   13
#define PIN_BUTTON_B4   9
#define PIN_BUTTON_L1   12
#define PIN_BUTTON_R1   10
#define PIN_BUTTON_L2   19
#define PIN_BUTTON_R2   16
#define PIN_BUTTON_S1   3
#define PIN_BUTTON_S2   0
#define PIN_BUTTON_L3   6
#define PIN_BUTTON_R3   7
#define PIN_BUTTON_A1   4
#define PIN_BUTTON_A2   5
#define PIN_SETTINGS    11

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define BUTTON_LAYOUT BUTTON_LAYOUT_HITBOX

#define BOARD_LEDS_PIN 2

#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_DPAD_UP     3
#define LEDS_BUTTON_B3   4
#define LEDS_BUTTON_B4   5
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_L1   7
#define LEDS_BUTTON_B1   8
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_R2   10
#define LEDS_BUTTON_L2   11

#define PLED_TYPE PLED_TYPE_RGB
#define PLED1_PIN 12
#define PLED2_PIN 13
#define PLED3_PIN 14
#define PLED4_PIN 15

#endif
