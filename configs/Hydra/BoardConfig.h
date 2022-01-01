/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef HYDRA_CONFIG_H_
#define HYDRA_CONFIG_H_

#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"

#define PIN_DPAD_DOWN   0
#define PIN_DPAD_UP     1
#define PIN_DPAD_LEFT   2
#define PIN_DPAD_RIGHT  3
#define PIN_BUTTON_B3   4
#define PIN_BUTTON_B4   5
#define PIN_BUTTON_R1   6
#define PIN_BUTTON_L1   7
#define PIN_BUTTON_B1   8
#define PIN_BUTTON_B2   9
#define PIN_BUTTON_R2   10
#define PIN_BUTTON_L2   11
#define PIN_BUTTON_S2   12
#define PIN_BUTTON_S1   13
#define PIN_BUTTON_R3   14
#define PIN_BUTTON_A1   15
#define PIN_BUTTON_A2   18
#define PIN_BUTTON_L3   19


#define DEFAULT_SOCD_MODE SOCD_MODE_UP_PRIORITY
#define BUTTON_LAYOUT BUTTON_LAYOUT_HITBOX

#define BOARD_LEDS_PIN 22

#define LED_BRIGHTNESS_MAXIMUM 200
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
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

// #define PLED_TYPE PLED_TYPE_PWM
// #define PLED1_PIN 16
// #define PLED2_PIN 17
// #define PLED3_PIN 18
// #define PLED4_PIN 19

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 20
#define I2C_SCL_PIN 21
#define I2C_BLOCK i2c0
#define I2C_SPEED 800000

#endif
