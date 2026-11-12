#ifndef _JIGGLE_TWO_STATIC_COLOR_H_
#define _JIGGLE_TWO_STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animation.h"
#include "../animationstation.h"
#include "jigglestaticcolor.h"

class JiggleTwoStaticColor : public JiggleStaticColor {
public:
  JiggleTwoStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  JiggleTwoStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~JiggleTwoStaticColor() { };

  virtual RGB AdjustColor(int ledIndex, RGB InColor, LightType lightType) override;
 
protected:

    int RainbowWheelFrame[FRAME_MAX];
    bool RainbowWheelReversed[FRAME_MAX];
};

#endif
