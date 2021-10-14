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
  HOTKEY_LEDS_NONE,
	HOTKEY_LEDS_ANIMATION_UP,
	HOTKEY_LEDS_ANIMATION_DOWN,
	HOTKEY_LEDS_PARAMETER_UP,
	HOTKEY_LEDS_PARAMETER_DOWN,
	HOTKEY_LEDS_BRIGHTNESS_UP,
	HOTKEY_LEDS_BRIGHTNESS_DOWN
} AnimationHotkey;

class AnimationStation
{
public:
  AnimationStation(PixelMatrix matrix);

  void Animate();
  void AddAnimation(Animation *animation);
  void HandleEvent(AnimationHotkey action);
  void SetStaticColor(RGB color);
  void Clear();
  void ChangeAnimation();
  void ApplyBrightness(uint32_t *frameValue);

  uint8_t GetMode();
  void SetMode(uint8_t mode);
  static void ConfigureBrightness(uint8_t max, uint8_t steps);
  static float GetBrightnessX();
  static uint8_t GetBrightness();
  static void SetBrightness(uint8_t brightness);
  static void DecreaseBrightness();
  static void IncreaseBrightness();

  std::vector<Animation*> animations;
  static absolute_time_t nextAnimationChange;
  static absolute_time_t nextBrightnessChange;
  RGB frame[100];

protected:
  inline static uint8_t getBrightnessStepSize() { return (brightnessMax / brightnessSteps); }

  static uint8_t brightnessMax;
  static uint8_t brightnessSteps;
  static uint8_t brightness;
  static float brightnessX;
  uint8_t animationIndex = 0;
  PixelMatrix matrix;
};

#endif
