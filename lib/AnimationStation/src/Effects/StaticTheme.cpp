#include "StaticTheme.hpp"

StaticTheme::StaticTheme(PixelMatrix &matrix, std::map<uint32_t, RGB> theme, RGB defaultColor)
  : Animation(matrix), theme(theme), defaultColor(defaultColor) { }

void StaticTheme::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      auto itr = theme.find(matrix->pixels[r][c].mask);
      if (itr != theme.end()) {
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
          frame[matrix->pixels[r][c].positions[p]] = itr->second;
        }
      }
      else {
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
          frame[matrix->pixels[r][c].positions[p]] = defaultColor;
        }
      }
    }
  }
}
