#ifndef _CHASE_H_
#define _CHASE_H_

#include "animation.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "animationstation.h"

//Chase lights the lights up in a specific order. 2 lights can be on at once as they overlaps on off times. There are multiple directions that can be used.

//List of chase types
typedef enum
{
  CHASETYPES_SEQUENTIAL,
  CHASETYPES_SEQUENTIAL_PINGPONG,
  CHASETYPES_LEFT_TO_RIGHT,
  CHASETYPES_RIGHT_TO_LEFT,
  CHASETYPES_TOP_TO_BOTTOM,
  CHASETYPES_BOTTOM_TO_TOP,
  CHASETYPES_HORIZONTAL_PINGPONG,
  CHASETYPES_VERTICAL_PINGPONG,
  CHASETYPES_RANDOM,
  CHASETYPES_MAX,
  CHASETYPES_TESTLAYOUT,
} ChaseTypes;

typedef enum
{
  SINGLECHASETYPES_SEQUENTIAL,
  SINGLECHASETYPES_LEFT_TO_RIGHT,
  SINGLECHASETYPES_RIGHT_TO_LEFT,
  SINGLECHASETYPES_TOP_TO_BOTTOM,
  SINGLECHASETYPES_BOTTOM_TO_TOP,
  SINGLECHASETYPES_MAX,
} SingleChaseTypes;

class Chase : public Animation {
public:
  Chase(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType, ChaseTypes InChaseType);
  ~Chase() {};

  virtual void Animate(RGB (&frame)[FRAME_MAX]) override;
  
  virtual void ParameterUp() override;
  virtual void ParameterDown() override;

  ChaseTypes ChaseTypeInUse = CHASETYPES_SEQUENTIAL;

protected:
 
  void CheckForEndOfSequence();

  float ChaseTimes[2];
  std::vector<uint32_t> OrderedLights;

  int MinXCoord = 0;
  int MinYCoord = 0;
  int MaxXCoord = 0;
  int MaxYCoord = 0;

  int CurrentLight = 0;

  bool Reversed = false;

  SingleChaseTypes RandomChaseType;
  
  absolute_time_t NextRunTime = nil_time;

  bool TestLayoutFlipFlop = false;
};

#endif
