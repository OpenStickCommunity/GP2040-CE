#ifndef _CHASE_H_
#define _CHASE_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>


class Chase : public Animation {
public:
  Chase(std::vector<Pixel> pixels);
  Chase(std::vector<Pixel> pixels, uint16_t cycleTime);

  void Animate(RGB (&frame)[100]);
  static void SetDefaultCycleTime(uint16_t cycleTime) {
    defaultCycleTime = cycleTime;
  }

protected:
  bool IsChasePixel(int i);
  int WheelFrame(int i);

  static uint16_t defaultCycleTime;
  int16_t cycleTime;
  int currentFrame = 0;
  int currentPixel = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif
