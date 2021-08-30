#include <stdio.h>
#include <stdlib.h>

#include "Animation.hpp"

uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

uint32_t Wheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return RGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return RGB(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return RGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void StaticColor::Animate(uint32_t (&frame)[100]) {
  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = this->color;
  }
}

void Rainbow::Animate(uint32_t (&frame)[100]) {
	if (to_ms_since_boot(get_absolute_time()) - this->nextRunTime < 0)
		return;

  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = Wheel(this->currentFrame);
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 0;
      reverse = false;
    }
  }
  else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 255;
      reverse = true;
    }
  }

	this->nextRunTime = to_ms_since_boot(get_absolute_time()) + 50;
}
