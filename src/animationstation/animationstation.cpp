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
#include "effects/idletimeout.h"
#include "effects/jiggletwostaticcolor.h"
#include "effects/randomcolor.h"

#include "animationstation.h"

#include "storagemanager.h"

#include "enums.pb.h"
#include "config.pb.h"

AnimationStation::AnimationStation() {
    AnimationOptions & options = Storage::getInstance().getAnimationOptions();

    // Changed when neopicoled starts up
    brightnessMax = 100;
    brightnessSteps = 10;
    brightnessStepValue = 0;
    normalisedBrightness = 0;
    TestMode = AnimationStationTestMode::AnimationStation_TestModeDisableTestMode;
    bTestModeChangeRequested = false;
    TestModePinOrNonButtonIndex = -1;
    TestModeLightIsNonButton = false;
    SetBrightnessStepValue(1);
    timeLastButtonPressed = get_absolute_time();

    lastAction = HOTKEY_LEDS_NONE;
  
    // For save update
    bChangeDetected = false;
    bRestartLeds = false;
}

void AnimationStation::SetLights(Lights InRGBLights) {
    RGBLights = InRGBLights;
}

void AnimationStation::SetMaxBrightness(uint8_t max) {
    brightnessMax = max;
    if(brightnessMax < brightnessSteps)
      brightnessMax = brightnessSteps;
}

void AnimationStation::HandleEvent(GamepadHotkey action) {
    AnimationOptions & options = Storage::getInstance().getAnimationOptions();

    // Do nothing if we haven't reached our 250ms timeout
    if (action == lastAction) {
      return;
    }

    switch(action) {
        case HOTKEY_LEDS_NONE:
            break;
        case HOTKEY_LEDS_BRIGHTNESS_UP:
            IncreaseBrightnessByStep();
            bChangeDetected = true;
            break;
        case HOTKEY_LEDS_BRIGHTNESS_DOWN:
            DecreaseBrightnessByStep();
            bChangeDetected = true;
            break;
        case HOTKEY_LEDS_PROFILE_UP:
            if ( IncreaseProfile() ) {
              bChangeDetected = true;
            }
            break;
        case HOTKEY_LEDS_PROFILE_DOWN:
            if ( DecreaseProfile() ) {
              bChangeDetected = true;
            }
            break;
        case HOTKEY_LEDS_PARAMETER_CYCLE:
            if ( baseAnimation == nullptr || options.baseProfileIndex == -1 )
                return;
            options.profiles[options.baseProfileIndex].baseCycleTime++;
            if(options.profiles[options.baseProfileIndex].baseCycleTime >= CYCLE_STEPS)
                options.profiles[options.baseProfileIndex].baseCycleTime = 0;
            baseAnimation->CycleParameterChange();
            EventManager::getInstance().triggerEvent(new GPLEDEvent(false, true, false, false, false));
            bChangeDetected = true;
            break;
        case HOTKEY_LEDS_CASE_PARAMETER_CYCLE:
            if ( caseAnimation == nullptr || options.baseProfileIndex == -1 )
                return;
            options.profiles[options.baseProfileIndex].baseCaseCycleTime++;
            if( options.profiles[options.baseProfileIndex].baseCaseCycleTime >= CYCLE_STEPS )
                options.profiles[options.baseProfileIndex].baseCaseCycleTime = 0;
            caseAnimation->CycleParameterChange();
            EventManager::getInstance().triggerEvent(new GPLEDEvent(false, false, true, false, false));
            bChangeDetected = true;
            break;
        case HOTKEY_LEDS_PRESS_PARAMETER_CYCLE:
            if ( buttonAnimation == nullptr || options.baseProfileIndex == -1 )
                return;
            options.profiles[options.baseProfileIndex].basePressedCycleTime++;
            if( options.profiles[options.baseProfileIndex].basePressedCycleTime >= CYCLE_STEPS )
                options.profiles[options.baseProfileIndex].basePressedCycleTime = 0;
            buttonAnimation->CycleParameterChange();
            EventManager::getInstance().triggerEvent(new GPLEDEvent(false, false, false, true, false));
            bChangeDetected = true;
            break;
        default:
            break;
    };

    lastAction = action;
}

void AnimationStation::ChangeProfile(int changeSize) {
    timeLastButtonPressed = get_absolute_time();

    this->SetMode(this->AdjustIndex(changeSize));
}

