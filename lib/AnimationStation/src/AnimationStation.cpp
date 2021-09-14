/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "AnimationStation.hpp"

uint8_t AnimationStation::brightness = 0;
float AnimationStation::brightnessX = 0;
absolute_time_t AnimationStation::nextAnimationChange = 0;
absolute_time_t AnimationStation::nextBrightnessChange = 0;
StaticColor *staticColor;

AnimationStation::AnimationStation(std::vector<Pixel> pixels) : pixels(pixels) {
  AnimationStation::SetBrightness(1);

  staticColor = new StaticColor(pixels, ColorWhite);

  animations.push_back(new StaticColor(pixels, ColorBlack));
  animations.push_back(staticColor);
  animations.push_back(new Rainbow(pixels));
  animations.push_back(new Chase(pixels));
}

void AnimationStation::AddAnimation(Animation *animation) {
  animations.push_back(animation);
}

void AnimationStation::HandleEvent(AnimationHotkey action) {
  if (action == HOTKEY_LEDS_BRIGHTNESS_UP) {
    AnimationStation::IncreaseBrightness();
  }

  if (action == HOTKEY_LEDS_BRIGHTNESS_DOWN) {
    AnimationStation::DecreaseBrightness();
  }

  if (action == HOTKEY_LEDS_ANIMATION_UP) {
    ChangeAnimation();
  }

  if (action == HOTKEY_LEDS_ANIMATION_DOWN) {
    ChangeAnimation();
  }
}

void AnimationStation::ChangeAnimation() {
  if (!time_reached(AnimationStation::nextAnimationChange)) {
    return;
  }

  animationIndex = (animationIndex + 1) % animations.size();
  AnimationStation::nextAnimationChange = make_timeout_time_ms(250);
}

void AnimationStation::Animate() {
  if (animations.size() == 0) {
    this->Clear();
    return;
  }

  Animation *animation = animations[animationIndex];
  if (!animation->isComplete()) {
    animation->Animate(frame);
  }
}

void AnimationStation::Clear() {
  memset(frame, 0, sizeof(frame));
}

float AnimationStation::GetBrightnessX() {
  return AnimationStation::brightnessX;
}

uint8_t AnimationStation::GetBrightness() {
  return AnimationStation::brightness;
}

uint8_t AnimationStation::GetMode() {
  return animationIndex;
}

void AnimationStation::SetMode(uint8_t mode) {
  animationIndex = mode;
}

void AnimationStation::ApplyBrightness(uint32_t *frameValue) {
  for (int i = 0; i < 100; i++)
    frameValue[i] = this->frame[i].value(brightnessX);
}

void AnimationStation::SetBrightness(uint8_t brightness) {
  AnimationStation::brightness = (brightness > BRIGHTNESS_STEPS) ? BRIGHTNESS_STEPS : brightness;
  AnimationStation::brightnessX = (AnimationStation::brightness * BRIGHTNESS_STEP_SIZE) / 255.0F;

  if (AnimationStation::brightnessX > 1)
    AnimationStation::brightnessX = 1;
  else if (AnimationStation::brightnessX < 0)
    AnimationStation::brightnessX = 0;
}

void AnimationStation::SetStaticColor(RGB color) {
  staticColor->SetColor(color);
}

void AnimationStation::DecreaseBrightness() {
  if (!time_reached(AnimationStation::nextBrightnessChange))
    return;

  if (AnimationStation::brightness > 0)
    AnimationStation::SetBrightness(--AnimationStation::brightness);

  AnimationStation::nextBrightnessChange = make_timeout_time_ms(250);
}

void AnimationStation::IncreaseBrightness() {
  if (!time_reached(AnimationStation::nextBrightnessChange))
    return;

  if (AnimationStation::brightness < BRIGHTNESS_STEP_SIZE)
    AnimationStation::SetBrightness(++AnimationStation::brightness);
  else if (AnimationStation::brightness > BRIGHTNESS_STEP_SIZE)
    AnimationStation::SetBrightness(BRIGHTNESS_STEPS);

  AnimationStation::nextBrightnessChange = make_timeout_time_ms(250);
}
