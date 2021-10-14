/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 *
 * Preset config for Pico with a GeeekPi Pico Screw Terminal Expansion Board:
 * https://www.amazon.com/gp/product/B0998594PL/
 *
 */

#ifndef GEEEK_PI_STICK_CONFIG_H_
#define GEEEK_PI_STICK_CONFIG_H_

#include <vector>
#include <GamepadEnums.h>
#include <GamepadState.h>
#include "Pixel.hpp"

#define PIN_DPAD_DOWN    4
#define PIN_DPAD_UP      5
#define PIN_DPAD_LEFT    6
#define PIN_DPAD_RIGHT   7

#define PIN_BUTTON_B1    8
#define PIN_BUTTON_B2    9
#define PIN_BUTTON_R2   10
#define PIN_BUTTON_L2   11
#define PIN_BUTTON_B3   12
#define PIN_BUTTON_B4   13
#define PIN_BUTTON_R1   14
#define PIN_BUTTON_L1   15

#define PIN_BUTTON_S1   17
#define PIN_BUTTON_S2   18
#define PIN_BUTTON_L3   19
#define PIN_BUTTON_R3   20
#define PIN_BUTTON_A1   21
#define PIN_BUTTON_A2   22

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL

#endif