bool AnimationStation::IncreaseProfile() {
    // find next valid index
    AnimationOptions & options = Storage::getInstance().getAnimationOptions();
    int32_t nextIndex = options.baseProfileIndex;
    while ( 1 ) { // find the next LED profile or disable profile
        if ( nextIndex == MAX_ANIMATION_PROFILES-1 ) { // disable profile
            SetMode(-1);
            EventManager::getInstance().triggerEvent(new GPLEDEvent(true, false, false, false, false));
            return true;
        }
        nextIndex++;
        if ( options.profiles[nextIndex].bEnabled ) {
            SetMode(nextIndex);
            EventManager::getInstance().triggerEvent(new GPLEDEvent(true, false, false, false, false));
            return true;
        }
    }
    return false;
}

bool AnimationStation::DecreaseProfile() {
    // find next valid index
    AnimationOptions & options = Storage::getInstance().getAnimationOptions();
    int32_t prevIndex = options.baseProfileIndex;
    if ( prevIndex == -1 ) { // Disabled, wraparound to the top
      prevIndex = MAX_ANIMATION_PROFILES;
    }
    while ( 1 ) {
        if ( prevIndex == 0 ) { // disable profile
            SetMode(-1);
            EventManager::getInstance().triggerEvent(new GPLEDEvent(true, false, false, false, false));
            return true;
        }
        prevIndex--;
        if ( options.profiles[prevIndex].bEnabled ) {
            SetMode(prevIndex);
            EventManager::getInstance().triggerEvent(new GPLEDEvent(true, false, false, false, false));
            return true;
        }
    }
    return false;
}

void AnimationStation::HandlePressedPins(std::vector<int32_t> pressedPins) {
    if(pressedPins.size()) {
        timeLastButtonPressed = get_absolute_time();
        this->lastPressed = pressedPins;
        if(this->buttonAnimation)
            this->buttonAnimation->UpdatePressed(pressedPins);
    } else {
      this->lastPressed.clear();
      if(this->buttonAnimation)
        this->buttonAnimation->ClearPressed();
    }
}

void AnimationStation::HandlePressedButtons(uint32_t pressedButtons) {
}

void AnimationStation::UpdateTestMode()
{
    if(!bTestModeChangeRequested)
        return;
    bTestModeChangeRequested = false;

    switch(TestMode) {
        case AnimationStationTestMode::AnimationStation_TestModeOff:
            SetMode(-1);
            break;
        case AnimationStationTestMode::AnimationStation_TestModeButtons:
        case AnimationStationTestMode::AnimationStation_TestModeLayout:
        case AnimationStationTestMode::AnimationStation_TestModeProfilePreview:
            SetMode(MAX_ANIMATION_PROFILES_INCLUDING_TEST - 1);
            break;
        case AnimationStationTestMode::AnimationStation_TestModeDisableTestMode:
            SetMode(0);     
            break;
        default:
            break;
    };
}

void AnimationStation::Animate()
{
    //Test mode checks
    UpdateTestMode();

    //timeout checks
    UpdateTimeout();

    //Check for options changing and need saving
    CheckForOptionsUpdate();

    //If no profiles running
    if (baseAnimation == nullptr || buttonAnimation == nullptr) {
        this->Clear();
        return;
    }

    baseAnimation->Animate(this->frame);
    if(caseAnimation != nullptr)
        caseAnimation->Animate(this->frame);
    buttonAnimation->Animate(this->frame);
}

void AnimationStation::Clear()
{
    //sets all lights to black (off)
    memset(frame, 0, sizeof(frame));
}

void AnimationStation::UpdateTimeout()
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();

  if(TestMode == AnimationStationTestMode::AnimationStation_TestModeDisableTestMode && options.autoDisableTime > 0)
  {
    if(bIsInIdleTimeout == false)
    {
      if((absolute_time_diff_us(timeLastButtonPressed, get_absolute_time()) / 1000) > options.autoDisableTime)
      {
        //turn off all lights (but leave pressed effects running so the first press on restart isnt lost)
        bIsInIdleTimeout = true;
        if(this->baseAnimation != nullptr)
          delete this->baseAnimation;
        if(this->caseAnimation != nullptr)
          delete this->caseAnimation;
        
        this->caseAnimation = nullptr;
        this->baseAnimation = new IdleTimeout(RGBLights, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE);
      }
    }
    else if((absolute_time_diff_us(timeLastButtonPressed, get_absolute_time()) / 1000) < options.autoDisableTime)
    {
      bIsInIdleTimeout = false;
      if(this->baseAnimation != nullptr)
        delete this->baseAnimation;
      bool bCaseLightsUsingButtonNonPressedAnim = options.profiles[options.baseProfileIndex].baseNonPressedEffect == options.profiles[options.baseProfileIndex].baseCaseEffect;

      //set new profile nonpressed animation
      this->baseAnimation = GetNonPressedEffectForEffectType(options.profiles[options.baseProfileIndex].baseNonPressedEffect, bCaseLightsUsingButtonNonPressedAnim ? EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE : EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY);
      //Set case animation if required
      if(!bCaseLightsUsingButtonNonPressedAnim)
      {
        this->caseAnimation = GetNonPressedEffectForEffectType(options.profiles[options.baseProfileIndex].baseCaseEffect, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY);
      }
    }
  }
}

