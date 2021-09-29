#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>


class Rainbow : public Animation {
public:
  Rainbow(std::vector<Pixel> pixels);
  Rainbow(std::vector<Pixel> pixels, uint16_t cycleTime);

  void Animate(RGB (&frame)[100]);
  static void SetDefaultCycleTime(uint16_t cycleTime) {
    defaultCycleTime = cycleTime;
  }

protected:
  static uint16_t defaultCycleTime;
  uint16_t cycleTime;
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif
