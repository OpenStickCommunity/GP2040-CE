#include <stdio.h>
#include <stdlib.h>

#include "Rainbow.hpp"
#include "Helpers.hpp"

void Rainbow::Animate(uint32_t (&frame)[100]) {
	if (to_ms_since_boot(get_absolute_time()) - this->nextRunTime < 0)
		return;

  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = Helpers::Wheel(this->currentFrame);
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

	this->nextRunTime = to_ms_since_boot(get_absolute_time()) + this->cycleTime;
}
