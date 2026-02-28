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

  virtual RGB AdjustColor(int ledIndex, RGB InColor) override;
 
protected:

    int RainbowWheelFrame[MAX_JITTER_VALUES];
    bool RainbowWheelReversed[MAX_JITTER_VALUES];
};

#endif
