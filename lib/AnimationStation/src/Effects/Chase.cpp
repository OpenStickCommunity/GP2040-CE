#include "Chase.hpp"

Chase::Chase(PixelMatrix &matrix) : Animation(matrix) {
}

void Chase::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      if (this->IsChasePixel(pixel.index)) {
        RGB color = RGB::wheel(this->WheelFrame(pixel.index));
        for (auto &pos : pixel.positions)
          frame[pos] = color;
      }
      else {
        for (auto &pos : pixel.positions)
          frame[pos] = ColorBlack;
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
    }
  } else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  this->nextRunTime = make_timeout_time_ms(AnimationStation::options.chaseCycleTime);
}

bool Chase::IsChasePixel(int i) {
  if (i == this->currentPixel || i == (this->currentPixel - 1) ||
      i == (this->currentPixel - 2)) {
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
    } else {
      frame = frame - 16;
    }
  }

  if (i == (this->currentPixel - 2) % pixelCount) {
    if (this->reverse) {
      frame = frame + 32;
    } else {
      frame = frame - 32;
    }
  }

  if (frame < 0) {
    return 0;
  }

  return frame;
}

// clamp chaseCycleTime to [1 ... INT16_MAX]
#define CHASE_CYCLE_INCREMENT   10
#define CHASE_CYCLE_MAX         INT16_MAX - CHASE_CYCLE_INCREMENT
#define CHASE_CYCLE_MIN         1         + CHASE_CYCLE_INCREMENT

void Chase::ParameterUp() {
  if (AnimationStation::options.chaseCycleTime < CHASE_CYCLE_MAX) {
    AnimationStation::options.chaseCycleTime = AnimationStation::options.chaseCycleTime + CHASE_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.chaseCycleTime = INT16_MAX;
  }
}

void Chase::ParameterDown() {
  if (AnimationStation::options.chaseCycleTime > CHASE_CYCLE_MIN) {
    AnimationStation::options.chaseCycleTime = AnimationStation::options.chaseCycleTime - CHASE_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.chaseCycleTime = 1;
  }
}