void AnimationStation::AssignLedPreset(const unsigned char* data, int32_t dataSize) {
  LEDOptions& options = Storage::getInstance().getLedOptions();
	options.lightClusterData_count = 0;
	options.lightClusterDataInitialised = true;
	for (int entryIndex = 0; (entryIndex * 6) + 5 < dataSize; ++entryIndex) //each data entry has 6 elements
	{
		int dataIndex = entryIndex * 6;
		options.lightClusterData[entryIndex].lightLocationData = data[dataIndex];
		options.lightClusterData[entryIndex].lightLocationData += ((int)data[dataIndex+1]) << 8;
		options.lightClusterData[entryIndex].lightLocationData += ((int)data[dataIndex+2]) << 16;
		options.lightClusterData[entryIndex].lightLocationData += ((int)data[dataIndex+3]) << 24;
		options.lightClusterData[entryIndex].lightTypeData = ((int)data[dataIndex+4]);
		options.lightClusterData[entryIndex].lightTypeData += ((int)data[dataIndex+5]) << 8;

		options.lightClusterData_count = entryIndex + 1;

		if(options.lightClusterData_count >= FRAME_MAX) //100 entries total
			return;
	}
}

int8_t AnimationStation::GetMode()
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();
  return options.baseProfileIndex;
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
    if(TestMode == AnimationStationTestMode::AnimationStation_TestModeLayout)
      newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_TESTLAYOUT);
    else
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
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_CIRCLE_CLOCKWISE:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_CIRCLE_CLOCKWISE);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_CIRCLE_ANTICLOCKWISE:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_CIRCLE_ANTICLOCKWISE);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_CIRCLE_PINGPONG:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_CIRCLE_PINGPONG);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_INDEX:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_INDEX);
    break;
  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_INDEX_PINGPONG:
    newEffect = new Chase(RGBLights, InButtonCaseEffectType, ChaseTypes::CHASETYPES_INDEX_PINGPONG);
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

  case AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAIN:
    newEffect = new Rain(RGBLights, InButtonCaseEffectType);
    break;

  default:
    break;
  }

  return newEffect;
}

void AnimationStation::SetMode(int8_t mode)
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();

  options.baseProfileIndex = mode;

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

  bool bCaseLightsUsingButtonNonPressedAnim = options.profiles[options.baseProfileIndex].baseNonPressedEffect == options.profiles[options.baseProfileIndex].baseCaseEffect;

  //set new profile nonpressed animation
  this->baseAnimation = GetNonPressedEffectForEffectType(options.profiles[options.baseProfileIndex].baseNonPressedEffect, bCaseLightsUsingButtonNonPressedAnim ? EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE : EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY);

  //Set case animation if required
  if(!bCaseLightsUsingButtonNonPressedAnim)
  {
    this->caseAnimation = GetNonPressedEffectForEffectType(options.profiles[options.baseProfileIndex].baseCaseEffect, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY);
  }

  //set new profile pressed animation
  //for effects that can alter multiple lights, tell them if they should also effect case lights
  EButtonCaseEffectType buttonCaseEffectType = options.profiles[options.baseProfileIndex].bUseCaseLightsInPressedAnimations ? EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE : EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY;
  switch (options.profiles[options.baseProfileIndex].basePressedEffect)
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
    this->buttonAnimation = new BurstColor(RGBLights, lastPressed, buttonCaseEffectType);
    break;

  case AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_BURST_SMALL:
    this->buttonAnimation = new BurstColor(RGBLights, lastPressed, buttonCaseEffectType, true);
    break;

  default:
    break;
  }
}

///////////////////////////////////
// Brightness functions
///////////////////////////////////

void AnimationStation::ApplyBrightness(uint32_t *frameValue)
{
  for (int i = 0; i < FRAME_MAX; i++)
    frameValue[i] = this->frame[i].value(format, normalisedBrightness);
}

