#include "StaticColor.hpp"

StaticColor::StaticColor(PixelMatrix &matrix, RGB color) : Animation(matrix), color(color) {

}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
        frame[matrix->pixels[r][c].positions[p]] = color;
      }
    }
  }
}
