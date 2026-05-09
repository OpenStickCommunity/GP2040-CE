#include "staticcolor.h"
#include "storagemanager.h"

StaticColor::StaticColor(PixelMatrix &matrix) : Animation(matrix) {
}

StaticColor::StaticColor(PixelMatrix &matrix, std::vector<Pixel> &inpixels) : Animation(matrix) {
  this->filtered = true;
  pixels = inpixels;
}

bool StaticColor::Animate(RGB (&frame)[NEOPICO_MAX_LEDS]) {
  UpdateTime();
  UpdatePresses(frame);

  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index || this->notInFilter(matrix->pixels[r][c]))
        continue;

      // Count down the timer
      DecrementFadeCounter(matrix->pixels[r][c].index);

      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
        // Interpolate from hitColor (color the button was assigned when pressed) back to the theme color
        if (!this->filtered) {
          frame[matrix->pixels[r][c].positions[p]] = BlendColor(hitColor[matrix->pixels[r][c].index], colors[this->GetColor()], times[matrix->pixels[r][c].index]);
        } else {
          frame[matrix->pixels[r][c].positions[p]] = colors[this->GetColor()];
        }
      }
    }
  }
  return true;
}

uint8_t StaticColor::GetColor() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  uint32_t idx = this->filtered
    ? animationOptions.buttonColorIndex
    : animationOptions.staticColorIndex;
  // colors[] has fixed size; a corrupt / stale stored index would otherwise
  // run colors[idx] (operator[]) past its internal buffer in Animate().
  if (colors.empty()) return 0;
  if (idx >= colors.size()) idx = 0;
  return static_cast<uint8_t>(idx);
}

void StaticColor::ParameterUp() {
  uint8_t colorIndex;
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    colorIndex = animationOptions.buttonColorIndex;
  } else {
    colorIndex = animationOptions.staticColorIndex;
  }

  if (colorIndex < colors.size() - 1) {
    colorIndex++;
  } else {
    colorIndex = 0;
  }

  this->SaveIndexOptions(colorIndex);
}

void StaticColor::SaveIndexOptions(uint8_t colorIndex) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    animationOptions.buttonColorIndex = colorIndex;
  } else {
    animationOptions.staticColorIndex = colorIndex;
  }
}

void StaticColor::ParameterDown() {
  uint8_t colorIndex;
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    colorIndex = animationOptions.buttonColorIndex;
  } else {
    colorIndex = animationOptions.staticColorIndex;
  }

  if (colorIndex > 0) {
    colorIndex--;
  } else {
    colorIndex = colors.size() - 1;
  }
  this->SaveIndexOptions(colorIndex);
}
