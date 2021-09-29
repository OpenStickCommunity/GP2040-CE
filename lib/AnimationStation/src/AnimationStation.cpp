/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "AnimationStation.hpp"

uint8_t AnimationStation::brightnessMax = 100;
uint8_t AnimationStation::brightnessSteps = 5;
uint8_t AnimationStation::brightness = 0;
float AnimationStation::brightnessX = 0;
absolute_time_t AnimationStation::nextAnimationChange = 0;
absolute_time_t AnimationStation::nextBrightnessChange = 0;
StaticColor *staticColor;

AnimationStation::AnimationStation(std::vector<Pixel> pixels) : pixels(pixels) {
  AnimationStation::SetBrightness(1);
}

void AnimationStation::ConfigureBrightness(uint8_t max, uint8_t steps) {
  brightnessMax = max;
  brightnessSteps = steps;
}

void AnimationStation::HandleEvent(AnimationHotkey action) {
  if (action == HOTKEY_LEDS_BRIGHTNESS_UP) {
    AnimationStation::IncreaseBrightness();
  }

  if (action == HOTKEY_LEDS_BRIGHTNESS_DOWN) {
    AnimationStation::DecreaseBrightness();
  }

  if (action == HOTKEY_LEDS_ANIMATION_UP) {
    ChangeAnimation(1);
  }

  if (action == HOTKEY_LEDS_ANIMATION_DOWN) {
    ChangeAnimation(-1);
  }
}

void AnimationStation::ChangeAnimation(int changeSize) {
  if (!time_reached(AnimationStation::nextAnimationChange)) {
    return;
  }

  this->SetMode(this->AdjustIndex(changeSize));
  AnimationStation::nextAnimationChange = make_timeout_time_ms(250);
}

uint16_t AnimationStation::AdjustIndex(int changeSize) {
  uint16_t newIndex = this->baseAnimationIndex + changeSize;

  if (newIndex >= TOTAL_EFFECTS) {
    return 0;
  }

  if (newIndex < 0) {
    return (TOTAL_EFFECTS - 1);
  }

  return newIndex;
}

void AnimationStation::HandlePressed(std::vector<Pixel> pressed) {
  this->buttonAnimation = new StaticColor(pressed, true);
}

void AnimationStation::ClearPressed() {
  this->buttonAnimation = nullptr;
}

void AnimationStation::Animate() {
  if (baseAnimation == nullptr) {
    this->Clear();
    return;
  }

  baseAnimation->Animate(this->frame);

  if (buttonAnimation != nullptr) {
    buttonAnimation->Animate(this->frame);
  }
}

void AnimationStation::Clear() { memset(frame, 0, sizeof(frame)); }

float AnimationStation::GetBrightnessX() {
  return AnimationStation::brightnessX;
}

uint8_t AnimationStation::GetBrightness() {
  return AnimationStation::brightness;
}

uint8_t AnimationStation::GetMode() { return this->baseAnimationIndex; }

void AnimationStation::SetMode(uint8_t mode) {
  this->baseAnimationIndex = mode;
  AnimationEffects newEffect =
      static_cast<AnimationEffects>(this->baseAnimationIndex);

  switch (newEffect) {
  case AnimationEffects::EFFECT_STATIC_COLOR:
    this->baseAnimation = new StaticColor(pixels);
    break;
  case AnimationEffects::EFFECT_RAINBOW:
    this->baseAnimation = new Rainbow(pixels);
    break;
  case AnimationEffects::EFFECT_CHASE:
    this->baseAnimation = new Chase(pixels);
    break;
  default:
    this->baseAnimation = new StaticColor(pixels, ColorBlack);
    break;
  }
}

void AnimationStation::ApplyBrightness(uint32_t *frameValue) {
  for (int i = 0; i < 100; i++)
    frameValue[i] = this->frame[i].value(brightnessX);
}

void AnimationStation::SetBrightness(uint8_t brightness) {
  AnimationStation::brightness =
      (brightness > brightnessSteps) ? brightnessSteps : brightness;
  AnimationStation::brightnessX =
      (AnimationStation::brightness * getBrightnessStepSize()) / 255.0F;

  if (AnimationStation::brightnessX > 1)
    AnimationStation::brightnessX = 1;
  else if (AnimationStation::brightnessX < 0)
    AnimationStation::brightnessX = 0;
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

  if (AnimationStation::brightness < getBrightnessStepSize())
    AnimationStation::SetBrightness(++AnimationStation::brightness);
  else if (AnimationStation::brightness > getBrightnessStepSize())
    AnimationStation::SetBrightness(brightnessSteps);

  AnimationStation::nextBrightnessChange = make_timeout_time_ms(250);
}
