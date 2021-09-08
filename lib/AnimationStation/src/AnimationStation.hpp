#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "hardware/clocks.h"

#include "Animation.hpp"

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
  AnimationStation(int numPixels);

  void Animate();
  void HandleEvent(AnimationHotkey action);
  void SetStaticColor();
  void SetRainbow();
  void SetChase();
  void Clear();
  void ChangeAnimation();

  static void SetBrightness(uint8_t brightness);  
  static void DecreaseBrightness();
  static void IncreaseBrightness();
  static uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
  static uint32_t Wheel(uint8_t pos);

  std::vector<Animation*> animations;
  static uint8_t brightness;
  static absolute_time_t nextBrightnessChange;
  static absolute_time_t nextAnimationChange;
  uint32_t frame[100];
private:
  int numPixels = 0;
};

#endif
