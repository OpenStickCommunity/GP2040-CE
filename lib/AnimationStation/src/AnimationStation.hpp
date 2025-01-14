#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "hardware/clocks.h"

#include "NeoPico.hpp"
#include "Animation.hpp"
#include "SpecialMoveSystem.hpp"

#define MAX_ANIMATION_PROFILES 4
#define MAX_CASE_LIGHTS 40          //this should be divisible by 4 as we pack 4 indexes into one config int32

//List of non-pressed animation types
typedef enum
{
  NONPRESSED_EFFECT_STATIC_COLOR,
  NONPRESSED_EFFECT_RAINBOW_SYNCED,
  NONPRESSED_EFFECT_RAINBOW_ROTATE,
  NONPRESSED_EFFECT_CHASE_SEQUENTIAL,
  NONPRESSED_EFFECT_CHASE_LEFT_TO_RIGHT,
  NONPRESSED_EFFECT_CHASE_RIGHT_TO_LEFT,
  NONPRESSED_EFFECT_CHASE_TOP_TO_BOTTOM,
  NONPRESSED_EFFECT_CHASE_BOTTOM_TO_TOP,
  NONPRESSED_EFFECT_CHASE_SEQUENTIAL_PINGPONG,
  NONPRESSED_EFFECT_CHASE_HORIZONTAL_PINGPONG,
  NONPRESSED_EFFECT_CHASE_VERTICAL_PINGPONG,
  NONPRESSED_EFFECT_CHASE_RANDOM,
  NONPRESSED_EFFECT_JIGGLESTATIC,
  NONPRESSED_EFFECT_JIGGLETWOSTATIC,
} AnimationNonPressedEffects;

//List of non-pressed animation types
typedef enum
{
  PRESSED_EFFECT_STATIC_COLOR,
  PRESSED_EFFECT_RANDOM,
  PRESSED_EFFECT_JIGGLESTATIC,
  PRESSED_EFFECT_JIGGLETWOSTATIC,
} AnimationPressedEffects;

typedef enum
{
  HOTKEY_LEDS_NONE,
	HOTKEY_LEDS_PROFILE_UP,
	HOTKEY_LEDS_PROFILE_DOWN,
  HOTKEY_LEDS_PRESS_PARAMETER_UP,
  HOTKEY_LEDS_PRESS_PARAMETER_DOWN,
	HOTKEY_LEDS_PARAMETER_UP,
	HOTKEY_LEDS_PARAMETER_DOWN,
	HOTKEY_LEDS_BRIGHTNESS_UP,
	HOTKEY_LEDS_BRIGHTNESS_DOWN,
  HOTKEY_LEDS_SPECIALMOVE_PROFILE_UP,
	HOTKEY_LEDS_SPECIALMOVE_PROFILE_DOWN,
} AnimationHotkey;

struct __attribute__ ((__packed__)) AnimationProfile
{
    bool bEnabled = false;

  	AnimationNonPressedEffects baseNonPressedEffect;
  	AnimationPressedEffects basePressedEffect;
    AnimationNonPressedEffects baseCaseEffect;

    int16_t baseCycleTime;
   
    uint32_t notPressedStaticColors[NUM_BANK0_GPIOS + 3]; //since we pack 4 into each. Adding 3 ensures we have space for extra pading
    uint32_t pressedStaticColors[NUM_BANK0_GPIOS + 3]; //since we pack 4 into each. Adding 3 ensures we have space for extra pading

    uint32_t caseStaticColors[MAX_CASE_LIGHTS];

    uint32_t buttonPressHoldTimeInMs;
    uint32_t buttonPressFadeOutTimeInMs;

    uint32_t nonPressedSpecialColour;
    uint32_t pressedSpecialColour;

    bool bUseCaseLightsInSpecialMoves;
};

struct __attribute__ ((__packed__)) AnimationOptions
{
  uint32_t checksum;
  uint8_t NumValidProfiles;
  AnimationProfile profiles[MAX_ANIMATION_PROFILES];
  uint8_t brightness;
  int8_t baseProfileIndex;
};

class AnimationStation
{
public:
  AnimationStation();

  void Animate();
  void HandleEvent(AnimationHotkey action);
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

  //passed in user options
  //static void SetOptions(AnimationOptions InOptions);
 
  //special move anim
  void SetSpecialMoveAnimation(SpecialMoveEffects AnimationToPlay, uint32_t OptionalParams);

  SpecialMoveSystem specialMoveSystem;

  //Running non-pressed animation
  Animation* baseAnimation;

  //Running pressed animation
  Animation* buttonAnimation;

  //Running special move animation
  Animation* specialMoveAnimation;

  //Buttons pressed (physical gipo pins) last frame, used when changing button theme so starts initialised
  std::vector<int32_t> lastPressed;

  static AnimationOptions options;

  static absolute_time_t nextChange;

  //Colour of all lights this frame
  RGB frame[100];

  static std::string printfs[4];

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static float brightnessX;

  //Light data
  Lights RGBLights;
};

#endif
