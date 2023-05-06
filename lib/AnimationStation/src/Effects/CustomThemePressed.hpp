#ifndef _CUSTOM_THEME_PRESSED_H_
#define _CUSTOM_THEME_PRESSED_H_

#include <map>
#include <vector>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class CustomThemePressed : public Animation {
public:
  CustomThemePressed(PixelMatrix &matrix);
  CustomThemePressed(PixelMatrix &matrix, std::vector<Pixel> &pixels);
  ~CustomThemePressed() { pixels = nullptr; };

  static bool HasTheme();
  static void SetCustomTheme(std::map<uint32_t, RGB> customTheme);
  void UpdatePixels(std::vector<Pixel> pixels);
  void Animate(RGB (&frame)[100]);
  void ParameterUp() { }
  void ParameterDown() { }
protected:
  std::vector<Pixel> *pixels;
  RGB defaultColor = ColorBlack;
  static std::map<uint32_t, RGB> theme;
};

#endif
