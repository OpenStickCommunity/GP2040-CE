#include "Chase.hpp"

Chase::Chase(PixelMatrix &matrix, uint16_t cycleTime) : Animation(matrix), cycleTime(cycleTime) {

}

void Chase::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      if (this->IsChasePixel(matrix->pixels[r][c].index)) {
        RGB color = RGB::wheel(this->WheelFrame(matrix->pixels[r][c].index));
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
          frame[matrix->pixels[r][c].positions[p]] = color;
        }
      } else {
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
          frame[matrix->pixels[r][c].positions[p]] = ColorBlack;
        }
      }
    }
  }

  currentPixel++;

  if (currentPixel > matrix->getPixelCount() - 1) {
    currentPixel = 0;
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

bool Chase::IsChasePixel(int i) {
  if (i == this->currentPixel || i == (this->currentPixel - 1) || i == (this->currentPixel - 2)) {
    return true;
  }

  return false;
}

int Chase::WheelFrame(int i) {
  int frame = this->currentFrame;
  int pixelCount = matrix->getPixelCount();
  if (i == (this->currentPixel - 1) % pixelCount) {
    if (this->reverse) {
      frame = frame + 16;
    }
    else {
      frame = frame - 16;
    }
  }

  if (i == (this->currentPixel - 2) % pixelCount) {
    if (this->reverse) {
      frame = frame + 32;
    }
    else {
      frame = frame - 32;
    }
  }

  if (frame < 0) {
    return 0;
  }

  return frame;
}
