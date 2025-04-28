/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "effects/chase.h"
#include "effects/rain.h"
#include "effects/rainbow.h"
#include "effects/staticcolor.h"
#include "effects/jigglestaticcolor.h"
#include "effects/burstcolor.h"
#include "effects/jiggletwostaticcolor.h"
#include "effects/randomcolor.h"
#include "effects/smpulsecolor.h"
#include "effects/smcirclecolor.h"
#include "effects/smwave.h"
#include "effects/smknightrider.h"
#include "specialmovesystem.h"

#include "animationstation.h"

#include "storagemanager.h"

#include "enums.pb.h"
#include "config.pb.h"

uint8_t AnimationStation::brightnessMax = 100;
uint8_t AnimationStation::brightnessSteps = 5;
float AnimationStation::brightnessX = 0;
absolute_time_t AnimationStation::nextChange = nil_time;
AnimationOptions_Unpacked AnimationStation::options = {};
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

void AnimationStation::HandleEvent(GamepadHotkey action)
{
  if (action == HOTKEY_LEDS_NONE)
  {
    AnimationStation::nextChange = nil_time;
    return;
  } 
  else if(!time_reached(AnimationStation::nextChange)) 
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
    this->buttonAnimation->PressParameterUp();
  }
  if (action == HOTKEY_LEDS_PRESS_PARAMETER_DOWN) 
  {
    this->buttonAnimation->PressParameterDown();
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

  //if we cant find it then either we're in the off state or this is probably the first call and the first profile isnt valid. 
  if(indexOfCurrentProfile == -1)
  {
    if(changeSize >= 0)
      return validIndexes[0];
    else
      return validIndexes[validIndexes.size() - 1];
  }

  int newProfileIndex = indexOfCurrentProfile + changeSize;

  //if we're going to wrap around then move to "OFF" profile
  if (newProfileIndex >= (int)validIndexes.size())
  {
    return -1;
  }
  else if (newProfileIndex < 0) 
  {
    return -1;
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
  if(caseAnimation != nullptr)
    caseAnimation->Animate(this->frame);
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

  checkForOptionsUpdate();
  specialMoveSystem.checkForOptionsUpdate();
}

void AnimationStation::Clear() 
{ 
  //sets all lights to black (off)
  memset(frame, 0, sizeof(frame)); 
}

void AnimationStation::SetSpecialMoveAnimation(SpecialMoveEffects AnimationToPlay, uint32_t OptionalParams)
{
  EButtonCaseEffectType buttonCaseEffect = EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY;
  if(this->options.profiles[this->options.baseProfileIndex].bUseCaseLightsInSpecialMoves)
    buttonCaseEffect = EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE;

  switch(AnimationToPlay)
  {
  case SpecialMoveEffects::SpecialMoveEffects_SMEFFECT_WAVE:
    this->specialMoveAnimation = new SMWave(RGBLights, buttonCaseEffect);
    break;

  case SpecialMoveEffects::SpecialMoveEffects_SMEFFECT_PULSECOLOR:
    this->specialMoveAnimation = new SMPulseColor(RGBLights, buttonCaseEffect);
    break;

  case SpecialMoveEffects::SpecialMoveEffects_SMEFFECT_CIRCLECOLOR:
    this->specialMoveAnimation = new SMCircleColor(RGBLights, buttonCaseEffect);
    break;

  case SpecialMoveEffects::SpecialMoveEffects_SMEFFECT_KNIGHTRIDER:
    this->specialMoveAnimation = new SMKnightRider(RGBLights, buttonCaseEffect);
    break;

  case SpecialMoveEffects::SpecialMoveEffects_SMEFFECT_RANDOMFLASH:
    //this->specialMoveAnimation = new SMRandomFlash(RGBLights, buttonCaseEffect);
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

Animation* AnimationStation::GetNonPressedEffectForEffectType(AnimationNonPressedEffects EffectType, EButtonCaseEffectType InButtonCaseEffectType)
{
  Animation* newEffect = nullptr;

  switch (EffectType) 
  {
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAINBOW_SYNCED:
    newEffect = new RainbowSynced(RGBLights, InButtonCaseEffectType);
    break;

  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAINBOW_ROTATE:
    newEffect = new RainbowRotate(RGBLights, InButtonCaseEffectType);
    break;

  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_SEQUENTIAL:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_SEQUENTIAL);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_LEFT_TO_RIGHT:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_LEFT_TO_RIGHT);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_RIGHT_TO_LEFT:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_RIGHT_TO_LEFT);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_TOP_TO_BOTTOM:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_TOP_TO_BOTTOM);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_BOTTOM_TO_TOP:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_BOTTOM_TO_TOP);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_SEQUENTIAL_PINGPONG:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_SEQUENTIAL_PINGPONG);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_HORIZONTAL_PINGPONG:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_HORIZONTAL_PINGPONG);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_VERTICAL_PINGPONG:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_VERTICAL_PINGPONG);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_RANDOM:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_RANDOM);
    break; 

  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_STATIC_COLOR:
    newEffect = new StaticColor(RGBLights, InButtonCaseEffectType);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_JIGGLESTATIC:
    newEffect = new JiggleStaticColor(RGBLights, InButtonCaseEffectType);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_JIGGLETWOSTATICS:
    newEffect = new JiggleTwoStaticColor(RGBLights, InButtonCaseEffectType);
    break;

  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAIN_LOW:
    newEffect = new Rain(RGBLights, InButtonCaseEffectType, ERainFrequency::RAIN_LOW);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAIN_MEDIUM:
    newEffect = new Rain(RGBLights, InButtonCaseEffectType, ERainFrequency::RAIN_MEDIUM);
    break;  
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAIN_HIGH:
    newEffect = new Rain(RGBLights, InButtonCaseEffectType, ERainFrequency::RAIN_HIGH);
    break;

  default:
    break;
  }

  return newEffect;
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
  if (this->caseAnimation != nullptr) 
  {
    delete this->caseAnimation;
    this->caseAnimation = nullptr;
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

  bool bCaseLightsUsingButtonNonPressedAnim = this->options.profiles[this->options.baseProfileIndex].baseNonPressedEffect == this->options.profiles[this->options.baseProfileIndex].baseCaseEffect;

  //set new profile nonpressed animation
  this->baseAnimation = GetNonPressedEffectForEffectType(this->options.profiles[this->options.baseProfileIndex].baseNonPressedEffect, bCaseLightsUsingButtonNonPressedAnim ? EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE : EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY);

  //Set case animation if required
  if(!bCaseLightsUsingButtonNonPressedAnim)
  {
    this->caseAnimation = GetNonPressedEffectForEffectType(this->options.profiles[this->options.baseProfileIndex].baseCaseEffect, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY);
  }
  
  //set new profile pressed animation
  switch (this->options.profiles[this->options.baseProfileIndex].basePressedEffect) 
  {
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_RANDOM:
    this->buttonAnimation = new RandomColor(RGBLights, lastPressed);
    break;

  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_STATIC_COLOR:
    this->buttonAnimation = new StaticColor(RGBLights, lastPressed);
    break;
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_JIGGLESTATIC:
    this->buttonAnimation = new JiggleStaticColor(RGBLights, lastPressed);
    break;
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_JIGGLETWOSTATICS:
    this->buttonAnimation = new JiggleTwoStaticColor(RGBLights, lastPressed);
    break;

  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_BURST:
    this->buttonAnimation = new BurstColor(RGBLights, false, false, lastPressed);
    break;
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_BURST_RANDOM:
    this->buttonAnimation = new BurstColor(RGBLights, true, false, lastPressed);
    break;
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_BURST_SMALL:
    this->buttonAnimation = new BurstColor(RGBLights, false, true, lastPressed);
    break;
  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_BURST_SMALL_RANDOM:
    this->buttonAnimation = new BurstColor(RGBLights, true, true, lastPressed);
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

void AnimationStation::decompressSettings()
{
	const AnimationOptions& optionsProto = Storage::getInstance().getAnimationOptions();
	
	options.checksum				= 0;
	options.NumValidProfiles = optionsProto.profiles_count;
	for(int index = 0; index < options.NumValidProfiles && index < 4; ++index) //MAX_ANIMATION_PROFILES from AnimationStation.hpp
	{
		options.profiles[index].bEnabled = optionsProto.profiles[index].bEnabled;
		options.profiles[index].baseNonPressedEffect = (AnimationNonPressedEffects)((int)optionsProto.profiles[index].baseNonPressedEffect);
		options.profiles[index].basePressedEffect = (AnimationPressedEffects)((int)optionsProto.profiles[index].basePressedEffect);
		options.profiles[index].baseCaseEffect = (AnimationNonPressedEffects)((int)optionsProto.profiles[index].baseCaseEffect);
		options.profiles[index].baseCycleTime = optionsProto.profiles[index].baseCycleTime;
		options.profiles[index].basePressedCycleTime = optionsProto.profiles[index].basePressedCycleTime;
		for(unsigned int packedPinIndex = 0; packedPinIndex < (NUM_BANK0_GPIOS/4)+1; ++packedPinIndex)
		{
			int pinIndex = packedPinIndex * 4;
			if(packedPinIndex < optionsProto.profiles[index].notPressedStaticColors_count)
			{
				options.profiles[index].notPressedStaticColors[pinIndex + 0] = optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] & 0xFF;
				options.profiles[index].notPressedStaticColors[pinIndex + 1] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 8) & 0xFF;
				options.profiles[index].notPressedStaticColors[pinIndex + 2] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 16) & 0xFF;
				options.profiles[index].notPressedStaticColors[pinIndex + 3] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 24) & 0xFF;
			}
			if(packedPinIndex < optionsProto.profiles[index].pressedStaticColors_count)
			{
				options.profiles[index].pressedStaticColors[pinIndex + 0] = optionsProto.profiles[index].pressedStaticColors[packedPinIndex] & 0xFF;
				options.profiles[index].pressedStaticColors[pinIndex + 1] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 8) & 0xFF;
				options.profiles[index].pressedStaticColors[pinIndex + 2] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 16) & 0xFF;
				options.profiles[index].pressedStaticColors[pinIndex + 3] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 24) & 0xFF;
			}
		}
		for(unsigned int packedCaseIndex = 0; packedCaseIndex < (MAX_CASE_LIGHTS / 4) && packedCaseIndex < optionsProto.profiles[index].caseStaticColors_count; ++packedCaseIndex)
		{
			int caseIndex = packedCaseIndex * 4;
			options.profiles[index].caseStaticColors[caseIndex + 0] = optionsProto.profiles[index].caseStaticColors[packedCaseIndex] & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 1] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 8) & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 2] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 16) & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 3] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 24) & 0xFF;
		}
		options.profiles[index].buttonPressHoldTimeInMs = optionsProto.profiles[index].buttonPressHoldTimeInMs;
		options.profiles[index].buttonPressFadeOutTimeInMs = optionsProto.profiles[index].buttonPressFadeOutTimeInMs;
		options.profiles[index].nonPressedSpecialColor = optionsProto.profiles[index].nonPressedSpecialColor;
		options.profiles[index].pressedSpecialColor = optionsProto.profiles[index].pressedSpecialColor;
		options.profiles[index].bUseCaseLightsInSpecialMoves = optionsProto.profiles[index].bUseCaseLightsInSpecialMoves;
	}
	options.brightness				= std::min<uint32_t>(optionsProto.brightness, 255);
	options.baseProfileIndex		= optionsProto.baseProfileIndex;

	customColors.clear();
	for(unsigned int customColIndex = 0; customColIndex < MAX_CUSTOM_COLORS; ++customColIndex)
	{
		customColors.push_back(optionsProto.customColors[customColIndex]);
	}
}

