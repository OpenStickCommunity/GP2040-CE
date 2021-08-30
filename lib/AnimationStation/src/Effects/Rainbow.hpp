#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "../Animation.hpp"

class Rainbow : public Animation {
public:
  Rainbow() : Animation() {
  }
  Rainbow(int firstPixel, int lastPixel, int cycleTime = 50, bool defaultAnimation = false) : Animation(firstPixel, lastPixel, defaultAnimation) {
    this->cycleTime = cycleTime;
  }
  void Animate(uint32_t (&frame)[100]);
protected:
  int cycleTime;
  int currentFrame = 0;
  bool reverse = false;
  uint32_t nextRunTime = 0;
};

#endif