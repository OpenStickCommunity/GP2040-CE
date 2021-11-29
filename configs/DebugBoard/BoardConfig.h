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
#include "hardware/i2c.h"
#include "OneBitDisplay/OneBitDisplay.h"
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"

// Remaining pins (0, 1 reserved for I2C): 16

#define PIN_DPAD_UP    4
#define PIN_DPAD_DOWN  5
#define PIN_DPAD_LEFT  2
#define PIN_DPAD_RIGHT 3
#define PIN_BUTTON_B3  10
#define PIN_BUTTON_B4  12
#define PIN_BUTTON_R1  14
#define PIN_BUTTON_L1  15
#define PIN_BUTTON_B1  22
#define PIN_BUTTON_B2  21
#define PIN_BUTTON_R2  19
#define PIN_BUTTON_L2  17
#define PIN_BUTTON_S1  28
#define PIN_BUTTON_S2  27
#define PIN_BUTTON_L3  6
#define PIN_BUTTON_R3  8
#define PIN_BUTTON_A1  26
#define PIN_BUTTON_A2  9 // Not connected

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define BUTTON_LAYOUT BUTTON_LAYOUT_WASD

#define BOARD_LEDS_PIN 7

#define LED_BRIGHTNESS_MAXIMUM 50
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
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
#define PLED1_PIN 20
#define PLED2_PIN 11
#define PLED3_PIN 18
#define PLED4_PIN 13

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BLOCK i2c0
#define I2C_SPEED 800000

#endif
