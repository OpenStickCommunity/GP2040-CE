#include "CustomThemePressed.hpp"

std::map<uint32_t, RGB> CustomThemePressed::theme;

CustomThemePressed::CustomThemePressed(PixelMatrix &inMatrix) : Animation(inMatrix) {
  this->filtered = true;
}

CustomThemePressed::CustomThemePressed(PixelMatrix &inMatrix, const std::vector<Pixel> &inPixels) : Animation(inMatrix) {
  this->filtered = true;
  this->pixels = inPixels;
}

void CustomThemePressed::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index || this->notInFilter(matrix->pixels[r][c]))
        continue;

      auto itr = theme.find(matrix->pixels[r][c].mask);
      if (itr != theme.end())
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++)
          frame[matrix->pixels[r][c].positions[p]] = itr->second;
      else
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++)
          frame[matrix->pixels[r][c].positions[p]] = defaultColor;
    }
  }
}

bool CustomThemePressed::HasTheme() {
  return CustomThemePressed::theme.size() > 0;
}

void CustomThemePressed::SetCustomTheme(const std::map<uint32_t, RGB>& customTheme) {
  CustomThemePressed::theme = customTheme;
}
