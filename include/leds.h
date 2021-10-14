/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <vector>
#include "AnimationStation.hpp"
#include "BoardConfig.h"
#include "gamepad.hpp"
#include "enums.h"

#define LED_BUTTON_COUNT 12

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

#ifndef LEDS_DPAD_LEFT
#define LEDS_DPAD_LEFT  -1
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

#ifndef LEDS_BUTTON_B3
#define LEDS_BUTTON_B3  -1
#endif

#ifndef LEDS_BUTTON_B4
#define LEDS_BUTTON_B4  -1
#endif

#ifndef LEDS_BUTTON_R1
#define LEDS_BUTTON_R1  -1
#endif

#ifndef LEDS_BUTTON_L1
#define LEDS_BUTTON_L1  -1
#endif

#ifndef LEDS_BUTTON_B1
#define LEDS_BUTTON_B1  -1
#endif

#ifndef LEDS_BUTTON_B2
#define LEDS_BUTTON_B2  -1
#endif

#ifndef LEDS_BUTTON_L2
#define LEDS_BUTTON_L2  -1
#endif

#ifndef LEDS_BUTTON_R2
#define LEDS_BUTTON_R2  -1
#endif

void configureAnimations(AnimationStation *as);
AnimationHotkey animationHotkeys(Gamepad *gamepad);
PixelMatrix createLedButtonLayout(LedLayout layout, int ledsPerPixel);
PixelMatrix createLedButtonLayout(LedLayout layout, std::vector<uint8_t> *positions);

extern PixelMatrix matrix;

class GPModule {
public:
  virtual void setup();
  virtual void loop();
  virtual void process(Gamepad *gamepad);
  absolute_time_t nextRunTime;
 	const uint32_t intervalMS = 10;
};
class LEDs : public GPModule {
public:
  void setup();
  void loop();
  void trySave();
  void process(Gamepad *gamepad);
	uint32_t frame[100];
};

#endif
