#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class StaticColor : public Animation {
public:
  StaticColor(Lights& InRGBLights);
  StaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~StaticColor() { };

  virtual void Animate(RGB (&frame)[100]) override;
protected:

  virtual RGB AdjustColor(RGB InColor) { return InColor; }
};

#endif
