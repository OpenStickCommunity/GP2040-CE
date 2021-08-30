#include "Helpers.hpp"

uint32_t Helpers::RGB(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

uint32_t Helpers::Wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return Helpers::RGB(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return Helpers::RGB(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return Helpers::RGB(pos * 3, 255 - pos * 3, 0);
  }
}
