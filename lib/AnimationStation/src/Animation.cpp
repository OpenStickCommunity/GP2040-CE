#include "Animation.hpp"

#include "AnimationStation.hpp"

#define PRESS_COOLDOWN_INCREMENT 500
#define PRESS_COOLDOWN_MAX 5000
#define PRESS_COOLDOWN_MIN 0

LEDFormat Animation::format;

Animation::Animation(Lights& InRGBLights) : RGBLights(&InRGBLights) 
{
  fadeTimes.clear();
  for(int ledIndex = 0; ledIndex < RGBLights->GetLedCount(); ++ledIndex)
  {
    fadeTimes.push_back(0); 
  }

  holdTimeInMs = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].buttonPressHoldTimeInMs;
  fadeoutTimeInMs = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].buttonPressFadeOutTimeInMs;
  
  //Since we use the fadeTimes array to know if a light is held. make sure that we hold for at least 1 ms.
  if(holdTimeInMs <= 0)
    holdTimeInMs = 1;
}

void Animation::UpdatePressed(std::vector<int32_t> InPressedPins)
{
  this->pressedPins = InPressedPins;
}

void Animation::ClearPressed()
{
  this->pressedPins.clear();
}

void Animation::UpdateTime() 
{
  absolute_time_t currentTime = get_absolute_time();
  updateTimeInMs = absolute_time_diff_us(lastUpdateTime, currentTime) / 1000;
  lastUpdateTime = currentTime;
}

void Animation::UpdatePresses() 
{
  if(!isButtonAnimation)
    return;

  //Set hold/fade time for all pressed buttons
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(RGBLights->AllLights[lightIndex].Type != LightType_ActionButton)
      continue;

    for(unsigned int pressedPinIndex = 0; pressedPinIndex < pressedPins.size(); ++pressedPinIndex)
    {
      if(pressedPins[pressedPinIndex] == RGBLights->AllLights[lightIndex].GIPOPin)
      {
        uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
        uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
        bool wasPressedOrFading = false;
        for(int ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
        {
          if(fadeTimes[ledIndex] > 0)
            wasPressedOrFading = true;
          fadeTimes[ledIndex] = holdTimeInMs + fadeoutTimeInMs; 
        }

        //if this is a new press, let effects know
        if(!wasPressedOrFading)
        {
          NewPressForPin(lightIndex);
        }
      }
    }
  }
}

void Animation::DecrementFadeCounters() 
{
  if(!isButtonAnimation)
    return;

  //Set hold/fade time for all pressed buttons
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    //early out, save frames!
    if(RGBLights->AllLights[lightIndex].Type != LightType_ActionButton || fadeTimes[RGBLights->AllLights[lightIndex].FirstLedIndex] <= 0)
      continue;

    //is this button still pressed ? 
    bool wasPressed = false;
    for(unsigned int pressedPinIndex = 0; pressedPinIndex < pressedPins.size(); ++pressedPinIndex)
    {
      if(pressedPins[pressedPinIndex] == RGBLights->AllLights[lightIndex].GIPOPin)
      {
        wasPressed = true;
      }
    }

    //if not then we can safely decrement the timer
    if(!wasPressed)
    {
      uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
      for(int ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        fadeTimes[ledIndex] -= updateTimeInMs;
        if (fadeTimes[ledIndex] < 0)
          fadeTimes[ledIndex] = 0;        
      }
    }
  }
}

RGB Animation::BlendColor(RGB start, RGB end, float alpha) 
{
  RGB result = ColorBlack;

  if (alpha < 0.0f) alpha = 0.0f;
  if (alpha > 1.0f) alpha = 1.0f;

  result.r = static_cast<uint32_t>(static_cast<float>(start.r) + ((static_cast<float>(end.r) - static_cast<float>(start.r)) * alpha));
  result.g = static_cast<uint32_t>(static_cast<float>(start.g) + ((static_cast<float>(end.g) - static_cast<float>(start.g)) * alpha));
  result.b = static_cast<uint32_t>(static_cast<float>(start.b) + ((static_cast<float>(end.b) - static_cast<float>(start.b)) * alpha));

  return result;
}

RGB Animation::FadeColor(RGB start, RGB end, uint32_t timeRemainingInMs) 
{
  RGB result = ColorBlack;

  //fade over?
  if (timeRemainingInMs <= 0) 
    return end;

  //still in hold time?
  if(timeRemainingInMs > fadeoutTimeInMs)
    return start;

  float progress = 1.0f - (static_cast<float>(timeRemainingInMs) / static_cast<float>(fadeoutTimeInMs));
  if (progress < 0.0f) progress = 0.0f;
  if (progress > 1.0f) progress = 1.0f;

  result.r = static_cast<uint32_t>(static_cast<float>(start.r) + ((static_cast<float>(end.r) - static_cast<float>(start.r)) * progress));
  result.g = static_cast<uint32_t>(static_cast<float>(start.g) + ((static_cast<float>(end.g) - static_cast<float>(start.g)) * progress));
  result.b = static_cast<uint32_t>(static_cast<float>(start.b) + ((static_cast<float>(end.b) - static_cast<float>(start.b)) * progress));

  return result;
}

//Type helpers
bool Animation::LightTypeIsForNonPressedAnimation(LightType Type)
{
  if(Type == LightType::LightType_ActionButton || Type == LightType::LightType_Case)
    return true;

  return false;
}

bool Animation::LightTypeIsForPressedAnimation(LightType Type)
{
  if(Type == LightType::LightType_ActionButton)
    return true;

  return false;
}

bool Animation::LightTypeIsForSpecialMoveAnimation(LightType Type)
{
  if(Type == LightType::LightType_ActionButton || Type == LightType::LightType_Case)
    return true;

  return false;
}