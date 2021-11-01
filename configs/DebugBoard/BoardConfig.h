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

#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"

/* Mapped to pins on Waveshare IPS panel w/4 button + HAT */

#define PIN_DPAD_UP     2
#define PIN_DPAD_DOWN   18
#define PIN_DPAD_LEFT   16
#define PIN_DPAD_RIGHT  20
#define PIN_BUTTON_B1   15
#define PIN_BUTTON_B2   17
#define PIN_BUTTON_B3   19
#define PIN_BUTTON_S1   21
#define PIN_BUTTON_S2   3

/* Map other buttons to remaining open pins */

#define PIN_BUTTON_B4   4
#define PIN_BUTTON_L1   5
#define PIN_BUTTON_R1   6
#define PIN_BUTTON_L2   7
#define PIN_BUTTON_R2   8
#define PIN_BUTTON_L3   9
#define PIN_BUTTON_R3   14
#define PIN_BUTTON_A1   26
#define PIN_BUTTON_A2   27

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 22

#define LED_BRIGHTNESS_MAXIMUM 50
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT LED_LAYOUT_ARCADE_WASD
#define LEDS_PER_PIXEL 4

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

#define PLED_TYPE PLED_TYPE_PWM
#define PLED1_PIN 10
#define PLED2_PIN 11
#define PLED3_PIN 12
#define PLED4_PIN 13

#endif
