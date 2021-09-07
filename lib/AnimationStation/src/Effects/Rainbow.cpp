#include <stdio.h>
#include <stdlib.h>

#include "Rainbow.hpp"

int Rainbow::defaultCycleTime = 40;

void Rainbow::Animate(uint32_t (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = AnimationStation::Wheel(this->currentFrame);
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 1;
      reverse = false;
    }
  }
  else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  this->nextRunTime = make_timeout_time_ms(this->cycleTime);
}
