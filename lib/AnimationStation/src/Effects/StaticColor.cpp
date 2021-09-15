#include "StaticColor.hpp"

StaticColor::StaticColor(std::vector<Pixel> pixels, RGB color) : Animation(pixels), color(color) {

}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (size_t i = 0; i != pixels.size(); i++) {
    for (size_t j = 0; j != pixels[i].positions.size(); j++) {
      frame[pixels[i].positions[j]] = color;
    }
  }
}
