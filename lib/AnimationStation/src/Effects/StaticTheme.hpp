#ifndef STATIC_THEME_H_
#define STATIC_THEME_H_

#include <iterator>
#include <map>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"

class StaticTheme : public Animation {
public:
  StaticTheme(std::vector<Pixel> pixels, std::map<uint32_t, RGB> theme, RGB defaultColor = ColorBlack);

  void Animate(RGB (&frame)[100]);
  std::map<uint32_t, RGB> theme;
protected:
  RGB defaultColor;
};

#endif
