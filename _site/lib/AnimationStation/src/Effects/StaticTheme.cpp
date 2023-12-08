#include "StaticTheme.hpp"

std::vector<std::map<uint32_t, RGB>> StaticTheme::themes = {};

StaticTheme::StaticTheme(PixelMatrix &matrix) : Animation(matrix) {
  if (AnimationStation::options.themeIndex >= StaticTheme::themes.size()) {
    AnimationStation::options.themeIndex = 0;
  }
}

void StaticTheme::Animate(RGB (&frame)[100]) {
  if (StaticTheme::themes.size() > 0) {
    for (size_t r = 0; r != matrix->pixels.size(); r++) {
      for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
        if (matrix->pixels[r][c].index == NO_PIXEL.index)
          continue;

        std::map<uint32_t, RGB> theme =
            StaticTheme::themes.at(AnimationStation::options.themeIndex);
        auto itr = theme.find(matrix->pixels[r][c].mask);
        if (itr != theme.end()) {
          for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
            frame[matrix->pixels[r][c].positions[p]] = itr->second;
          }
        } else {
          for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
            frame[matrix->pixels[r][c].positions[p]] = defaultColor;
          }
        }
      }
    }
  }
}

void StaticTheme::ParameterUp() {
  if (AnimationStation::options.themeIndex < StaticTheme::themes.size() - 1) {
    AnimationStation::options.themeIndex++;
  } else {
    AnimationStation::options.themeIndex = 0;
  }
}

void StaticTheme::ParameterDown() {

  if (AnimationStation::options.themeIndex > 0) {
    AnimationStation::options.themeIndex--;
  } else {
    AnimationStation::options.themeIndex = StaticTheme::themes.size() - 1;
  }
}
