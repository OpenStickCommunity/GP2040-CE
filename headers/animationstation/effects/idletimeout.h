#ifndef _IDLE_TIMEOUT_H_
#define _IDLE_TIMEOUT_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "animation.h"
#include "animationstation.h"
#include "staticcolor.h"

class IdleTimeout : public StaticColor {
public:
  IdleTimeout(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  IdleTimeout(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~IdleTimeout() { };


protected:

  virtual RGB GetNonPressedColorForLight(uint32_t LightIndex) override;
};

#endif
