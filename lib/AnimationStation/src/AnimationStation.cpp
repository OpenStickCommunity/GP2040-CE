/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Effects/Chase.hpp"
#include "Effects/Rainbow.hpp"
#include "Effects/StaticColor.hpp"
#include "Effects/RandomColor.hpp"

#include "AnimationStation.hpp"

uint8_t AnimationStation::brightnessMax = 100;
uint8_t AnimationStation::brightnessSteps = 5;
float AnimationStation::brightnessX = 0;
absolute_time_t AnimationStation::nextChange = nil_time;
AnimationOptions AnimationStation::options = {};

AnimationStation::AnimationStation()
{
  AnimationStation::SetBrightness(1);

  //ensure valid profile set
  ChangeProfile(0);
}

void AnimationStation::ConfigureBrightness(uint8_t max, uint8_t steps)
{
  brightnessMax = max;
  brightnessSteps = steps;
}

void AnimationStation::HandleEvent(AnimationHotkey action)
{
  if (action == HOTKEY_LEDS_NONE || !time_reached(AnimationStation::nextChange)) 
  {
    return;
  }
  AnimationStation::nextChange = make_timeout_time_ms(250);

  //Adjust brigness
  if (action == HOTKEY_LEDS_BRIGHTNESS_UP) 
  {
    AnimationStation::IncreaseBrightness();
  }
  if (action == HOTKEY_LEDS_BRIGHTNESS_DOWN) 
  {
    AnimationStation::DecreaseBrightness();
  }

  //Switch to new profile
  if (action == HOTKEY_LEDS_PROFILE_UP) 
  {
    ChangeProfile(1);
  }
  if (action == HOTKEY_LEDS_PROFILE_DOWN) 
  {
    ChangeProfile(-1);
  }

  //Adjust existing profile hotkeys
  if (this->baseAnimation == nullptr || this->buttonAnimation == nullptr) 
  {
    return;
  }

  if (action == HOTKEY_LEDS_PARAMETER_UP) 
  {
    this->baseAnimation->ParameterUp();
  }
  if (action == HOTKEY_LEDS_PARAMETER_DOWN) 
  {
    this->baseAnimation->ParameterDown();
  }
  if (action == HOTKEY_LEDS_PRESS_PARAMETER_UP) 
  {
    this->buttonAnimation->ParameterUp();
  }
  if (action == HOTKEY_LEDS_PRESS_PARAMETER_DOWN) 
  {
    this->buttonAnimation->ParameterDown();
  }  
}

void AnimationStation::ChangeProfile(int changeSize) 
{
  this->SetMode(this->AdjustIndex(changeSize));
}

uint16_t AnimationStation::AdjustIndex(int changeSize)
{
  std::vector<int> validIndexes;

  //if no profiles defined then return -1 to turn everything off
  for(int index = 0; index < MAX_ANIMATION_PROFILES; ++index)
  {
    if(options.profiles[index].bIsValidProfile)
      validIndexes.push_back(index);
  }

  if(validIndexes.size() == 0)
    return -1;

  //find index of current profile
  int indexOfCurrentProfile = 0;
  for(int index = 0; index < validIndexes.size(); ++index)
  {
    if(validIndexes[index] == (int)this->options.baseProfileIndex)
    {
      indexOfCurrentProfile = index;
      break;
    }
  }

  //if we cant find it then this is probably the first call and the first profile isnt valid. Just return whichever is the first valid profile
  if(indexOfCurrentProfile == -1)
    return validIndexes[0];

  int newProfileIndex = indexOfCurrentProfile + changeSize;

  if (newProfileIndex >= validIndexes.size())
  {
    return validIndexes[0];
  }

  if (newProfileIndex < 0) 
  {
    return validIndexes[validIndexes.size() - 1];
  }

  return (uint16_t)validIndexes[newProfileIndex];
}

void AnimationStation::HandlePressedPins(std::vector<int32_t> pressedPins)
{
  if(pressedPins.size())
  {
    this->lastPressed = pressedPins;
    this->buttonAnimation->UpdatePressed(pressedPins);
  }
  else
  {
    this->lastPressed.clear();
    this->buttonAnimation->ClearPressed();
  }
}

