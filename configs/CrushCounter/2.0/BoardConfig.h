/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <GamepadEnums.h>
#include "enums.h"
#include "NeoPico.hpp"
#include "PlayerLEDs.h"

#define PIN_DPAD_UP     16
#define PIN_DPAD_DOWN   28
#define PIN_DPAD_LEFT   18
#define PIN_DPAD_RIGHT  29
#define PIN_BUTTON_B1   8 // K1
#define PIN_BUTTON_B2   10 // K2
#define PIN_BUTTON_B3   7 // P1
#define PIN_BUTTON_B4   9  // P2
#define PIN_BUTTON_L1   13 // P4
#define PIN_BUTTON_R1   11 // P3
#define PIN_BUTTON_L2   14 // K4
#define PIN_BUTTON_R2   12 // K3
#define PIN_BUTTON_S1   20  // Select
#define PIN_BUTTON_S2   19  // Start
#define PIN_BUTTON_L3   23  // L3
#define PIN_BUTTON_R3   24  // R3
#define PIN_BUTTON_A1   21  // Home
#define PIN_BUTTON_A2   22  // Capture
#define PIN_SETTINGS    25
#define BOARD_LEDS_PIN 17
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

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define BUTTON_LAYOUT BUTTON_LAYOUT_HITBOX

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define I2C_BLOCK i2c1
#define DISPLAY_I2C_ADDR 0x3C

#define I2C_SPEED 800000

#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1

