#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "hardware/clocks.h"

class Rainbow : public Animation {
public:
  Rainbow(std::vector<Pixel> pixels);

  void Animate(RGB (&frame)[100]);

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
