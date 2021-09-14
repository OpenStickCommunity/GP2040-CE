#include "StaticTheme.hpp"

StaticTheme::StaticTheme(std::vector<Pixel> pixels, std::map<uint32_t, RGB> theme, RGB defaultColor)
  : Animation(pixels), theme(theme), defaultColor(defaultColor) { }

void StaticTheme::Animate(RGB (&frame)[100]) {
  for (size_t i = 0; i != pixels.size(); i++) {
    bool found = false;

    auto itr = theme.find(pixels[i].mask);
    if (itr != theme.end()) {
      for (size_t j = 0; j != pixels[i].positions.size(); j++) {
        frame[pixels[i].positions[j]] = itr->second;
      }
    }
    else {
      for (size_t j = 0; j != pixels[i].positions.size(); j++) {
        frame[pixels[i].positions[j]] = defaultColor;
      }
    }
  }
}
