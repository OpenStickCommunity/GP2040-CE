#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"
#include "hardware/clocks.h"

class Rainbow : public Animation {
public:
  Rainbow() : Animation() {
    this->cycleTime = Rainbow::defaultCycleTime;
    this->mode = RAINBOW;
  }
  void Animate(uint32_t (&frame)[100]);

  static void SetDefaultCycleTime(int cycleTime) {
    Rainbow::defaultCycleTime = cycleTime;
  }
protected:
  static int defaultCycleTime;
  int cycleTime;
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif