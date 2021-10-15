#include "Rainbow.hpp"

Rainbow::Rainbow(PixelMatrix &matrix, uint16_t cycleTime) : Animation(matrix), cycleTime(cycleTime) {

}

void Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      RGB color = RGB::wheel(this->currentFrame);
      for (auto &pos : pixel.positions)
        frame[pos] = color;
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
