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
#include "Effects/Rainbow.hpp"
#include "Effects/StaticColor.hpp"
#include "Effects/StaticTheme.hpp"

typedef enum
{
  EFFECT_STATIC_COLOR,
  EFFECT_RAINBOW,
  EFFECT_CHASE,
  EFFECT_STATIC_THEME
} AnimationEffects;

// We can't programmatically determine how many elements are in an enum. Yes, that's dumb.
const int TOTAL_EFFECTS = 4;

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
	HOTKEY_LEDS_BRIGHTNESS_DOWN
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
  static uint8_t GetBrightness();
  static void SetBrightness(uint8_t brightness);
  static void DecreaseBrightness();
  static void IncreaseBrightness();
  static void SetOptions(AnimationOptions options);

  Animation* baseAnimation;
  Animation* buttonAnimation;
  std::vector<Pixel> lastPressed;
  static AnimationOptions options;
  static absolute_time_t nextChange;
  RGB frame[100];

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }
  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static float brightnessX;
  PixelMatrix matrix;
};

#endif