void AnimationStation::Animate() 
{
  //If no profiles running
  if (baseAnimation == nullptr || buttonAnimation == nullptr) 
  {
    this->Clear();
    return;
  }

  baseAnimation->Animate(this->frame);
  buttonAnimation->Animate(this->frame);
}

void AnimationStation::Clear() 
{ 
  //sets all lights to black (off)
  memset(frame, 0, sizeof(frame)); 
}

int8_t AnimationStation::GetMode() 
{ 
  return this->options.baseProfileIndex; 
}

void AnimationStation::SetMode(int8_t mode) 
{
  this->options.baseProfileIndex = mode;

  //remove old animations
  if (this->baseAnimation != nullptr) 
  {
    delete this->baseAnimation;
  }
  if (this->buttonAnimation != nullptr) 
  {
    delete this->buttonAnimation;
  }

  //turn off all lights
  this->Clear();

  //no profiles
  if(mode == -1)
    return; 

  //set new profile nonpressed animation
  switch ((AnimationNonPressedEffects)this->options.profiles[this->options.baseProfileIndex].baseNonPressedEffect) 
  {
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_RAINBOW_SYNCED:
    this->baseAnimation = new RainbowSynced(RGBLights);
    break;

  case AnimationNonPressedEffects::NONPRESSED_EFFECT_RAINBOW_ROTATE:
    this->baseAnimation = new RainbowRotate(RGBLights);
    break;

/*  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE:
    this->baseAnimation = new Chase(RGBLights);
    break;
*/
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_STATIC_COLOR:
    this->baseAnimation = new StaticColor(RGBLights);
    break;

  default:
    break;
  }

  //set new profile pressed animation
  switch ((AnimationPressedEffects)this->options.profiles[this->options.baseProfileIndex].basePressedEffect) 
  {
/*  case AnimationPressedEffects::PRESSED_EFFECT_RANDOM:
    this->buttonAnimation = new RandomColour(RGBLights, lastPressed);
*/
  case AnimationPressedEffects::PRESSED_EFFECT_STATIC_COLOR:
    this->buttonAnimation = new StaticColor(RGBLights, lastPressed);

  default:
    break;
  }
}

void AnimationStation::SetOptions(AnimationOptions options) 
{
  AnimationStation::options = options;
  AnimationStation::SetBrightness(options.brightness);
}

///////////////////////////////////
// Brightness functions
///////////////////////////////////

void AnimationStation::ApplyBrightness(uint32_t *frameValue) 
{
  for (int i = 0; i < 100; i++)
    frameValue[i] = this->frame[i].value(Animation::format, brightnessX);
}

void AnimationStation::SetBrightness(uint8_t brightness) 
{
  AnimationStation::options.brightness =
      (brightness > brightnessSteps) ? brightnessSteps : options.brightness;
  AnimationStation::brightnessX =
      (AnimationStation::options.brightness * getBrightnessStepSize()) / 255.0F;

  if (AnimationStation::brightnessX > 1)
    AnimationStation::brightnessX = 1;
  else if (AnimationStation::brightnessX < 0)
    AnimationStation::brightnessX = 0;
}

void AnimationStation::DecreaseBrightness() 
{
  if (AnimationStation::options.brightness > 0)
    AnimationStation::SetBrightness(--AnimationStation::options.brightness);
}

void AnimationStation::IncreaseBrightness()
{
  if (AnimationStation::options.brightness < getBrightnessStepSize())
    AnimationStation::SetBrightness(++AnimationStation::options.brightness);
  else if (AnimationStation::options.brightness > getBrightnessStepSize())
    AnimationStation::SetBrightness(brightnessSteps);
}

void AnimationStation::DimBrightnessTo0() 
{
  AnimationStation::brightnessX = 0;
}

float AnimationStation::GetBrightnessX() 
{
  return AnimationStation::brightnessX;
}

uint8_t AnimationStation::GetBrightness() 
{
  return AnimationStation::options.brightness;
}