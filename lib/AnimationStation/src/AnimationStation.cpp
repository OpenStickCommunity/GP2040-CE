/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "AnimationStation.hpp"
#include "Effects/Chase.hpp"
#include "Effects/Rainbow.hpp"
#include "Effects/StaticColor.hpp"

uint8_t AnimationStation::brightness = 0;
float AnimationStation::brightnessMultiplier = 0;
absolute_time_t AnimationStation::nextBrightnessChange = 0;
absolute_time_t AnimationStation::nextAnimationChange = 0;

AnimationStation::AnimationStation(int numPixels) {
  this->numPixels = numPixels;
	AnimationStation::SetBrightness(BRIGHTNESS_SEGMENT);
}

void AnimationStation::HandleEvent(AnimationHotkey action) {
  if (action == HOTKEY_LEDS_BRIGHTNESS_UP) {
    AnimationStation::IncreaseBrightness();
  }

  if (action == HOTKEY_LEDS_BRIGHTNESS_DOWN) {
    AnimationStation::DecreaseBrightness();
  }

  if (action == HOTKEY_LEDS_ANIMATION_UP) {
    this->ChangeAnimation();
  }

  if (action == HOTKEY_LEDS_ANIMATION_DOWN) {
    this->ChangeAnimation();
  }
}

void AnimationStation::ChangeAnimation() {
  if (!time_reached(AnimationStation::nextAnimationChange)) {
    return;
  }

  if (this->animations.size() > 0) {
    switch (this->animations.at(0)->mode) {
    case STATIC:
      this->animations.push_back(new Rainbow());
      break;
    case RAINBOW:
      this->animations.push_back(new Chase());
      break;
    default:
      this->animations.push_back(new StaticColor());
      break;
    }

    this->animations.erase(this->animations.begin());
  }

  AnimationStation::nextAnimationChange = make_timeout_time_ms(250);
}

void AnimationStation::SetStaticColor() {
  this->animations.push_back(new StaticColor());
}

void AnimationStation::SetRainbow() {
  this->animations.push_back(new Rainbow());
}

void AnimationStation::SetChase() { this->animations.push_back(new Chase()); }

void AnimationStation::Animate() {
  if (this->animations.size() == 0) {
    this->Clear();
    return;
  }

  for (auto &element : this->animations) {
    /* non-base animations (eg. button presses) only run
      a certain number of times. before we animate, we need
      to verify that it isn't already complete */

    if (element->isComplete()) {
      this->animations.erase(std::remove(this->animations.begin(), this->animations.end(), element), this->animations.end());
    }
    else {
      element->Animate(this->frame);
    }
  }
}

void AnimationStation::Clear() {
  for (int i = 0; i < this->numPixels; ++i) {
    frame[i] = 0;
  }
}

float AnimationStation::GetBrightnessMultiplier() {
  return AnimationStation::brightnessMultiplier;
}

uint8_t AnimationStation::GetBrightness() {
  return AnimationStation::brightness;
}

void AnimationStation::SetBrightness(uint8_t brightness) {
  AnimationStation::brightness = brightness;

  AnimationStation::brightnessMultiplier = brightness / (float)BRIGHTNESS_MAXIMUM;
  if (AnimationStation::brightnessMultiplier > 1)
    AnimationStation::brightnessMultiplier = 1;
  else if (AnimationStation::brightnessMultiplier < 0)
    AnimationStation::brightnessMultiplier = 0;
}

void AnimationStation::DecreaseBrightness() {
  if (!time_reached(AnimationStation::nextBrightnessChange)) {
    return;
  }

  if (AnimationStation::brightness >= BRIGHTNESS_SEGMENT)
    AnimationStation::SetBrightness(AnimationStation::brightness - BRIGHTNESS_SEGMENT);

  AnimationStation::nextBrightnessChange = make_timeout_time_ms(250);
}

void AnimationStation::IncreaseBrightness() {
  if (!time_reached(AnimationStation::nextBrightnessChange)) {
    return;
  }

  if (AnimationStation::brightness < BRIGHTNESS_MAXIMUM)
    AnimationStation::SetBrightness(AnimationStation::brightness + BRIGHTNESS_SEGMENT);

  AnimationStation::nextBrightnessChange = make_timeout_time_ms(250);
}

uint32_t AnimationStation::RGB(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r * brightnessMultiplier) << 8) |
         ((uint32_t)(g * brightnessMultiplier) << 16) |
         (uint32_t)(b * brightnessMultiplier);
}

uint32_t AnimationStation::Wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return AnimationStation::RGB(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return AnimationStation::RGB(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return AnimationStation::RGB(pos * 3, 255 - pos * 3, 0);
  }
}
