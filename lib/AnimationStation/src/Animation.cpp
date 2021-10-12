#include "Animation.hpp"

Animation::Animation(std::vector<Pixel> pixels) : pixels(pixels) {
}

void Animation::UpdatePixels(std::vector<Pixel> pixels) {
  this->pixels = pixels;
}

void Animation::ClearPixels() {
  this->pixels.clear();
}
