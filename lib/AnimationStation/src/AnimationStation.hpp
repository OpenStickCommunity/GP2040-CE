#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "hardware/clocks.h"

#include "NeoPico.hpp"
#include "Animation.hpp"
#include "Effects/Chase.hpp"
#include "Effects/CustomTheme.hpp"
#include "Effects/CustomThemePressed.hpp"
#include "Effects/Rainbow.hpp"
#include "Effects/StaticColor.hpp"
#include "Effects/StaticTheme.hpp"

//List of non-pressed animation types
typedef enum
{
  EFFECT_STATIC_COLOR,
  EFFECT_RAINBOW_SYNCED,
  EFFECT_RAINBOW_ROTATE,
  EFFECT_CHASE,
} AnimationEffects;

const int TOTAL_EFFECTS = 4; // Exclude custom theme until verified present

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
} AnimationHotkey;

struct __attribute__ ((__packed__)) AnimationProfile
{
  	AnimationEffects baseAnimationEffect;

    int16_t baseCycleTime;

    uint32_t notPressedStaticColors[NUM_BANK0_GPIOS];
    uint32_t pressedStaticColors[NUM_BANK0_GPIOS];

    uint32_t buttonPressHoldTimeInMs;
    uint32_t buttonPressFadeOutTimeInMs;
};

struct __attribute__ ((__packed__)) AnimationOptions
{
  uint32_t checksum;
  std::vector<AnimationProfile> profiles;
  uint8_t brightness;
  uint8_t baseProfileIndex;
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

  uint8_t GetMode();
  void SetMode(uint8_t mode);
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
  static void SetOptions(AnimationOptions options);

  //Running non-pressed animation
  Animation* baseAnimation;

  //Running pressed animation
  Animation* buttonAnimation;

  //Buttons pressed (physical gipo pins) last frame, used when changing button theme so starts initialised
  std::vector<int32_t> lastPressed;

  static AnimationOptions options;

  static absolute_time_t nextChange;

  //Colour of all lights this frame
  RGB frame[100];

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static float brightnessX;

  //Light data
  Lights RGBLights;
};

#endif
