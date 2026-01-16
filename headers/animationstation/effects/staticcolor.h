#ifndef _STATIC_COLOR_H_
#define _STATIC_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "animation.h"
#include "animationstation.h"

class StaticColor : public Animation {
public:
  StaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  StaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~StaticColor() { };

  virtual void Animate(RGB (&frame)[FRAME_MAX]) override;
protected:

  virtual RGB AdjustColor(RGB InColor) { return InColor; }
};

#endif
