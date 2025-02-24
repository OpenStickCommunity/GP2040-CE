#ifndef _RANDOM_COLOR_H_
#define _RANDOM_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../Animation.hpp"
#include "../AnimationStation.hpp"

class RandomColor : public Animation {
public:
  RandomColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  RandomColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins);
  ~RandomColor() { };

  virtual void Animate(RGB (&frame)[100]) override;
 
protected:

  virtual void NewPressForPin(int lightIndex) override;

  std::vector<RGB> savedPressedColour;
};

#endif
