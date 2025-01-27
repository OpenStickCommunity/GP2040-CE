#ifndef _RAINBOW_H_
#define _RAINBOW_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../AnimationStation.hpp"

class RainbowSynced : public Animation 
{
public:
  RainbowSynced(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  ~RainbowSynced() {};

  virtual void Animate(RGB (&frame)[100]) override;

  //These change the speed of the rainbow changing color
  void ParameterUp();
  void ParameterDown();

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

  void Animate(RGB (&frame)[100]);

  //These change the speed of the rainbow changing color
  void ParameterUp();
  void ParameterDown();

protected:
  int currentFrame = 0;
  absolute_time_t nextRunTime = nil_time;
};

#endif
