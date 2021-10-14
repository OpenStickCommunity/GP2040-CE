#ifndef PIXEL_HPP_
#define PIXEL_HPP_

struct Pixel {
  Pixel(int index, uint32_t mask = 0) : index(index), mask(mask) { }
  Pixel(int index, std::vector<uint8_t> positions) : index(index), positions(positions) { }
  Pixel(int index, uint32_t mask, std::vector<uint8_t> positions) : index(index), mask(mask), positions(positions) { }

  int index;                      // The pixel index
  uint32_t mask;                  // Used to detect per-pixel lighting
  std::vector<uint8_t> positions; // The actual LED indexes on the chain
};

const Pixel NO_PIXEL(-1);

struct PixelMatrix {
  PixelMatrix(std::vector<std::vector<Pixel>> pixels, int ledsPerPixel = -1) : pixels(pixels), ledsPerPixel(ledsPerPixel) {

  }

  std::vector<std::vector<Pixel>> pixels;
  uint8_t ledsPerPixel;

  inline int getLedCount() {
    int count = 0;
    for (int c = 0; c != pixels.size(); c++)
      for (int r = 0; r != pixels[c].size(); r++)
        if (pixels[c][r].index == NO_PIXEL.index)
          continue;
        else
          count += pixels[c][r].positions.size();

    return count;
  }

  inline uint16_t getPixelCount() {
    uint16_t count = 0;
    for (size_t r = 0; r != pixels.size(); r++) {
      count += pixels[r].size();
    }

    return count;
  }

  void updatePositions() {
    if (ledsPerPixel > 0) {
      for (auto &col : pixels) {
        for (auto &pixel : col) {
          if (pixel.index == NO_PIXEL.index)
            continue;

          pixel.positions.resize(ledsPerPixel);
          for (int p = 0; p != ledsPerPixel; p++)
            pixel.positions[p] = (pixel.index * ledsPerPixel) + p;
        }
      }
    }
  }

};

#endif
