#ifndef _JIGGLE_STATIC_COLOR_H_
#define _JIGGLE_STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animation.h"
#include "../animationstation.h"
#include "staticcolor.h"

class JiggleStaticColor : public StaticColor {
public:
  JiggleStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  JiggleStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~JiggleStaticColor() { };

  virtual RGB AdjustColor(RGB InColor) override;
 
protected:
};

#endif
