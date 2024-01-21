#include "StaticColor.hpp"

StaticColor::StaticColor(PixelMatrix &matrix) : Animation(matrix) {
}

StaticColor::StaticColor(PixelMatrix &matrix, std::vector<Pixel> &inpixels) : Animation(matrix) {
  this->filtered = true;
  pixels = inpixels;
}

void StaticColor::Animate(RGB (&frame)[100]) {
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
