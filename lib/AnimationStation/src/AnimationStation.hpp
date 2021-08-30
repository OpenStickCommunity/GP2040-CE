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
  void SetRainbow(bool defaultAnimation, int firstPixel, int lastPixel);
  void Clear();

  uint32_t frame[100];
private:
  int numPixels = 0;
  std::vector<Animation*> animations;
};

#endif