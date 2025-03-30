#include "rainbow.h"
#include "storagemanager.h"

Rainbow::Rainbow(PixelMatrix &matrix) : Animation(matrix) {
}

bool Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return false;
  }

  UpdateTime();
  UpdatePresses(frame);

  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      // Count down the timer
      DecrementFadeCounter(pixel.index);

      RGB color = RGB::wheel(this->currentFrame);
      for (auto &pos : pixel.positions)
        frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
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

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  this->nextRunTime = make_timeout_time_ms(animationOptions.rainbowCycleTime);

  return true;
}

// clamp rainbowCycleTime to [1 ... INT16_MAX]
#define RAINBOW_CYCLE_INCREMENT   10
#define RAINBOW_CYCLE_MAX         INT16_MAX - RAINBOW_CYCLE_INCREMENT
#define RAINBOW_CYCLE_MIN         1         + RAINBOW_CYCLE_INCREMENT

void Rainbow::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime < RAINBOW_CYCLE_MAX) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime + RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = INT16_MAX;
  }
}

void Rainbow::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime > RAINBOW_CYCLE_MIN) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime - RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = 1;
  }
}
