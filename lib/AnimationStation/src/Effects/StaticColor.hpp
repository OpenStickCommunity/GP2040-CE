#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"

class StaticColor : public Animation {
public:
  StaticColor(PixelMatrix &matrix, RGB color);

  void Animate(RGB (&frame)[100]);

  void SetColor(RGB color) {
    this->color = color;
  }
protected:
  RGB color;
};

#endif
