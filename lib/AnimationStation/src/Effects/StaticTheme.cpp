#include "StaticTheme.hpp"

std::vector<std::map<uint32_t, RGB>> StaticTheme::themes = {};

StaticTheme::StaticTheme(PixelMatrix &inMatrix) : Animation(inMatrix) {
  if (AnimationStation::options.themeIndex >= StaticTheme::themes.size()) {
    AnimationStation::options.themeIndex = 0;
  }
}

void StaticTheme::Animate(RGB (&frame)[100]) {
  if (StaticTheme::themes.size() > 0) {
    UpdateTime();
    UpdatePresses(frame);

    for (auto &pixel_row : matrix->pixels) {
      for (auto &pixel : pixel_row) {
        if (pixel.index == NO_PIXEL.index)
          continue;

        // Count down the timer
        DecrementFadeCounter(pixel.index);

        std::map<uint32_t, RGB> theme = StaticTheme::themes.at(AnimationStation::options.themeIndex);
        auto itr = theme.find(pixel.mask);
        if (itr != theme.end()) {
          for (auto &position : pixel.positions) {
            auto& ledRGBState = GetLedRGBStateAtIndex(pixel.index);
            frame[position] = BlendColor(ledRGBState.HitColor, itr->second, ledRGBState.Time);
          }
        } else {
          for (auto &position : pixel.positions) {
            frame[position] = defaultColor;
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
