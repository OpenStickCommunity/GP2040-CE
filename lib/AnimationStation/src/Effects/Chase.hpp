#ifndef _CHASE_H_
#define _CHASE_H_

#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"

class Chase : public Animation {
public:
  Chase() : Animation() {
  }
  Chase(int firstPixel, int lastPixel, int cycleTime = 50, bool defaultAnimation = false) : Animation(firstPixel, lastPixel, defaultAnimation) {
    this->cycleTime = cycleTime;
    this->currentPixel = firstPixel;
    this->totalPixels = this->lastPixel - this->firstPixel + 1;
  }
  void Animate(uint32_t (&frame)[100]);
protected:
  bool IsChasePixel(int i);
  int WheelFrame(int i);

  int cycleTime;
  int currentFrame = 0;
  int currentPixel = 0;
  int totalPixels;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif