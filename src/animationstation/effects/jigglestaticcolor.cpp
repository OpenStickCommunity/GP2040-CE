#include "jigglestaticcolor.h"
#include "staticcolor.h"

#define JIGGLE_CYCLE_MAX         300
#define JIGGLE_CYCLE_MIN         50

JiggleStaticColor::JiggleStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : StaticColor(InRGBLights, InButtonCaseEffectType) 
{
  for(int index = 0; index < MAX_JITTER_VALUES; ++index)
  {
    JitterVal[index] = 0;
    JitterReverse[index] = false;
  }

  CycleParameterChange();
}

JiggleStaticColor::JiggleStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : StaticColor(InRGBLights, InPressedPins) 
{
  for(int index = 0; index < MAX_JITTER_VALUES; ++index)
  {
    JitterVal[index] = 0;
    JitterReverse[index] = false;
  }

  CycleParameterChange();
}

RGB JiggleStaticColor::AdjustColor(int ledIndex, RGB InColor)
{
  //Generate a colour thats it 50% of the in color
  RGB outColor;
  outColor.r = (int)(((float)InColor.r) * 0.50f);
  outColor.g = (int)(((float)InColor.g) * 0.50f);
  outColor.b = (int)(((float)InColor.b) * 0.50f);

  return AdvanceColor(ledIndex, outColor, InColor);
}
 
RGB JiggleStaticColor::AdvanceColor(int ledIndex, RGB InColor, RGB DestColor)
{
  int SafeLightIndex = ledIndex % MAX_JITTER_VALUES;

  //Advance Jitter values
  if(!JitterReverse[SafeLightIndex])
  {
    JitterVal[SafeLightIndex] += ((((float)(rand() % 100)) * cycleTime) / 100.0f);
    if(JitterVal[SafeLightIndex] > 100)
    {
      JitterVal[SafeLightIndex] = 100;
      JitterReverse[SafeLightIndex] = true;
    }
  }
  else
  {
    JitterVal[SafeLightIndex] -= ((((float)(rand() % 100)) * cycleTime) / 100.0f);
    if(JitterVal[SafeLightIndex] < 0)
    {
      JitterVal[SafeLightIndex] = 0;
      JitterReverse[SafeLightIndex] = false;
    }
  }

  RGB outColor;
  float colDiff = (int)DestColor.r - (int)InColor.r;
  int newR = InColor.r + (colDiff * (JitterVal[SafeLightIndex] / 100.0f));
  if(newR > 255)
    newR = 255;
  outColor.r = newR;

  colDiff = (int)DestColor.g - (int)InColor.g;
  int newG = InColor.g + (colDiff * (JitterVal[SafeLightIndex] / 100.0f));
  if(newG > 255)
    newG = 255;
  outColor.g = newG;

  colDiff = (int)DestColor.b - (int)InColor.b;
  int newB = InColor.b + (colDiff * (JitterVal[SafeLightIndex] / 100.0f));
  if(newB > 255)
    newB = 255;
  outColor.b = newB;

  return outColor;
}

void JiggleStaticColor::CycleParameterChange() 
{
    int16_t cycleStep;
    if(ButtonCaseEffectType == EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY)
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCaseCycleTime;
    else
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

    cycleTime = JIGGLE_CYCLE_MIN + (((JIGGLE_CYCLE_MAX - JIGGLE_CYCLE_MIN) / CYCLE_STEPS) * cycleStep);
}