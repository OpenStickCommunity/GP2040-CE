#ifndef CUSTOM_THEME_H_
#define CUSTOM_THEME_H_

#include <map>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class CustomTheme : public Animation {
public:
  CustomTheme(PixelMatrix &matrix);
  ~CustomTheme() {  };

  static bool HasTheme();
  static void SetCustomTheme(std::map<uint32_t, RGB> customTheme);
  virtual void UpdatePixels(std::vector<Pixel> pixels);
  void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();
protected:
  RGB defaultColor = ColorBlack;
  static std::map<uint32_t, RGB> theme;
  
  // timers per button
  static std::map<uint32_t, int32_t> times;
  static std::map<uint32_t, RGB> hitColor;    

  RGB BlendColor(RGB start, RGB end, uint32_t frame);

  absolute_time_t lastUpdateTime = nil_time;

  uint32_t coolDownTimeInMs = 1000;

};

#endif
