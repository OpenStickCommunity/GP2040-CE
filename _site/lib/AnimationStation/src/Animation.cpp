#include "Animation.hpp"

LEDFormat Animation::format;

Animation::Animation(PixelMatrix &matrix) : matrix(&matrix) {
}

void Animation::UpdatePixels(std::vector<Pixel> pixels) {
  this->pixels = pixels;
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
