#ifndef _JIGGLE_STATIC_COLOR_H_
#define _JIGGLE_STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animation.h"
#include "../animationstation.h"
#include "staticcolor.h"

#define MAX_JITTER_VALUES 100

class JiggleStaticColor : public StaticColor {
public:
  JiggleStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  JiggleStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~JiggleStaticColor() { };

  virtual RGB AdjustColor(int ledIndex, RGB InColor) override;

  virtual void CycleParameterChange() override;

protected:

  RGB AdvanceColor(int ledIndex, RGB InColor, RGB DestColor);

  float JitterVal[MAX_JITTER_VALUES];
  bool JitterReverse[MAX_JITTER_VALUES];
};

#endif
