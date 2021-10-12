#include "StaticColor.hpp"

int StaticColor::defaultColorIndex = 2;

StaticColor::StaticColor(std::vector<Pixel> pixels)
    : Animation(pixels), colorIndex(StaticColor::defaultColorIndex) {}

StaticColor::StaticColor(std::vector<Pixel> pixels, int colorIndex)
    : Animation(pixels), colorIndex(colorIndex) {}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (size_t i = 0; i != pixels.size(); i++) {
    for (size_t j = 0; j != pixels[i].positions.size(); j++) {
      frame[pixels[i].positions[j]] = colors.at(this->colorIndex);
    }
  }
}

void StaticColor::ParameterUp() {
  if (this->colorIndex < colors.size() - 1)
  {
    this->colorIndex++;
  }
  else {
    this->colorIndex = 0;
  }
}

void StaticColor::ParameterDown() {
  if (this->colorIndex > 0) {
    this->colorIndex--;
  }
  else {
    this->colorIndex = colors.size() - 1;
  }
}
