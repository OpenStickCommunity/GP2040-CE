#include "StaticColor.hpp"

StaticColor::StaticColor(PixelMatrix &inMatrix) : Animation(inMatrix) {
}

StaticColor::StaticColor(PixelMatrix& inMatrix, const std::vector<Pixel>& inPixels) : Animation(inMatrix) {
  this->filtered = true;
  pixels = inPixels;
}

void StaticColor::Animate(RGB (&frame)[100]) {
  UpdateTime();
  UpdatePresses(frame);

  for (auto &pixel_row : matrix->pixels) {
    for (auto &pixel : pixel_row) {
      if (pixel.index == NO_PIXEL.index || this->notInFilter(pixel))
        continue;

      // Count down the timer
      DecrementFadeCounter(pixel.index);

      for (auto &position : pixel.positions) {
        // Interpolate from hitColor (color the button was assigned when pressed) back to the theme color
        if (!this->filtered) {
          auto& ledRGBState = GetLedRGBStateAtIndex(pixel.index);
          frame[position] = BlendColor(ledRGBState.HitColor, colors[this->GetColor()], ledRGBState.Time);
        } else {
          frame[position] = colors[this->GetColor()];
        }
      }
    }
  }
}

uint8_t StaticColor::GetColor() {
  if (this->filtered) {
    return AnimationStation::options.buttonColorIndex;
  } else {
    return AnimationStation::options.staticColorIndex;
  }
}

void StaticColor::ParameterUp() {
  uint8_t colorIndex;
  if (this->filtered) {
    colorIndex = AnimationStation::options.buttonColorIndex;
  } else {
    colorIndex = AnimationStation::options.staticColorIndex;
  }

  if (colorIndex < colors.size() - 1) {
    colorIndex++;
  } else {
    colorIndex = 0;
  }

  this->SaveIndexOptions(colorIndex);
}

void StaticColor::SaveIndexOptions(uint8_t colorIndex) {
  if (this->filtered) {
    AnimationStation::options.buttonColorIndex = colorIndex;
  } else {
    AnimationStation::options.staticColorIndex = colorIndex;
  }
}

void StaticColor::ParameterDown() {
  uint8_t colorIndex;
  if (this->filtered) {
    colorIndex = AnimationStation::options.buttonColorIndex;
  } else {
    colorIndex = AnimationStation::options.staticColorIndex;
  }

  if (colorIndex > 0) {
    colorIndex--;
  } else {
    colorIndex = colors.size() - 1;
  }
  this->SaveIndexOptions(colorIndex);
}
