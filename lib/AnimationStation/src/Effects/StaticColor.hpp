#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class StaticColor : public Animation {
public:
  StaticColor() : Animation() {
    this->color = StaticColor::defaultColor;
    this->mode = STATIC;
  }

  void Animate(uint32_t (&frame)[100]);

  static void SetDefaultColor(uint32_t color) {
    StaticColor::defaultColor = color;
  }
protected:
  static uint32_t defaultColor;
  uint32_t color;
};

#endif