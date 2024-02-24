#ifndef _CUSTOM_THEME_PRESSED_H_
#define _CUSTOM_THEME_PRESSED_H_

#include <map>
#include <vector>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class CustomThemePressed : public Animation {
public:
  CustomThemePressed(PixelMatrix &inMatrix);
  CustomThemePressed(PixelMatrix &inMatrix, const std::vector<Pixel>& inPixels);

  static bool HasTheme();
  static void SetCustomTheme(std::map<uint32_t, RGB> customTheme);
  void Animate(RGB (&frame)[100]);
  void ParameterUp() { }
  void ParameterDown() { }
protected:
  RGB defaultColor = ColorBlack;
  static std::map<uint32_t, RGB> theme;
};

#endif
