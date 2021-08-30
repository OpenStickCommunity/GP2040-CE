#include <stdio.h>
#include <stdlib.h>

#include "Animation.hpp"

void StaticColor::Animate(uint32_t (&frame)[100]) {
  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = this->color;
  }
}
