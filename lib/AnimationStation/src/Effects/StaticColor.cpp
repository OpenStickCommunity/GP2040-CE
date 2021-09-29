#include "StaticColor.hpp"

RGB StaticColor::defaultColor = ColorRed;

StaticColor::StaticColor(Pixel pixel)
    : Animation(pixel), color(StaticColor::defaultColor) {}

StaticColor::StaticColor(std::vector<Pixel> pixels)
    : Animation(pixels), color(StaticColor::defaultColor) {}

StaticColor::StaticColor(std::vector<Pixel> pixels, bool ephemeral)
    : Animation(pixels), color(StaticColor::defaultColor) {
      this->baseAnimation = !ephemeral;
    }

StaticColor::StaticColor(std::vector<Pixel> pixels, RGB color)
    : Animation(pixels), color(color) {}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (size_t i = 0; i != pixels.size(); i++) {
    for (size_t j = 0; j != pixels[i].positions.size(); j++) {
      frame[pixels[i].positions[j]] = color;
    }
  }
}
