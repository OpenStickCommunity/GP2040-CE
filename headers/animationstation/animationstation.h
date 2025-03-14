#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "hardware/clocks.h"

#include "NeoPico.hpp"
#include "animation.h"
#include "effects/chase.h"
#include "effects/customtheme.h"
#include "effects/customthemepressed.h"
#include "effects/rainbow.h"
#include "effects/staticcolor.h"
#include "effects/statictheme.h"

typedef enum
{
  EFFECT_STATIC_COLOR,
  EFFECT_RAINBOW,
  EFFECT_CHASE,
  EFFECT_STATIC_THEME,
  EFFECT_CUSTOM_THEME,
} AnimationEffects;

const int TOTAL_EFFECTS = 4; // Exclude custom theme until verified present

typedef enum
{
  HOTKEY_LEDS_NONE,
	HOTKEY_LEDS_ANIMATION_UP,
	HOTKEY_LEDS_ANIMATION_DOWN,
	HOTKEY_LEDS_PARAMETER_UP,
  HOTKEY_LEDS_PRESS_PARAMETER_UP,
  HOTKEY_LEDS_PRESS_PARAMETER_DOWN,
	HOTKEY_LEDS_PARAMETER_DOWN,
	HOTKEY_LEDS_BRIGHTNESS_UP,
	HOTKEY_LEDS_BRIGHTNESS_DOWN,
	HOTKEY_LEDS_FADETIME_UP,
	HOTKEY_LEDS_FADETIME_DOWN,
	AMBIENT_LIGHT_EFFECTS_CHANGE, 
	AMBIENT_LIGHT_EFFECTS_ON_OFF, 
	AMBIENT_LIGHT_EFFECTS_BRIGHTNESS_UP,
	AMBIENT_LIGHT_EFFECTS_BRIGHTNESS_DOWN,
	AMBIENT_LIGHT_EFFECTS_PARAMETER_UP,
	AMBIENT_LIGHT_EFFECTS_PARAMETER_DOWN,
	AMBIENT_LIGHT_EFFECTS_FRAME_SPEED_UP,
	AMBIENT_LIGHT_EFFECTS_FRAME_SPEED_DOWN,
	AMBIENT_LIGHT_EFFECTS_CUSTOM_LINKAGE,
} AnimationHotkey;

struct __attribute__ ((__packed__)) AnimationOptions
{
  uint32_t checksum;
	uint8_t baseAnimationIndex;
  uint8_t brightness;
  uint8_t staticColorIndex;
  uint8_t buttonColorIndex;
  int16_t chaseCycleTime;
  int16_t rainbowCycleTime;
  uint8_t themeIndex;
  bool hasCustomTheme;
  uint32_t customThemeUp;
  uint32_t customThemeDown;
  uint32_t customThemeLeft;
  uint32_t customThemeRight;
  uint32_t customThemeB1;
  uint32_t customThemeB2;
  uint32_t customThemeB3;
  uint32_t customThemeB4;
  uint32_t customThemeL1;
  uint32_t customThemeR1;
  uint32_t customThemeL2;
  uint32_t customThemeR2;
  uint32_t customThemeS1;
  uint32_t customThemeS2;
  uint32_t customThemeL3;
  uint32_t customThemeR3;
  uint32_t customThemeA1;
  uint32_t customThemeA2;
  uint32_t customThemeUpPressed;
  uint32_t customThemeDownPressed;
  uint32_t customThemeLeftPressed;
  uint32_t customThemeRightPressed;
  uint32_t customThemeB1Pressed;
  uint32_t customThemeB2Pressed;
  uint32_t customThemeB3Pressed;
  uint32_t customThemeB4Pressed;
  uint32_t customThemeL1Pressed;
  uint32_t customThemeR1Pressed;
  uint32_t customThemeL2Pressed;
  uint32_t customThemeR2Pressed;
  uint32_t customThemeS1Pressed;
  uint32_t customThemeS2Pressed;
  uint32_t customThemeL3Pressed;
  uint32_t customThemeR3Pressed;
  uint32_t customThemeA1Pressed;
  uint32_t customThemeA2Pressed;
  uint32_t buttonPressColorCooldownTimeInMs;
  uint8_t ambientLightEffectsCountIndex;
  float alStaticColorBrightnessCustomX;  
  float alGradientBrightnessCustomX;
  float alChaseBrightnessCustomX;
  float alStaticBrightnessCustomThemeX;
  bool ambientLightCustomLinkageModeFlag;
  uint8_t ambientLightGradientSpeed;
  int16_t ambientLightChaseSpeed;
  float ambientLightBreathSpeed;
  uint8_t alCustomStaticThemeIndex;
  uint8_t alCustomStaticColorIndex;
};

class AnimationStation
{
public:
  AnimationStation();

  void Animate();
  void HandleEvent(AnimationHotkey action);
  void Clear();
  void ChangeAnimation(int changeSize);
  void ApplyBrightness(uint32_t *frameValue);
  uint16_t AdjustIndex(int changeSize);
  void HandlePressed(std::vector<Pixel> pressed);
  void ClearPressed();

  uint8_t GetMode();
  void SetMode(uint8_t mode);
  void SetMatrix(PixelMatrix matrix);
  static void ConfigureBrightness(uint8_t max, uint8_t steps);
  static float GetBrightnessX();
  static float GetLinkageModeOfBrightnessX();
  static uint8_t GetBrightness();
  static void SetBrightness(uint8_t brightness);
  static void DecreaseBrightness();
  static void IncreaseBrightness();
  static void DimBrightnessTo0();
  static void SetOptions(AnimationOptions options);

  Animation* baseAnimation;
  Animation* buttonAnimation;
  std::vector<Pixel> lastPressed;
  static AnimationOptions options;
  static absolute_time_t nextChange;
  static uint8_t effectCount;
  RGB frame[100];
  uint8_t GetBrightnessSteps(){ return this->brightnessSteps; };
  uint8_t GetCustomBrightnessStepsSize(){ return (brightnessMax / brightnessSteps); };
  RGB linkageFrame[100]; // copy baseAnimation frame exclude buttonAnimation frame
  bool ambientLightEffectsChangeFlag = false; 
  bool ambientLightOnOffFlag = false;
  bool ambientLightLinlageOnOffFlag = false;
  bool aleLedsBrightnessCustomXupFlag = false;
  bool aleLedsBrightnessCustomXDownFlag = false;
  bool aleLedsParameterCustomUpFlag = false;
  bool aleLedsParameterCustomDownFlag = false;
  bool alGradientChaseBreathSpeedUpFlag = false;
  bool alGradientChaseBreathSpeedDownFlag = false;
  bool alCustomLinkageModeFlag = false; 

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  inline static uint8_t getLinkageModeOfBrightnessStepSize() { return (255 / brightnessSteps); }
  static float linkageModeOfBrightnessX;
  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static float brightnessX;
  PixelMatrix matrix;
};

#endif
