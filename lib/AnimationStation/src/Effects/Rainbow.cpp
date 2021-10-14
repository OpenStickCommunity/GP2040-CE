#include "Rainbow.hpp"

Rainbow::Rainbow(PixelMatrix &matrix, uint16_t cycleTime) : Animation(matrix), cycleTime(cycleTime) {

}

void Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      RGB color = RGB::wheel(this->currentFrame);
      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
        frame[matrix->pixels[r][c].positions[p]] = color;
      }
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
