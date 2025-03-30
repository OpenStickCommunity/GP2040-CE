#ifndef _CUSTOM_THEME_PRESSED_H_
#define _CUSTOM_THEME_PRESSED_H_

#include <map>
#include <vector>
#include "animation.h"
#include "animationstation.h"

class CustomThemePressed : public Animation {
public:
  CustomThemePressed(PixelMatrix &matrix);
  CustomThemePressed(PixelMatrix &matrix, std::vector<Pixel> &pixels);
  ~CustomThemePressed() { pixels = nullptr; };
  bool HasTheme();
  bool Animate(RGB (&frame)[100]);
  void ParameterUp() { }
  void ParameterDown() { }
protected:
  std::vector<Pixel> *pixels;
  RGB defaultColor = ColorBlack;
  std::map<uint32_t, RGB> theme;
};

#endif