void AnimationStation::SetBrightnessStepValue(uint8_t brightness)
{
  brightnessStepValue = brightness;
  ApplyBrightnessStepValue();
}

void AnimationStation::ApplyBrightnessStepValue()
{
  brightnessStepValue = std::clamp<uint32_t>(brightnessStepValue, 0, brightnessSteps);
  normalisedBrightness = (brightnessStepValue * getBrightnessStepSize()) / 255.0F;
  normalisedBrightness = std::clamp<float>(normalisedBrightness, 0.0f, 1.0f);
}

void AnimationStation::DecreaseBrightnessByStep()
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();
  options.brightness = std::clamp<int32_t>(((int32_t)options.brightness)-1, 0, brightnessSteps);
  SetBrightnessStepValue(options.brightness);

  EventManager::getInstance().triggerEvent(new GPLEDEvent(false, false, false, false, true));
}

void AnimationStation::IncreaseBrightnessByStep()
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();
  options.brightness = std::clamp<int32_t>(options.brightness+1, 0, brightnessSteps);
  SetBrightnessStepValue(options.brightness);

  EventManager::getInstance().triggerEvent(new GPLEDEvent(false, false, false, false, true));
}

void AnimationStation::DimBrightnessTo0()
{
  normalisedBrightness = 0;
}

float AnimationStation::GetNormalisedBrightness()
{
  return normalisedBrightness;
}

uint8_t AnimationStation::GetBrightnessStepValue()
{
  return brightnessStepValue;
}

void AnimationStation::CopyTestProfile(const AnimationProfile* newProfile) {
    AnimationOptions& options = Storage::getInstance().getAnimationOptions();
    int ProfileIndex = MAX_ANIMATION_PROFILES_INCLUDING_TEST - 1;
    memcpy(&options.profiles[ProfileIndex], newProfile, sizeof(AnimationProfile));
}

void AnimationStation::SetTestModeLayout(){
    //Set up test profile that is Chase Random
    AnimationOptions& options = Storage::getInstance().getAnimationOptions();
    int testProfileIndex = MAX_ANIMATION_PROFILES_INCLUDING_TEST - 1;
    options.profiles[testProfileIndex].baseCaseEffect = AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_LEFT_TO_RIGHT;
    options.profiles[testProfileIndex].baseNonPressedEffect = AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_CHASE_LEFT_TO_RIGHT;
    options.profiles[testProfileIndex].basePressedEffect = AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_STATIC_COLOR;
    memset(options.profiles[testProfileIndex].notPressedStaticColors.bytes, 0, NUM_BANK0_GPIOS);
    memset(options.profiles[testProfileIndex].pressedStaticColors.bytes, 0, NUM_BANK0_GPIOS);
    memset(options.profiles[testProfileIndex].nonButtonStaticColors.bytes, 0, MAX_NON_BUTTON_LIGHT_COLOR_INDEXES);
    options.profiles[testProfileIndex].nonPressedSpecialColor = 0xFFFFFF; //White
    options.profiles[testProfileIndex].caseSpecialColor = 0xFFFFFF; //White
    options.profiles[testProfileIndex].baseCycleTime = 2;
    options.profiles[testProfileIndex].basePressedCycleTime = 2;
    options.profiles[testProfileIndex].baseCaseCycleTime = 2;
}

void AnimationStation::SetTestModeButton(){
    //Set up test profile that is all black
    AnimationOptions& options = Storage::getInstance().getAnimationOptions();
    int testProfileIndex = MAX_ANIMATION_PROFILES_INCLUDING_TEST - 1;
    options.profiles[testProfileIndex].baseCaseEffect = AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_STATIC_COLOR;
    options.profiles[testProfileIndex].baseNonPressedEffect = AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_STATIC_COLOR;
    options.profiles[testProfileIndex].basePressedEffect = AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_STATIC_COLOR;
    memset(options.profiles[testProfileIndex].notPressedStaticColors.bytes, 0, NUM_BANK0_GPIOS);
    memset(options.profiles[testProfileIndex].pressedStaticColors.bytes, 0, NUM_BANK0_GPIOS);
    memset(options.profiles[testProfileIndex].nonButtonStaticColors.bytes, 0, MAX_NON_BUTTON_LIGHT_COLOR_INDEXES);
}

void AnimationStation::InitSettings()
{
	const AnimationOptions& options = Storage::getInstance().getAnimationOptions();

	customColors.clear();
	for(unsigned int customColIndex = 0; customColIndex < MAX_CUSTOM_COLORS; ++customColIndex)
	{
		customColors.push_back(options.customColors[customColIndex]);
	}
}

