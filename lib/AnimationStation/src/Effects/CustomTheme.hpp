#ifndef CUSTOM_THEME_H_
#define CUSTOM_THEME_H_

#include <map>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class CustomTheme : public Animation {
public:
  CustomTheme(PixelMatrix &matrix);
  ~CustomTheme() {};

  static bool HasTheme();
  static void SetCustomTheme(std::map<uint32_t, RGB> customTheme);
  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  RGB defaultColor = ColorBlack;
  static std::map<uint32_t, RGB> theme;
};

#endif
