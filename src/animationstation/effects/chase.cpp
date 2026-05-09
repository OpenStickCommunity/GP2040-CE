#include "chase.h"
#include "storagemanager.h"

#define CHASE_CYCLE_INCREMENT   10
#define CHASE_CYCLE_MAX         INT16_MAX/2
#define CHASE_CYCLE_MIN         10

Chase::Chase(PixelMatrix &matrix) : Animation(matrix) {
}

bool Chase::Animate(RGB (&frame)[NEOPICO_MAX_LEDS]) {
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

      if (this->IsChasePixel(pixel.index)) {
        RGB color = RGB::wheel(this->WheelFrame(pixel.index));
        for (auto &pos : pixel.positions)
          frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
      } else {
        for (auto &pos : pixel.positions)
          frame[pos] = BlendColor(hitColor[pixel.index], ColorBlack, times[pixel.index]);
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

  // this really shouldn't be nessecary, but something outside the param down might be changing this
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.chaseCycleTime < CHASE_CYCLE_MIN) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MIN;
  } else if (animationOptions.chaseCycleTime > CHASE_CYCLE_MAX) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MAX;
  }

  this->nextRunTime = make_timeout_time_ms(animationOptions.chaseCycleTime);

  return true;
}

// Modulo that always returns a non-negative result. Standard `%` on negative ints
// gives an implementation-defined / negative answer, which previously meant the
// trailing pixels at currentPixel-1 / currentPixel-2 never matched any real pixel
// when currentPixel was 0 or 1 (chase tail visually disappeared at the wrap).
static inline int wrapMod(int n, int m) {
  if (m <= 0) return 0;
  int r = n % m;
  return (r < 0) ? r + m : r;
}

bool Chase::IsChasePixel(int i) {
  const int pixelCount = (int)matrix->getPixelCount();
  if (pixelCount <= 0) return false;
  return i == this->currentPixel
      || i == wrapMod(this->currentPixel - 1, pixelCount)
      || i == wrapMod(this->currentPixel - 2, pixelCount);
}

int Chase::WheelFrame(int i) {
  int frame = this->currentFrame;
  const int pixelCount = (int)matrix->getPixelCount();
  if (pixelCount <= 0) return frame;

  if (i == wrapMod(this->currentPixel - 1, pixelCount)) {
    frame += this->reverse ? 16 : -16;
  }
  if (i == wrapMod(this->currentPixel - 2, pixelCount)) {
    frame += this->reverse ? 32 : -32;
  }

  // Don't clamp to 255: RGB::wheel() takes uint8_t, and letting the value wrap
  // around the color wheel keeps the chase tail visually continuous. Only guard
  // the negative case, which would cast to a large unsigned value.
  if (frame < 0) {
    return 0;
  }
  return frame;
}

void Chase::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.chaseCycleTime = animationOptions.chaseCycleTime + CHASE_CYCLE_INCREMENT;
  if (animationOptions.chaseCycleTime > CHASE_CYCLE_MAX) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MAX;
  }
}

void Chase::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.chaseCycleTime =animationOptions.chaseCycleTime - CHASE_CYCLE_INCREMENT;
  if (animationOptions.chaseCycleTime < CHASE_CYCLE_MIN) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MIN;
  }
}