void AnimationStation::CheckForOptionsUpdate()
{
  //No saving in test/webconfig mode
  if(TestMode != AnimationStationTestMode::AnimationStation_TestModeDisableTestMode)
    return;

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
      EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(false));
    }
  }
}

//Testmode functions
void AnimationStation::SetTestMode(AnimationStationTestMode testType, const AnimationProfile* testProfile, uint8_t overrideBrightness, uint8_t overrideMaxBrightness) {
    bTestModeChangeRequested = true;
    TestMode = testType;
    SetMaxBrightness(overrideMaxBrightness);
    SetBrightnessStepValue(overrideBrightness);
    if(TestMode == AnimationStationTestMode::AnimationStation_TestModeProfilePreview)
      CopyTestProfile(testProfile);
    else if(TestMode == AnimationStationTestMode::AnimationStation_TestModeLayout) {
      SetTestModeLayout();
    }
    else if(TestMode == AnimationStationTestMode::AnimationStation_TestModeButtons) {
      SetTestModeButton();
    }
}

void AnimationStation::SetTestPinState(int PinOrNonButtonIndex, bool IsNonButtonLight)
{
    AnimationOptions & options = Storage::getInstance().getAnimationOptions();
    int testProfileIndex = MAX_ANIMATION_PROFILES_INCLUDING_TEST - 1;

    //reset old test light
    if(TestModePinOrNonButtonIndex != -1) {
      if(TestModeLightIsNonButton) {
        options.profiles[testProfileIndex].nonButtonStaticColors.bytes[TestModePinOrNonButtonIndex] = 0x00; //Black/off
      } else {
        options.profiles[testProfileIndex].notPressedStaticColors.bytes[TestModePinOrNonButtonIndex] = 0x00; //Black/off
      }
    }

    //Store new test light
    TestModePinOrNonButtonIndex = PinOrNonButtonIndex;
    TestModeLightIsNonButton = IsNonButtonLight;

    if(TestModePinOrNonButtonIndex != -1) {
        if(IsNonButtonLight) {
          options.profiles[testProfileIndex].notPressedStaticColors.bytes[PinOrNonButtonIndex] = 0x01; //White
        } else {
          options.profiles[testProfileIndex].notPressedStaticColors.bytes[PinOrNonButtonIndex] = 0x01; //White
        }
    }
}

void AnimationStation::ClearTestMode()
{
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();
  bTestModeChangeRequested = true;
  TestMode = AnimationStationTestMode::AnimationStation_TestModeDisableTestMode;
  LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
  SetMaxBrightness(ledOptions.brightnessMaximum);
  SetBrightnessStepValue(options.brightness);
}

RGB AnimationStation::GetColorForIndex(uint32_t ColorIndex) {
    //pre defined color?
    if(ColorIndex < (uint32_t)colors.size())
      return colors[ColorIndex];

    //must be custom color
    ColorIndex -= colors.size();
    if(ColorIndex > customColors.size())
    {
      //error, no such color
      return colors[0];
    }
    return customColors[ColorIndex];
}

//Get correct color for light index
RGB AnimationStation::StaticGetNonPressedColorForLight(Lights* AllLights, uint32_t LightIndex) {
  AnimationStation & AnimStation = AnimationStation::getInstance();
  AnimationOptions & options = Storage::getInstance().getAnimationOptions();
  int colIndex = 0;
  Light* thisLight = &(AllLights->AllLights[LightIndex]);
  if(thisLight->Type == LightType::LightType_ActionButton || thisLight->Type == LightType::LightType_Turbo)
  {
    //button
    colIndex = options.profiles[options.baseProfileIndex].notPressedStaticColors.bytes[thisLight->GPIOPin];
  }
  else
  {
    //If we're in test mode for case lights then turn all lights black and return white for the requested case Light
    if(AnimStation.getTestModeLightIsNonButton() && AnimStation.getTestModePinOrNonButtonIndex() != -1)
    {
      colIndex = 0;
      if(thisLight->Type == LightType::LightType_Turbo && ((int)thisLight->FirstLedIndex == AnimStation.getTestModePinOrNonButtonIndex()))
        colIndex = 1;
    }
    else
    {
      //case light or player led
      colIndex = options.profiles[options.baseProfileIndex].nonButtonStaticColors.bytes[thisLight->NonButtonIndex];
    }
  }

  return GetColorForIndex(colIndex);
}
