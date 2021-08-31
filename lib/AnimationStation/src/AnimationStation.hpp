#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "Animation.hpp"
class AnimationStation
{
public:
  AnimationStation(int numPixels);
  void Animate();
  void SetStaticColor(bool defaultAnimation, uint32_t color, int firstPixel, int lastPixel);
  void SetRainbow(bool defaultAnimation, int firstPixel, int lastPixel, int cycleTime);
  void SetChase(bool defaultAnimation, int firstPixel, int lastPixel, int cycleTime);
  void Clear();

  static void SetBrightness(float brightness);
  static uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
  static uint32_t Wheel(uint8_t pos);
  
  std::vector<Animation*> animations;
  static float brightness;
  uint32_t frame[100];
private:
  int numPixels = 0;
};

#endif