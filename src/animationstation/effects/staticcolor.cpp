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

<<<<<<< HEAD:src/animationstation/effects/staticcolor.cpp
bool StaticColor::Animate(RGB (&frame)[100]) {
=======
void StaticColor::Animate(RGB (&frame)[100]) 
{
>>>>>>> LED_ConvertToGIPOWithNewAnimations:lib/AnimationStation/src/Effects/StaticColor.cpp
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
        frame[ledIndex] = AdjustColor(GetNonPressedColorForLight(lightIndex));
      }
      else if (isButtonAnimation && LightTypeIsForPressedAnimation(RGBLights->AllLights[lightIndex].Type))
      {
        frame[ledIndex] = FadeColor(AdjustColor(GetPressedColorForLight(lightIndex)),
                                        frame[ledIndex],
                                        fadeTimes[ledIndex]);    
      }
    }
  }
<<<<<<< HEAD:src/animationstation/effects/staticcolor.cpp
  return true;
}

uint8_t StaticColor::GetColor() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    return animationOptions.buttonColorIndex;
  } else {
    return animationOptions.staticColorIndex;
  }
}

void StaticColor::ParameterUp() {
  uint8_t colorIndex;
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    colorIndex = animationOptions.buttonColorIndex;
  } else {
    colorIndex = animationOptions.staticColorIndex;
  }

  if (colorIndex < colors.size() - 1) {
    colorIndex++;
  } else {
    colorIndex = 0;
  }

  this->SaveIndexOptions(colorIndex);
}

void StaticColor::SaveIndexOptions(uint8_t colorIndex) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    animationOptions.buttonColorIndex = colorIndex;
  } else {
    animationOptions.staticColorIndex = colorIndex;
  }
}

void StaticColor::ParameterDown() {
  uint8_t colorIndex;
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (this->filtered) {
    colorIndex = animationOptions.buttonColorIndex;
  } else {
    colorIndex = animationOptions.staticColorIndex;
  }

  if (colorIndex > 0) {
    colorIndex--;
  } else {
    colorIndex = colors.size() - 1;
  }
  this->SaveIndexOptions(colorIndex);
}
=======

  // Count down the timer
  DecrementFadeCounters();
}
>>>>>>> LED_ConvertToGIPOWithNewAnimations:lib/AnimationStation/src/Effects/StaticColor.cpp
