#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "hardware/clocks.h"

class Rainbow : public Animation {
public:
  Rainbow(PixelMatrix &matrix, uint16_t cycleTime = 40);

  void Animate(RGB (&frame)[100]);
protected:
  uint16_t cycleTime;
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif
