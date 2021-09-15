/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <vector>
#include <MPG.h>
#include "AnimationStation.hpp"

#ifndef LEDS_DPAD_LEFT
#define LEDS_DPAD_LEFT -1
#endif

#ifndef LEDS_DPAD_DOWN
#define LEDS_DPAD_DOWN  -1
#endif

#ifndef LEDS_DPAD_RIGHT
#define LEDS_DPAD_RIGHT -1
#endif

#ifndef LEDS_DPAD_UP
#define LEDS_DPAD_UP    -1
#endif

#ifndef LEDS_BUTTON_01
#define LEDS_BUTTON_01  -1
#endif

#ifndef LEDS_BUTTON_02
#define LEDS_BUTTON_02  -1
#endif

#ifndef LEDS_BUTTON_03
#define LEDS_BUTTON_03  -1
#endif

#ifndef LEDS_BUTTON_04
#define LEDS_BUTTON_04  -1
#endif

#ifndef LEDS_BUTTON_05
#define LEDS_BUTTON_05  -1
#endif

#ifndef LEDS_BUTTON_06
#define LEDS_BUTTON_06  -1
#endif

#ifndef LEDS_BUTTON_07
#define LEDS_BUTTON_07  -1
#endif

#ifndef LEDS_BUTTON_08
#define LEDS_BUTTON_08  -1
#endif

#ifndef LEDS_BUTTON_09
#define LEDS_BUTTON_09  -1
#endif

#ifndef LEDS_BUTTON_10
#define LEDS_BUTTON_10  -1
#endif

#ifndef LEDS_BUTTON_11
#define LEDS_BUTTON_11  -1
#endif

#ifndef LEDS_BUTTON_12
#define LEDS_BUTTON_12  -1
#endif

#ifndef LEDS_BUTTON_13
#define LEDS_BUTTON_13  -1
#endif

#ifndef LEDS_BUTTON_14
#define LEDS_BUTTON_14  -1
#endif

#ifndef LEDS_BRIGHTNESS
#define LEDS_BRIGHTNESS 75
#endif

#ifndef LEDS_RAINBOW_CYCLE_TIME
#define LEDS_RAINBOW_CYCLE_TIME 40
#endif

#ifndef LEDS_CHASE_CYCLE_TIME
#define LEDS_CHASE_CYCLE_TIME 85
#endif

#ifndef LEDS_STATIC_COLOR_COLOR
#define LEDS_STATIC_COLOR_COLOR ColorRed
#endif

extern const std::vector<Pixel> pixels;

void configureAnimations(AnimationStation *as);
AnimationHotkey animationHotkeys(MPG *gamepad);

#endif
