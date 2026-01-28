#include "staticcolor.h"
#include "storagemanager.h"

StaticColor::StaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
}

StaticColor::StaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : Animation(InRGBLights, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY) 
{
  isButtonAnimation = true;
  pressedPins = InPressedPins;
}

void StaticColor::Animate(RGB (&frame)[FRAME_MAX]) 
{
  UpdateTime();
  UpdatePresses();

  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
    uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

    for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
    {
      //Non pressed simply sets the RGB color
      if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        if(!isButtonAnimation)
        {
          frame[ledIndex] = AdjustColor(GetNonPressedColorForLight(lightIndex));
        }
        else if (isButtonAnimation)
        {
          frame[ledIndex] = FadeColor(AdjustColor(GetPressedColorForLight(lightIndex)),
                                          frame[ledIndex],
                                          fadeTimes[ledIndex]);    
        }
      }
    }
  }

  // Count down the timer
  DecrementFadeCounters();
}
