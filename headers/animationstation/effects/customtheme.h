#ifndef CUSTOM_THEME_H_
#define CUSTOM_THEME_H_

#include <map>
#include "animation.h"
#include "animationstation.h"

class CustomTheme : public Animation {
public:
  CustomTheme(PixelMatrix &matrix);
  ~CustomTheme() {  };

  bool HasTheme();
  bool Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  std::map<uint32_t, RGB> theme;
};

#endif
