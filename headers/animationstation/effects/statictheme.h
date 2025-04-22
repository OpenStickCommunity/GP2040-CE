#ifndef STATIC_THEME_H_
#define STATIC_THEME_H_

#include <iterator>
#include <map>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "animation.h"
#include "animationstation.h"

class StaticTheme : public Animation {
public:
  StaticTheme(PixelMatrix &matrix);
  ~StaticTheme() {};

  void AddTheme(const std::map<uint32_t, RGB>& theme) { themes.push_back(theme); }
  void ClearThemes() { themes.clear(); }
  bool Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  RGB defaultColor = ColorBlack;
  std::vector<std::map<uint32_t, RGB>> themes;
};

#endif
