#include "animation.h"
#include "animationstation.h"
#include "storagemanager.h"

#define PRESS_COOLDOWN_INCREMENT 500
#define PRESS_COOLDOWN_MAX 5000
#define PRESS_COOLDOWN_MIN 0

LEDFormat Animation::format;
std::map<uint32_t, int32_t> Animation::times = {};
std::map<uint32_t, RGB> Animation::hitColor = {};

Animation::Animation(PixelMatrix &matrix) : matrix(&matrix) {
  for (size_t r = 0; r != matrix.pixels.size(); r++) {
    for (size_t c = 0; c != matrix.pixels[r].size(); c++) {
      if (matrix.pixels[r][c].index == NO_PIXEL.index)
        continue;
      times.insert_or_assign(matrix.pixels[r][c].index, 0);
      hitColor.insert_or_assign(matrix.pixels[r][c].index, defaultColor);
    }
  }
}

void Animation::UpdatePixels(std::vector<Pixel> inpixels) {
  this->pixels = inpixels;
}

void Animation::UpdateTime() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  coolDownTimeInMs = animationOptions.buttonPressColorCooldownTimeInMs;

  absolute_time_t currentTime = get_absolute_time();
  updateTimeInMs = absolute_time_diff_us(lastUpdateTime, currentTime) / 1000;
  lastUpdateTime = currentTime;
}

void Animation::UpdatePresses(RGB (&frame)[100]) {
  // Queue up blend on hit
  for (size_t p = 0; p < pixels.size(); p++) {
    if (pixels[p].index != NO_PIXEL.index) {
      times[pixels[p].index] = coolDownTimeInMs;
      hitColor[pixels[p].index] = frame[pixels[p].positions[0]];
    }
  }
}

void Animation::DecrementFadeCounter(int32_t index) {
  times[index] -= updateTimeInMs;
  if (times[index] < 0) {
    times[index] = 0;
  };
}

void Animation::ClearPixels() {
  this->pixels.clear();
}

/* Some of these animations are filtered to specific pixels, such as button press animations.
This somewhat backwards named method determines if a specific pixel is _not_ included in the filter */
bool Animation::notInFilter(Pixel pixel) {
  if (!this->filtered) {
    return false;
  }

  for (size_t i = 0; i < this->pixels.size(); i++) {
    if (pixel == this->pixels.at(i)) {
      return false;
    }
  }

  return true;
}

RGB Animation::BlendColor(RGB start, RGB end, uint32_t timeRemainingInMs) {
  RGB result = ColorBlack;

  if (timeRemainingInMs <= 0) {
    return end;
  }

  float progress = 1.0f - (static_cast<float>(timeRemainingInMs) / static_cast<float>(coolDownTimeInMs));
  if (progress < 0.0f) progress = 0.0f;
  if (progress > 1.0f) progress = 1.0f;

  result.r = static_cast<uint32_t>(static_cast<float>(start.r + (end.r - start.r) * progress));
  result.g = static_cast<uint32_t>(static_cast<float>(start.g + (end.g - start.g) * progress));
  result.b = static_cast<uint32_t>(static_cast<float>(start.b + (end.b - start.b) * progress));

  return result;
}


void Animation::FadeTimeUp() {
  AnimationOptions & anmationOptions = Storage::getInstance().getAnimationOptions();
  anmationOptions.buttonPressColorCooldownTimeInMs = anmationOptions.buttonPressColorCooldownTimeInMs + PRESS_COOLDOWN_INCREMENT;

  if (anmationOptions.buttonPressColorCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
    anmationOptions.buttonPressColorCooldownTimeInMs = PRESS_COOLDOWN_MAX;
  }
}

void Animation::FadeTimeDown() {
  AnimationOptions & anmationOptions = Storage::getInstance().getAnimationOptions();
  anmationOptions.buttonPressColorCooldownTimeInMs = anmationOptions.buttonPressColorCooldownTimeInMs - PRESS_COOLDOWN_INCREMENT;

  if (anmationOptions.buttonPressColorCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
    anmationOptions.buttonPressColorCooldownTimeInMs = PRESS_COOLDOWN_MIN;
  }
}
