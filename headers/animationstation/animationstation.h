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

#include "config.pb.h"

#define MAX_ANIMATION_PROFILES 4
#define MAX_ANIMATION_PROFILES_INCLUDING_TEST (MAX_ANIMATION_PROFILES+1)
#define MAX_NON_BUTTON_LIGHT_COLOR_INDEXES _u(32)          //Total of color indexs in animation.h + Max custom colours (then increased to be a multiple of 4)

#define CYCLE_STEPS 10

typedef enum
{
  AnimationStation_TestModeDisableTestMode = 0,
  AnimationStation_TestModeOff,
	AnimationStation_TestModeButtons,
	AnimationStation_TestModeLayout,
	AnimationStation_TestModeProfilePreview
} AnimationStationTestMode;

class AnimationStation
{
public:
	AnimationStation(AnimationStation const&) = delete;
	void operator=(AnimationStation const&)  = delete;
	static AnimationStation& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static AnimationStation instance;
		return instance;
	}

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
  void SetMaxBrightness(uint8_t max);
  float GetNormalisedBrightness();
  uint8_t GetBrightnessStepValue();
  void SetBrightnessStepValue(uint8_t brightness);
  void ApplyBrightnessStepValue();
  void DecreaseBrightnessByStep();
  void IncreaseBrightnessByStep();
  void DimBrightnessTo0();

  //static void DecompressProfile(int ProfileIndex, const AnimationProfile* ProfileToDecompress);
  //void DecompressSettings();
  void CheckForOptionsUpdate();
 
  //Testing/webconfig
  void SetTestMode(AnimationStationTestMode TestType, const AnimationProfile* TestProfile, uint8_t overrideBrightness, uint8_t overrideMaxBrightness);
  void SetTestPinState(int PinOrNonButtonIndex, bool IsNonButtonLight);
  void ClearTestMode();

  uint8_t getBrightnessSteps() { return brightnessSteps; }
  uint8_t getTestMode() { return TestMode; }
  bool getTestModeLightIsNonButton() { return TestModeLightIsNonButton; }
  int getTestModePinOrNonButtonIndex() { return TestModePinOrNonButtonIndex; }

protected:
  uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  uint8_t brightnessMax; //0-255
  uint8_t brightnessStepValue; //0-10
  float normalisedBrightness; //0-1

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

private:
	AnimationStation();

  //Running non-pressed animation
  Animation* baseAnimation;

  //Running case animation
  Animation* caseAnimation;

  //Running pressed animation
  Animation* buttonAnimation;

  //Buttons pressed (physical gipo pins) last frame, used when changing button theme so starts initialised
  std::vector<int32_t> lastPressed;

  absolute_time_t nextChange;

  //Color of all lights this frame
  RGB frame[FRAME_MAX];

  uint8_t brightnessSteps; 

  //Testing/webconfig
  AnimationStationTestMode TestMode;
  bool bTestModeChangeRequested;
  int TestModePinOrNonButtonIndex;
  bool TestModeLightIsNonButton;
};

#endif
