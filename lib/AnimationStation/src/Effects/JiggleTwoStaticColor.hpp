#ifndef _JIGGLE_TWO_STATIC_COLOR_H_
#define _JIGGLE_TWO_STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"
#include "StaticColor.hpp"

class JiggleTwoStaticColor : public StaticColor {
public:
  JiggleTwoStaticColor(Lights& InRGBLights);
  JiggleTwoStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~JiggleTwoStaticColor() { };

  virtual RGB AdjustColor(RGB InColor) override;
 
protected:
};

#endif
