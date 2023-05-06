#include "CustomTheme.hpp"

std::map<uint32_t, RGB> CustomTheme::theme;

CustomTheme::CustomTheme(PixelMatrix &matrix) : Animation(matrix) {

}

void CustomTheme::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
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

bool CustomTheme::HasTheme() {
  return CustomTheme::theme.size() > 0;
}

void CustomTheme::SetCustomTheme(std::map<uint32_t, RGB> customTheme) {
  CustomTheme::theme = customTheme;
  AnimationStation::effectCount = TOTAL_EFFECTS + 1;
}

void CustomTheme::ParameterUp() {

}

void CustomTheme::ParameterDown() {

}
