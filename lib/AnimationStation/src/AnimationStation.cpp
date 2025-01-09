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
#include "Effects/SMPulseColour.hpp"
#include "Effects/SMCircleColour.hpp"
#include "Effects/SMWave.hpp"
#include "Effects/SMKnightRider.hpp"
#include "SpecialMoveSystem.hpp"

#include "AnimationStation.hpp"

uint8_t AnimationStation::brightnessMax = 100;
uint8_t AnimationStation::brightnessSteps = 5;
float AnimationStation::brightnessX = 0;
absolute_time_t AnimationStation::nextChange = nil_time;
AnimationOptions AnimationStation::options = {};
std::string AnimationStation::printfs[4];

AnimationStation::AnimationStation()
{
  AnimationStation::SetBrightness(1);

  //ensure valid profile set
  ChangeProfile(0);
}

void AnimationStation::SetLights(Lights InRGBLights)
{
  RGBLights = InRGBLights;
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

  //Switch to new profile
  if (action == HOTKEY_LEDS_SPECIALMOVE_PROFILE_UP) 
  {
    specialMoveSystem.ChangeSpecialMoveProfile(1);
  }
  if (action == HOTKEY_LEDS_SPECIALMOVE_PROFILE_DOWN) 
  {
    specialMoveSystem.ChangeSpecialMoveProfile(-1);
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
  if(options.NumValidProfiles == 0)
    return -1;

  //Check to see which ones are enabled. If none then return -1 to turn everything off
  for(int index = 0; index < MAX_ANIMATION_PROFILES; ++index)
  {
    if(options.profiles[index].bEnabled)
      validIndexes.push_back(index);
  }
  if(validIndexes.size() == 0)
    return -1;

  //find index of current profile
  int indexOfCurrentProfile = -1;
  for(unsigned int index = 0; index < validIndexes.size(); ++index)
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

  if (newProfileIndex >= (int)validIndexes.size())
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
    if(this->buttonAnimation)
      this->buttonAnimation->UpdatePressed(pressedPins);
  }
  else
  {
    this->lastPressed.clear();
    if(this->buttonAnimation)
      this->buttonAnimation->ClearPressed();
  }
}

void AnimationStation::HandlePressedButtons(uint32_t pressedButtons)
{
  specialMoveSystem.HandlePressedButtons(pressedButtons);
}

void AnimationStation::Animate() 
{
  //If no profiles running
  if (baseAnimation == nullptr || buttonAnimation == nullptr) 
  {
    this->Clear();
    return;
  }

  specialMoveSystem.Update();

  baseAnimation->Animate(this->frame);
  buttonAnimation->Animate(this->frame);
  if(specialMoveAnimation)
  {
    specialMoveAnimation->Animate(this->frame);
    //Special moves can end once their animation is over. Clean up if its finished
    if(specialMoveAnimation->IsFinished())
    {
      delete specialMoveAnimation;
      specialMoveAnimation = nullptr;
      specialMoveSystem.SetSpecialMoveAnimationOver();
    }
  }
}

void AnimationStation::Clear() 
{ 
  //sets all lights to black (off)
  memset(frame, 0, sizeof(frame)); 
}

void AnimationStation::SetSpecialMoveAnimation(SpecialMoveEffects AnimationToPlay, uint32_t OptionalParams)
{
    switch(AnimationToPlay)
    {
    case SpecialMoveEffects::SPECIALMOVE_SMEFFECT_WAVE:
        this->specialMoveAnimation = new SMWave(RGBLights);
        break;

    case SpecialMoveEffects::SPECIALMOVE_SMEFFECT_PULSECOLOR:
        this->specialMoveAnimation = new SMPulseColour(RGBLights);
        break;

    case SpecialMoveEffects::SPECIALMOVE_SMEFFECT_CIRCLECOLOR:
        this->specialMoveAnimation = new SMCircleColour(RGBLights);
        break;

    case SpecialMoveEffects::SPECIALMOVE_SMEFFECT_KNIGHTRIDER:
        this->specialMoveAnimation = new SMKnightRider(RGBLights);
        break;

    case SpecialMoveEffects::SPECIALMOVE_SMEFFECT_RANDOMFLASH:
        //this->specialMoveAnimation = new SMRandomFlash(RGBLights);
        break;

    default:
        break;
    }

    if(this->specialMoveAnimation)
    {
        this->specialMoveAnimation->SetOptionalParams(OptionalParams);
    }
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
    this->baseAnimation = nullptr;
  }
  if (this->buttonAnimation != nullptr) 
  {
    delete this->buttonAnimation;
    this->buttonAnimation = nullptr;
  }

  //turn off all lights
  this->Clear();

  //no profiles
  if(mode == -1)
    return; 

  //debug
  //this->options.profiles[this->options.baseProfileIndex].baseNonPressedEffect = NONPRESSED_EFFECT_CHASE_RANDOM;

  //set new profile nonpressed animation
  switch (this->options.profiles[this->options.baseProfileIndex].baseNonPressedEffect) 
  {
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_RAINBOW_SYNCED:
    this->baseAnimation = new RainbowSynced(RGBLights);
    break;

  case AnimationNonPressedEffects::NONPRESSED_EFFECT_RAINBOW_ROTATE:
    this->baseAnimation = new RainbowRotate(RGBLights);
    break;

  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_SEQUENTIAL:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_SEQUENTIAL);
    break;
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_LEFT_TO_RIGHT:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_LEFT_TO_RIGHT);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_RIGHT_TO_LEFT:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_RIGHT_TO_LEFT);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_TOP_TO_BOTTOM:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_TOP_TO_BOTTOM);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_BOTTOM_TO_TOP:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_BOTTOM_TO_TOP);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_SEQUENTIAL_PINGPONG:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_SEQUENTIAL_PINGPONG);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_HORIZONTAL_PINGPONG:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_HORIZONTAL_PINGPONG);
    break;
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_VERTICAL_PINGPONG:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_VERTICAL_PINGPONG);
    break;  
  case AnimationNonPressedEffects::NONPRESSED_EFFECT_CHASE_RANDOM:
    this->baseAnimation = new Chase(RGBLights, ChaseTypes::CHASETYPES_RANDOM);
    break; 

  case AnimationNonPressedEffects::NONPRESSED_EFFECT_STATIC_COLOR:
    this->baseAnimation = new StaticColor(RGBLights);
    break;

  default:
    break;
  }

  //set new profile pressed animation
  switch (this->options.profiles[this->options.baseProfileIndex].basePressedEffect) 
  {
  case AnimationPressedEffects::PRESSED_EFFECT_RANDOM:
    this->buttonAnimation = new RandomColor(RGBLights, lastPressed);
    break;

  case AnimationPressedEffects::PRESSED_EFFECT_STATIC_COLOR:
    this->buttonAnimation = new StaticColor(RGBLights, lastPressed);
    break;

  default:
    break;
  }
}

/*void AnimationStation::SetOptions(AnimationOptions InOptions) 
{
  options = InOptions;
  AnimationStation::SetBrightness(options.brightness);
}*/

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