void AnimationStation::checkForOptionsUpdate()
{
  bool bChangeDetected = false;
  AnimationOptions& optionsProto = Storage::getInstance().getAnimationOptions();

  //Any changes?
	for(int index = 0; index < MAX_ANIMATION_PROFILES && !bChangeDetected; ++index)
	{
		if(optionsProto.profiles[index].baseCycleTime != options.profiles[index].baseCycleTime)
      bChangeDetected = true;
		else if(optionsProto.profiles[index].basePressedCycleTime != options.profiles[index].basePressedCycleTime)
      bChangeDetected = true;
	}
	if(optionsProto.brightness != options.brightness)
      bChangeDetected = true;
	else if(optionsProto.baseProfileIndex != options.baseProfileIndex)
      bChangeDetected = true;

  //only change settings if they've been static for a little while
  if(bChangeDetected)
  {
    if(!bAnimConfigSaveNeeded)
    {
      bAnimConfigSaveNeeded = true;
      timeAnimationSaveSet = get_absolute_time();
    }

    if(bAnimConfigSaveNeeded && (absolute_time_diff_us(timeAnimationSaveSet, get_absolute_time()) / 1000) > 1000) // 1 second delay on saves
    {
      bAnimConfigSaveNeeded = false;
      for(int index = 0; index < MAX_ANIMATION_PROFILES; ++index)
      {
        optionsProto.profiles[index].baseCycleTime = options.profiles[index].baseCycleTime;
        optionsProto.profiles[index].basePressedCycleTime = options.profiles[index].basePressedCycleTime;
      }
      optionsProto.brightness					= options.brightness;
      optionsProto.baseProfileIndex			= options.baseProfileIndex;

      EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(false));
    }
  }
}
