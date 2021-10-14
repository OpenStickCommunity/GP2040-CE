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
