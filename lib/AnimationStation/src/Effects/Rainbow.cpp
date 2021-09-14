#include "Rainbow.hpp"

int Rainbow::defaultCycleTime = 40;

Rainbow::Rainbow(std::vector<Pixel> pixels) : Animation(pixels) {
  this->cycleTime = Rainbow::defaultCycleTime;
}

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
