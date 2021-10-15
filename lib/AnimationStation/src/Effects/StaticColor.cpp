 #include "StaticColor.hpp"

StaticColor::StaticColor(PixelMatrix &matrix, RGB color) : Animation(matrix), color(color) {

}

void StaticColor::Animate(RGB (&frame)[100]) {
  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      for (auto &pos : pixel.positions)
        frame[pos] = color;
    }
  }
}
