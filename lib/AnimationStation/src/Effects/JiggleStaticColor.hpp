#ifndef _JIGGLE_STATIC_COLOR_H_
#define _JIGGLE_STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"
#include "StaticColor.hpp"

class JiggleStaticColor : public StaticColor {
public:
  JiggleStaticColor(Lights& InRGBLights);
  JiggleStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~JiggleStaticColor() { };

  virtual RGB AdjustColor(RGB InColor) override;
 
protected:
};

#endif
