#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "animation.h"
#include "animationstation.h"

class StaticColor : public Animation {
public:
  StaticColor(PixelMatrix &matrix);
  StaticColor(PixelMatrix &matrix, std::vector<Pixel> &pixels);
  ~StaticColor() { };

  bool Animate(RGB (&frame)[100]);
  void SaveIndexOptions(uint8_t colorIndex);
  uint8_t GetColor();
  void ParameterUp();
  void ParameterDown();
protected:
};

#endif
