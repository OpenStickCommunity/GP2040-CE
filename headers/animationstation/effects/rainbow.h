#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "animation.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "animationstation.h"

class RainbowSynced : public Animation 
{
public:
  RainbowSynced(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  ~RainbowSynced() {};

  virtual void Animate(RGB (&frame)[FRAME_MAX]) override;

  //These change the speed of the rainbow changing color
  virtual void ParameterUp() override;
  virtual void ParameterDown() override;

protected:
  int currentFrame = 0;
  bool reverse = false;
  absolute_time_t nextRunTime = nil_time;
};

class RainbowRotate : public Animation 
{
public:
  RainbowRotate(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  ~RainbowRotate() {};

  virtual void Animate(RGB (&frame)[FRAME_MAX]) override;

  //These change the speed of the rainbow changing color
  virtual void ParameterUp() override;
  virtual void ParameterDown() override;

protected:
  int currentFrame = 0;
  absolute_time_t nextRunTime = nil_time;
};

#endif
