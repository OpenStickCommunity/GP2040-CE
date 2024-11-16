#include "StaticColor.hpp"

StaticColor::StaticColor(Lights& InRGBLights) : Animation(InRGBLights) 
{
}

StaticColor::StaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : Animation(InRGBLights) 
{
  isButtonAnimation = true;
  pressedPins = InPressedPins;
}

void StaticColor::Animate(RGB (&frame)[100]) 
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
      if(!isButtonAnimation && LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].notPressedStaticColors[RGBLights->AllLights[lightIndex].GIPOPin];
      }
      else if (isButtonAnimation && LightTypeIsForPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = FadeColor(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].pressedStaticColors[RGBLights->AllLights[lightIndex].GIPOPin],
                                        frame[ledIndex],
                                        fadeTimes[ledIndex]);    
      }
    }
  }

  // Count down the timer
  DecrementFadeCounters();
}