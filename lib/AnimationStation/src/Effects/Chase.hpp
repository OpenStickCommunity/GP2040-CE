#ifndef _CHASE_H_
#define _CHASE_H_

#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"
#include "hardware/clocks.h"

class Chase : public Animation {
public:
  Chase() : Animation() {
    this->cycleTime = Chase::defaultCycleTime;
    this->currentPixel = this->firstPixel;
    this->totalPixels = this->lastPixel - this->firstPixel + 1;
    this->mode = CHASE;
  }
  void Animate(uint32_t (&frame)[100]);

  static void SetDefaultCycleTime(int cycleTime) {
    Chase::defaultCycleTime = cycleTime;
  }
protected:
  static int defaultCycleTime;
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