/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef DEBUG_BOARD_H_
#define DEBUG_BOARD_H_

#include "MPG.h"

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

#define LEDS_DPAD_UP     3
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_RIGHT  2
#define LEDS_BUTTON_01   8
#define LEDS_BUTTON_02   9
#define LEDS_BUTTON_03   4
#define LEDS_BUTTON_04   5
#define LEDS_BUTTON_05   7
#define LEDS_BUTTON_06   6
#define LEDS_BUTTON_07   11
#define LEDS_BUTTON_08   10
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 22
#define BOARD_LEDS_COUNT 16

#define LEDS_BASE_ANIMATION CHASE

#endif
