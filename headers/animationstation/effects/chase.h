#ifndef _CHASE_H_
#define _CHASE_H_

#include "animation.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "animationstation.h"

class Chase : public Animation {
public:
  Chase(PixelMatrix &matrix);
  ~Chase() {};

  bool Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

protected:
  bool IsChasePixel(int i);
  int WheelFrame(int i);
  int currentFrame = 0;
  int currentPixel = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = nil_time;
};

#endif
