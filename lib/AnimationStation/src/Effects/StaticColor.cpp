#include <stdio.h>
#include <stdlib.h>

#include "StaticColor.hpp"

uint32_t StaticColor::defaultColor = 255;

void StaticColor::Animate(uint32_t (&frame)[100]) {
  float brightness = AnimationStation::GetBrightnessX();
  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    frame[i] = this->color * brightness;
  }
}
