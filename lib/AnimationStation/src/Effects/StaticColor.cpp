 #include "StaticColor.hpp"

StaticColor::StaticColor(PixelMatrix &matrix) : Animation(matrix) {
}

StaticColor::StaticColor(PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pixels(&pixels) {
  this->filtered = true;
}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index || this->notInFilter(matrix->pixels[r][c]))
        continue;

      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
        frame[matrix->pixels[r][c].positions[p]] = colors[this->GetColor()];
      }
    }
  }
}

uint8_t StaticColor::GetColor() {
  if (this->filtered) {
    return AnimationStation::options.buttonColorIndex;
  }
  else {
    return AnimationStation::options.staticColorIndex;
  }
}

void StaticColor::ParameterUp() {
  uint8_t colorIndex;
  if (this->filtered) {
    colorIndex = AnimationStation::options.buttonColorIndex;
  }
  else {
    colorIndex = AnimationStation::options.staticColorIndex;
  }

  if (colorIndex < colors.size() - 1)
  {
    colorIndex++;
  }
  else {
    colorIndex = 0;
  }

  this->SaveIndexOptions(colorIndex);
}

void StaticColor::SaveIndexOptions(uint8_t colorIndex) {
  if (this->filtered) {
    AnimationStation::options.buttonColorIndex = colorIndex;
  }
  else {
    AnimationStation::options.staticColorIndex = colorIndex;
  }
}

void StaticColor::ParameterDown() {
  uint8_t colorIndex;
  if (this->filtered) {
    colorIndex = AnimationStation::options.buttonColorIndex;
  }
  else {
    colorIndex = AnimationStation::options.staticColorIndex;
  }

  if (colorIndex > 0) {
    colorIndex--;
  }
  else {
    colorIndex = colors.size() - 1;
  }
  this->SaveIndexOptions(colorIndex);
}
