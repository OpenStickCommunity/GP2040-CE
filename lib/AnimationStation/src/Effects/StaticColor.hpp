#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class StaticColor : public Animation {
public:
  StaticColor(PixelMatrix &matrix);
  StaticColor(PixelMatrix &matrix, std::vector<Pixel> &pixels);
  ~StaticColor() { pixels = nullptr; };

  void Animate(RGB (&frame)[100]);
  void SaveIndexOptions(uint8_t colorIndex);
  uint8_t GetColor();
  void ParameterUp();
  void ParameterDown();
protected:
  std::vector<Pixel> *pixels;
};

#endif
