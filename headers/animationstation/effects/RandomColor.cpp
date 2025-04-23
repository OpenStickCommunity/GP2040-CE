#include "randomcolor.h"

RandomColor::RandomColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
}

RandomColor::RandomColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : Animation(InRGBLights, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY) 
{
  isButtonAnimation = true;
  pressedPins = InPressedPins;

  savedPressedColour.clear();
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    savedPressedColour.push_back(RGB(0));
  }
}

void RandomColor::NewPressForPin(int lightIndex)
{
  //dont pick 0 as thats black
  savedPressedColour[lightIndex] = colors[(rand() % (colors.size()-1)) + 1];
}

void RandomColor::Animate(RGB (&frame)[100]) 
{
  UpdateTime();
  UpdatePresses();

  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
    uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

    for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
    {
      //Non pressed simply sets the RGB colour
      if (LightTypeIsForPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = FadeColor(savedPressedColour[lightIndex],
                                        frame[ledIndex],
                                        fadeTimes[ledIndex]);    
      }
    }
  }

  // Count down the timer
  DecrementFadeCounters();
}