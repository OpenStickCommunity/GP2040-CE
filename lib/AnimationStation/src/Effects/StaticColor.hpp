#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include "../Animation.hpp"

class StaticColor : public Animation {
public:
  StaticColor() : Animation() {

  }
  StaticColor(int firstPixel, int lastPixel, bool defaultAnimation, uint32_t color) : Animation(firstPixel, lastPixel, defaultAnimation) {
    this->color = color;
  }
  void Animate(uint32_t (&frame)[100]);
protected:
  uint32_t color;
};

#endif