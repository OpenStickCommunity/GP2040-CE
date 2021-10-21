#include "StaticTheme.hpp"

StaticTheme::StaticTheme(PixelMatrix &matrix, std::map<uint32_t, RGB> theme, RGB defaultColor)
  : Animation(matrix), theme(theme), defaultColor(defaultColor) { }

void StaticTheme::Animate(RGB (&frame)[100]) {
  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      auto itr = theme.find(pixel.mask);
      if (itr != theme.end()) {
        for (auto &pos : pixel.positions)
          frame[pos] = itr->second;
      }
      else {
        for (auto &pos : pixel.positions)
          frame[pos] = defaultColor;
      }
    }
  }
}

void StaticTheme::ParameterUp() {

}

void StaticTheme::ParameterDown() {
  
}