#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "hardware/clocks.h"

#include "NeoPico.h"
#include "animation.h"
#include "specialmovesystem.h"

#include "config.pb.h"

#define MAX_ANIMATION_PROFILES 4
#define MAX_ANIMATION_PROFILES_INCLUDING_TEST (MAX_ANIMATION_PROFILES+1)
#define MAX_CASE_LIGHTS 40          //this should be divisible by 4 as we pack 4 indexes into one config int32

typedef enum
{
  AnimationStation_TestModeInvalid,
  AnimationStation_TestModeOff,
	AnimationStation_TestModeButtons,
	AnimationStation_TestModeLayout,
	AnimationStation_TestModeProfilePreview,
} AnimationStationTestMode;

struct __attribute__ ((__packed__)) AnimationProfile_Unpacked
{
    bool bEnabled = false;

  	AnimationNonPressedEffects baseNonPressedEffect;
  	AnimationPressedEffects basePressedEffect;
    AnimationNonPressedEffects baseCaseEffect;

    int16_t baseCycleTime;
    int16_t basePressedCycleTime;

    uint32_t notPressedStaticColors[NUM_BANK0_GPIOS + 3]; //since we pack 4 into each. Adding 3 ensures we have space for extra pading
    uint32_t pressedStaticColors[NUM_BANK0_GPIOS + 3]; //since we pack 4 into each. Adding 3 ensures we have space for extra pading

    uint32_t caseStaticColors[MAX_CASE_LIGHTS];

    uint32_t buttonPressHoldTimeInMs;
    uint32_t buttonPressFadeOutTimeInMs;

    uint32_t nonPressedSpecialColor;
    uint32_t pressedSpecialColor;

    bool bUseCaseLightsInSpecialMoves;
    bool bUseCaseLightsInPressedAnimations;
};

struct __attribute__ ((__packed__)) AnimationOptions_Unpacked
{
  uint32_t checksum;
  uint8_t NumValidProfiles;
  AnimationProfile_Unpacked profiles[MAX_ANIMATION_PROFILES_INCLUDING_TEST];
  uint8_t brightness;
  int8_t baseProfileIndex;
  uint32_t autoDisableTime;
};

class AnimationStation
{
public:
  AnimationStation();

  void Animate();
  void HandleEvent(GamepadHotkey action);
  void Clear();
  void ApplyBrightness(uint32_t *frameValue);

  //Change profiles
  void ChangeProfile(int changeSize);
  uint16_t AdjustIndex(int changeSize);

  //What buttons (physical gpio pins) are pressed this frame
  void HandlePressedPins(std::vector<int32_t> pressedPins);

  //What buttons (logical ones) are pressed this frame
  void HandlePressedButtons(uint32_t pressedButtons);

  int8_t GetMode();
  void SetMode(int8_t mode);
  void SetLights(Lights InRGBLights);

  //Brightness settings
  static void ConfigureBrightness(uint8_t max, uint8_t steps);
  static float GetBrightnessX();
  static uint8_t GetBrightness();
  static void SetBrightness(uint8_t brightness);
  static void DecreaseBrightness();
  static void IncreaseBrightness();
  static void DimBrightnessTo0();

  static void DecompressProfile(int ProfileIndex, const AnimationProfile* ProfileToDecompress);
  void DecompressSettings();
  void CheckForOptionsUpdate();
 
  //special move anim
  void SetSpecialMoveAnimation(SpecialMoveEffects AnimationToPlay, uint32_t OptionalParams);

  //Testing/webconfig
  static void SetTestMode(AnimationStationTestMode TestType, const AnimationProfile* TestProfile);
  static void SetTestPinState(int PinOrCaseIndex, bool IsCaseLight);

  SpecialMoveSystem specialMoveSystem;

  //Running non-pressed animation
  Animation* baseAnimation;

  //Running case animation
  Animation* caseAnimation;

  //Running pressed animation
  Animation* buttonAnimation;

  //Running special move animation
  Animation* specialMoveAnimation;

  //Buttons pressed (physical gipo pins) last frame, used when changing button theme so starts initialised
  std::vector<int32_t> lastPressed;

  static AnimationOptions_Unpacked options;

  static absolute_time_t nextChange;

  //Color of all lights this frame
  RGB frame[FRAME_MAX];

  static std::string printfs[4];

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static float brightnessX;

  Animation* GetNonPressedEffectForEffectType(AnimationNonPressedEffects EffectType, EButtonCaseEffectType InButtonCaseEffectType);

  //webconfig test mode
  void UpdateTestMode();

  void UpdateTimeout();

  //Light data
  Lights RGBLights;

  //options/save
  absolute_time_t timeAnimationSaveSet;
  bool bAnimConfigSaveNeeded = false;

  //idletimeout
  absolute_time_t timeLastButtonPressed;
  bool bIsInIdleTimeout = false;

  //Testing/webconfig
  static AnimationStationTestMode TestMode;
  static bool bTestModeChangeRequested;
  static int TestModePinOrCaseIndex;
  static bool TestModeLightIsCase;
};

#endif
