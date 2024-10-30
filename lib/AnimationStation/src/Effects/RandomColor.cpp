#include "RandomColor.hpp"

RandomColor::RandomColor(Lights& InRGBLights) : Animation(InRGBLights) 
{
}

RandomColor::RandomColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : Animation(InRGBLights) 
{
  isButtonAnimation = true;
  pressedPins = InPressedPins;

  savedPressedColour.clear();
  for(int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    savedPressedColour.push_back(RGB(0));
  }
}

void RandomColor::NewPressForPin(int lightIndex)
{
    savedPressedColour[lightIndex] = colors[rand() % colors.size()];
}

void RandomColor::Animate(RGB (&frame)[100]) 
{
  UpdateTime();
  UpdatePresses();

  for(int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
    uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

    for(int ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
    {
      //Non pressed simply sets the RGB colour
      if(!isButtonAnimation && LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].notPressedStaticColors[RGBLights->AllLights[lightIndex].GIPOPin];
      }
      else if (isButtonAnimation && LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = BlendColor(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].notPressedStaticColors[RGBLights->AllLights[lightIndex].GIPOPin],
                                        frame[ledIndex],
                                        fadeTimes[ledIndex]);    
      }
    }
  }

  // Count down the timer
  DecrementFadeCounters();
}