#ifndef STATIC_THEME_H_
#define STATIC_THEME_H_

#include <iterator>
#include <map>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class StaticTheme : public Animation {
public:
  StaticTheme(PixelMatrix &matrix);
  ~StaticTheme() {};

  static void AddTheme(std::map<uint32_t, RGB> theme);
  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  RGB defaultColor = ColorBlack;
  static std::vector<std::map<uint32_t, RGB>> themes;
};

#endif
