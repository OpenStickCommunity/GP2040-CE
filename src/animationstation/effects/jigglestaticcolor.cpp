#include "jigglestaticcolor.h"
#include "staticcolor.h"

JiggleStaticColor::JiggleStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : StaticColor(InRGBLights, InButtonCaseEffectType) 
{
}

JiggleStaticColor::JiggleStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : StaticColor(InRGBLights, InPressedPins) 
{
}

RGB JiggleStaticColor::AdjustColor(RGB InColor)
{
  RGB outColor;
  float newR = (((float)InColor.r) * 0.85f) + (38 * (((float)(rand() % 100) / 100.0f)));
  if(newR > 255.0f)
    newR = 255;
  outColor.r = (int)newR;

float newG = (((float)InColor.g) * 0.85f) + (38 * (((float)(rand() % 100) / 100.0f)));
  if(newG > 255.0f)
    newG = 255;
  outColor.g = (int)newG;

 float newB = (((float)InColor.b) * 0.85f) + (38 * (((float)(rand() % 100) / 100.0f)));
  if(newB > 255.0f)
    newB = 255;
  outColor.b = (int)newB;

  return outColor;
}
 