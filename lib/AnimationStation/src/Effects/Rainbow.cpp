#include "Rainbow.hpp"

uint16_t Rainbow::defaultCycleTime = 40;

Rainbow::Rainbow(std::vector<Pixel> pixels)
    : Animation(pixels), cycleTime(Rainbow::defaultCycleTime) {}

Rainbow::Rainbow(std::vector<Pixel> pixels, uint16_t cycleTime)
    : Animation(pixels), cycleTime(cycleTime) {}

void Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (size_t i = 0; i != pixels.size(); i++) {
    RGB color = RGB::wheel(this->currentFrame);
    for (size_t j = 0; j != pixels[i].positions.size(); j++) {
      frame[pixels[i].positions[j]] = color;
    }
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 1;
      reverse = false;
      this->currentLoop++;
    }
  } else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  this->nextRunTime = make_timeout_time_ms(this->cycleTime);
}
