#ifndef _CHASE_H_
#define _CHASE_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "hardware/clocks.h"

class Chase : public Animation {
public:
  Chase(std::vector<Pixel> pixels);

  void Animate(RGB (&frame)[100]);

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
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif
