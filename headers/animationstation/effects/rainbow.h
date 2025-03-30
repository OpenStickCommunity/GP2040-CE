#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "animation.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "animationstation.h"

class Rainbow : public Animation {
public:
  Rainbow(PixelMatrix &matrix);
  ~Rainbow() {};

  bool Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

protected:
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = nil_time;
};

#endif
