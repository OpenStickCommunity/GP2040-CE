#include "Animation.hpp"
#include <algorithm>

#include "AnimationStation.hpp"


#define PRESS_COOLDOWN_INCREMENT 500
#define PRESS_COOLDOWN_MAX 5000
#define PRESS_COOLDOWN_MIN 0

LEDFormat Animation::format;
std::map<uint32_t, LedRGBState> Animation::ledRGBStates;

Animation::Animation(PixelMatrix &inMatrix) : matrix(&inMatrix) {
  for (auto &pixel_row : matrix->pixels) {
    for (auto &pixel: pixel_row) {
      if (pixel.index != NO_PIXEL.index) {
        ledRGBStates.insert_or_assign(pixel.index, LedRGBState{defaultColor, 0});
      }
    }
  }
}

void Animation::UpdatePixels(const std::vector<Pixel>& inpixels) {
  this->pixels = inpixels;
}

void Animation::UpdateTime() {
  coolDownTimeInMs = AnimationStation::options.buttonPressColorCooldownTimeInMs;

  absolute_time_t currentTime = get_absolute_time();
  updateTimeInMs = absolute_time_diff_us(lastUpdateTime, currentTime) / 1000;
  lastUpdateTime = currentTime;
}

void Animation::UpdatePresses(RGB (&frame)[100]) {
  // Queue up blend on hit
  for (auto &pixel : pixels) {
    if (pixel.index != NO_PIXEL.index) {
      auto& ledRGBState = ledRGBStates[pixel.index];
      ledRGBState.Time = coolDownTimeInMs;
      ledRGBState.HitColor = frame[pixel.positions[0]];
    }
  }
}

void Animation::DecrementFadeCounter(int32_t index) {
  auto& ledRGBState = ledRGBStates[index];
  ledRGBState.Time -= updateTimeInMs;
  if (ledRGBState.Time < 0) {
    ledRGBState.Time = 0;
  };
}

void Animation::ClearPixels() {
  this->pixels.clear();
}

/* Some of these animations are filtered to specific pixels, such as button press animations.
This somewhat backwards named method determines if a specific pixel is _not_ included in the filter */
bool Animation::notInFilter(const Pixel& pixel) {
  if (!this->filtered) {
    return false;
  }

  return std::none_of(this->pixels.cbegin(), this->pixels.cend(), [pixel](const Pixel& internalPixel){
    return pixel == internalPixel;
  });
}

RGB Animation::BlendColor(const RGB& start, const RGB& end, uint32_t timeRemainingInMs) {
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
  AnimationStation::options.buttonPressColorCooldownTimeInMs = AnimationStation::options.buttonPressColorCooldownTimeInMs + PRESS_COOLDOWN_INCREMENT;

  if (AnimationStation::options.buttonPressColorCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
    AnimationStation::options.buttonPressColorCooldownTimeInMs = PRESS_COOLDOWN_MAX;
  }
}

void Animation::FadeTimeDown() {
  AnimationStation::options.buttonPressColorCooldownTimeInMs = AnimationStation::options.buttonPressColorCooldownTimeInMs - PRESS_COOLDOWN_INCREMENT;

  if (AnimationStation::options.buttonPressColorCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
    AnimationStation::options.buttonPressColorCooldownTimeInMs = PRESS_COOLDOWN_MIN;
  }
}

const LedRGBState& Animation::GetLedRGBStateAtIndex(uint32_t index) {
  return ledRGBStates[index];
}
