/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef BENTOBOX_CONFIG_H_
#define BENTOBOX_CONFIG_H_

#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"

#define PIN_DPAD_UP     2
#define PIN_DPAD_DOWN   12
#define PIN_DPAD_RIGHT  11
#define PIN_DPAD_LEFT   13
#define PIN_BUTTON_B1   3
#define PIN_BUTTON_B2   4
#define PIN_BUTTON_R2   5
#define PIN_BUTTON_L2   6
#define PIN_BUTTON_B3   10
#define PIN_BUTTON_B4   9
#define PIN_BUTTON_R1   8
#define PIN_BUTTON_L1   7
#define PIN_BUTTON_S1   21
#define PIN_BUTTON_S2   22
#define PIN_BUTTON_L3   18
#define PIN_BUTTON_R3   17
#define PIN_BUTTON_A1   20
#define PIN_BUTTON_A2   19

#define DEFAULT_SOCD_MODE SOCD_MODE_SECOND_INPUT_PRIORITY
#define BUTTON_LAYOUT BUTTON_LAYOUT_HITBOX

#define BOARD_LEDS_PIN 15

#define LED_BRIGHTNESS_MAXIMUM 255
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 2

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_DPAD_UP     11
#define LEDS_BUTTON_B3   3
#define LEDS_BUTTON_B4   4
#define LEDS_BUTTON_R1   5
#define LEDS_BUTTON_L1   6
#define LEDS_BUTTON_B1   10
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_R2   8
#define LEDS_BUTTON_L2   7

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define I2C_BLOCK i2c1
#define I2C_SPEED 800000

#endif
