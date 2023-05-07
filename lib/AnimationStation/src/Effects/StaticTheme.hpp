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

  static void AddTheme(const std::map<uint32_t, RGB>& theme) { themes.push_back(theme); }
  static void ClearThemes() { themes.clear(); }
  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  RGB defaultColor = ColorBlack;
  static std::vector<std::map<uint32_t, RGB>> themes;
};

#endif
