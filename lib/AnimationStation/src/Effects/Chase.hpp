#ifndef _CHASE_H_
#define _CHASE_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>


class Chase : public Animation {
public:
  Chase(PixelMatrix &matrix, uint16_t cycleTime = 85);
  ~Chase() {};

  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

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
