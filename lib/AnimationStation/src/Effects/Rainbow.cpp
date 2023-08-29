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

// clamp rainbowCycleTime to [1 ... INT16_MAX]
#define RAINBOW_CYCLE_INCREMENT   10
#define RAINBOW_CYCLE_MAX         INT16_MAX - RAINBOW_CYCLE_INCREMENT
#define RAINBOW_CYCLE_MIN         1         + RAINBOW_CYCLE_INCREMENT

void Rainbow::ParameterUp() {
  if (AnimationStation::options.rainbowCycleTime < RAINBOW_CYCLE_MAX) {
    AnimationStation::options.rainbowCycleTime = AnimationStation::options.rainbowCycleTime + RAINBOW_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.rainbowCycleTime = INT16_MAX;
  }
}

void Rainbow::ParameterDown() {
  if (AnimationStation::options.rainbowCycleTime > RAINBOW_CYCLE_MIN) {
    AnimationStation::options.rainbowCycleTime = AnimationStation::options.rainbowCycleTime - RAINBOW_CYCLE_INCREMENT;
  } else {
    AnimationStation::options.rainbowCycleTime = 1;
  }
}
