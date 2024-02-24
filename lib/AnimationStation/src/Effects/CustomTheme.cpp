#include "CustomTheme.hpp"

std::map<uint32_t, RGB> CustomTheme::theme;

CustomTheme::CustomTheme(PixelMatrix &inMatrix) : Animation(inMatrix) {
}

void CustomTheme::Animate(RGB (&frame)[100]) {
  UpdateTime();
  UpdatePresses(frame);

  for (auto &pixel_row : matrix->pixels) {
    for (auto &pixel : pixel_row) {
      if (pixel.index == NO_PIXEL.index) {
        continue;
      }

      // Count down the timer
      DecrementFadeCounter(pixel.index);

      auto itr = theme.find(pixel.mask);
      if (itr != theme.end()) {
        auto& ledRGBState = GetLedRGBStateAtIndex(pixel.index);

        for (auto& position : pixel.positions) {
          // Interpolate from hitColor (color the button was assigned when pressed) back to the theme color
          frame[position] = BlendColor(ledRGBState.HitColor, itr->second, ledRGBState.Time);
        }
      }
      else {
        for (auto& position : pixel.positions) {
          frame[position] = defaultColor;
        }
      }
    }
  }
}

bool CustomTheme::HasTheme() {
  return CustomTheme::theme.size() > 0;
}

void CustomTheme::SetCustomTheme(const std::map<uint32_t, RGB>& customTheme) {
  CustomTheme::theme = customTheme;
  AnimationStation::effectCount = TOTAL_EFFECTS + 1;
}

void CustomTheme::ParameterUp() {
}

void CustomTheme::ParameterDown() {
}
