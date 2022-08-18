/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include <GamepadEnums.h>

#define PIN_DPAD_UP     2
#define PIN_DPAD_DOWN   3
#define PIN_DPAD_RIGHT  4
#define PIN_DPAD_LEFT   5
#define PIN_BUTTON_B1   6
#define PIN_BUTTON_B2   7
#define PIN_BUTTON_R2   8
#define PIN_BUTTON_L2   9
#define PIN_BUTTON_B3   10
#define PIN_BUTTON_B4   11
#define PIN_BUTTON_R1   12
#define PIN_BUTTON_L1   13
#define PIN_BUTTON_S1   16
#define PIN_BUTTON_S2   17
#define PIN_BUTTON_L3   18
#define PIN_BUTTON_R3   19
#define PIN_BUTTON_A1   20
#define PIN_BUTTON_A2   21
#define PIN_BUTTON_TURBO 14
#define PIN_SLIDER_LS    26
#define PIN_SLIDER_RS    27

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define BUTTON_LAYOUT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX

#define SPLASH_MODE STATICSPLASH

#define TURBO_LED_PIN 15

#define BOARD_LEDS_PIN 28

#define LED_BRIGHTNESS_MAXIMUM 50
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

#define PLED_TYPE PLED_TYPE_NONE
#define PLED1_PIN -1
#define PLED2_PIN -1
#define PLED3_PIN -1
#define PLED4_PIN -1

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BLOCK i2c0
#define I2C_SPEED 400000

#endif
