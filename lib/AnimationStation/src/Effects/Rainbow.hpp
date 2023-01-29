#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../AnimationStation.hpp"

class Rainbow : public Animation {
public:
  Rainbow(PixelMatrix &matrix);
  ~Rainbow() {};

  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

protected:
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = nil_time;
};

#endif
