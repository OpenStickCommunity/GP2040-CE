#ifndef _CHASE_H_
#define _CHASE_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "hardware/clocks.h"

class Chase : public Animation {
public:
  Chase(PixelMatrix &matrix, uint16_t cycleTime = 85);

  void Animate(RGB (&frame)[100]);

protected:
  bool IsChasePixel(int i);
  int WheelFrame(int i);

  int16_t cycleTime;
  int currentFrame = 0;
  int currentPixel = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = 0;
};

#endif
