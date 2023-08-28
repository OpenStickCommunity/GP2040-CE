#include "Rainbow.hpp"

Rainbow::Rainbow(PixelMatrix &matrix) : Animation(matrix) {
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
    }
  } else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  this->nextRunTime = make_timeout_time_ms(AnimationStation::options.rainbowCycleTime);
}

#define RAINBOW_CYCLE_INCREMENT 10
void Rainbow::ParameterUp() {
  if (AnimationStation::options.chaseCycleTime < 65535 - RAINBOW_CYCLE_INCREMENT) {
    AnimationStation::options.rainbowCycleTime = AnimationStation::options.rainbowCycleTime + RAINBOW_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.rainbowCycleTime = 65535;
  }
}

void Rainbow::ParameterDown() {
  if (AnimationStation::options.rainbowCycleTime > RAINBOW_CYCLE_INCREMENT) {
    AnimationStation::options.rainbowCycleTime = AnimationStation::options.rainbowCycleTime - RAINBOW_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.rainbowCycleTime = 1;
  }
}
#undef RAINBOW_CYCLE_INCREMENT
