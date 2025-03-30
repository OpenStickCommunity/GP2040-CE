/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "animationstation.h"
#include "animationstorage.h"

#include "storagemanager.h"

AnimationStation::AnimationStation() {
    brightnessMax = 100;
    brightnessSteps = 5;
    brightnessX = 0;
    linkageModeOfBrightnessX = 0;
    nextChange = nil_time;
    effectCount = TOTAL_EFFECTS;
    AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
    if (animationOptions.hasCustomTheme) {
        effectCount++; // increase our effect count
    }
    SetBrightness(1);
}

void AnimationStation::ConfigureBrightness(uint8_t max, uint8_t steps) {
  brightnessMax = max;
  brightnessSteps = steps;
}

void AnimationStation::HandleEvent(GamepadHotkey action) {
  if (action == HOTKEY_LEDS_NONE || !time_reached(nextChange) ) {
    return;
  }
  nextChange = make_timeout_time_ms(250);
  AnimationOptions hotkeyAnimationOptions = Storage::getInstance().getAnimationOptions();
  bool reqSave = false;

  if (action == HOTKEY_LEDS_BRIGHTNESS_UP) {
    IncreaseBrightness();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_BRIGHTNESS_DOWN) {
    DecreaseBrightness();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_ANIMATION_UP) {
    ChangeAnimation(1);
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_ANIMATION_DOWN) {
    ChangeAnimation(-1);
    reqSave = true;
  }

  if (this->baseAnimation == nullptr || this->buttonAnimation == nullptr) {
    return;
  }
  
  if (action == HOTKEY_LEDS_PARAMETER_UP) {
    this->baseAnimation->ParameterUp();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_PARAMETER_DOWN) {
    this->baseAnimation->ParameterDown();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_PRESS_PARAMETER_UP) {
    this->buttonAnimation->ParameterUp();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_PRESS_PARAMETER_DOWN) {
    this->buttonAnimation->ParameterDown();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_FADETIME_UP) {
    this->baseAnimation->FadeTimeUp();
    reqSave = true;
  }

  if (action == HOTKEY_LEDS_FADETIME_DOWN) {
    this->baseAnimation->FadeTimeDown();
    reqSave = true;
  }

	if (reqSave) {
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	}
}

void AnimationStation::ChangeAnimation(int changeSize) {
  this->SetMode(this->AdjustIndex(changeSize));
}

uint16_t AnimationStation::AdjustIndex(int changeSize) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();

  int newIndex = animationOptions.baseAnimationIndex + changeSize;

  if (newIndex >= effectCount) {
    return 0;
  }

  if (newIndex < 0) {
    return (effectCount - 1);
  }

  return (uint16_t)newIndex;
}

void AnimationStation::HandlePressed(std::vector<Pixel> pressed) {
  this->lastPressed = pressed;
  this->baseAnimation->UpdatePixels(pressed);  
  this->buttonAnimation->UpdatePixels(pressed);
}

void AnimationStation::ClearPressed() {
  if (this->buttonAnimation != nullptr) {
    this->buttonAnimation->ClearPixels();
  }
  if (this->baseAnimation != nullptr) {
    this->baseAnimation->ClearPixels();
  }

  this->lastPressed.clear();
}

void AnimationStation::Animate() {
  if (baseAnimation == nullptr || buttonAnimation == nullptr) {
    this->Clear();
    return;
  }

  // Only copy our frame to linkage frame if the animation effect updated our frame[]
  if ( baseAnimation->Animate(this->frame) == true ) {
    // Copy frame to linkage frame before button press
    for(int i = 0; i < 100; i++){
      linkageFrame[i] = this->frame[i];
    }
  }

  buttonAnimation->Animate(this->frame);
}

void AnimationStation::Clear() {
  memset(frame, 0, sizeof(frame));
}

float AnimationStation::GetBrightnessX() {
  return brightnessX;
}

float AnimationStation::GetLinkageModeOfBrightnessX() {
  return linkageModeOfBrightnessX;
}

uint8_t AnimationStation::GetBrightness() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  return animationOptions.brightness;
}

void AnimationStation::SetMode(uint8_t mode) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.baseAnimationIndex = mode;
  AnimationEffects newEffect =
      static_cast<AnimationEffects>(animationOptions.baseAnimationIndex);

  if (this->baseAnimation != nullptr) {
    delete this->baseAnimation;
  }
  if (this->buttonAnimation != nullptr) {
    delete this->buttonAnimation;
  }

  this->Clear();

  switch (newEffect) {
  case AnimationEffects::EFFECT_RAINBOW:
    this->baseAnimation = new Rainbow(matrix);
    this->buttonAnimation = new StaticColor(matrix, lastPressed);
    break;
  case AnimationEffects::EFFECT_CHASE:
    this->baseAnimation = new Chase(matrix);
    this->buttonAnimation = new StaticColor(matrix, lastPressed);
    break;
  case AnimationEffects::EFFECT_STATIC_THEME:
    this->baseAnimation = new StaticTheme(matrix);
    this->buttonAnimation = new StaticColor(matrix, lastPressed);
    break;
  case AnimationEffects::EFFECT_CUSTOM_THEME:
    this->baseAnimation = new CustomTheme(matrix);
    this->buttonAnimation = new CustomThemePressed(matrix, lastPressed);
    break;
  default:
    this->baseAnimation = new StaticColor(matrix);
    this->buttonAnimation = new StaticColor(matrix, lastPressed);
    break;
  }
}

void AnimationStation::SetMatrix(PixelMatrix matrix) {
  this->matrix = matrix;
}

void AnimationStation::ApplyBrightness(uint32_t *frameValue) {
  for (int i = 0; i < 100; i++)
    frameValue[i] = this->frame[i].value(Animation::format, brightnessX);
}

void AnimationStation::SetBrightness(uint8_t brightness) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.brightness =
      (brightness > brightnessSteps) ? brightnessSteps : brightness;
  brightnessX =
      (animationOptions.brightness * getBrightnessStepSize()) / 255.0F;
  linkageModeOfBrightnessX =
      (animationOptions.brightness * getLinkageModeOfBrightnessStepSize()) / 255.0F;
  if (linkageModeOfBrightnessX > 1)
      linkageModeOfBrightnessX = 1;
  else if (linkageModeOfBrightnessX < 0)
      linkageModeOfBrightnessX = 0;
  if (brightnessX > 1.0f)
    brightnessX = 1.0f;
  else if (brightnessX < 0.0f)
    brightnessX = 0.0f;
}

void AnimationStation::DecreaseBrightness() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.brightness > 0)
    SetBrightness(animationOptions.brightness-1);
}

void AnimationStation::IncreaseBrightness() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.brightness < getBrightnessStepSize())
    SetBrightness(animationOptions.brightness+1);
  else if (animationOptions.brightness > getBrightnessStepSize())
    SetBrightness(brightnessSteps);
}

void AnimationStation::DimBrightnessTo0() {
  brightnessX = 0;
}
