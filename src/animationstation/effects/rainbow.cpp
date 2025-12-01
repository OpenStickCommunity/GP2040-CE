#include "rainbow.h"
#include "storagemanager.h"

#define RAINBOW_COLORWHEEL_FRAME_MAX 255

#define RAINBOW_CYCLE_MAX         60
#define RAINBOW_CYCLE_MIN         2

//grid distance to frame offset value
#define RAINBOW_GRID_OFFSET_ADJUST  30

RainbowSynced::RainbowSynced(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  CycleParameterChange();
}

void RainbowSynced::Animate(RGB (&frame)[FRAME_MAX]) 
{
  if (time_reached(this->nextRunTime)) 
  {
    if (reverse)
    {
      currentFrame--;

      if (currentFrame < 0) 
      {
        currentFrame = 1;
        reverse = false;
      }
    } 
    else 
    {
      currentFrame++;

      if (currentFrame > RAINBOW_COLORWHEEL_FRAME_MAX) 
      {
        currentFrame = RAINBOW_COLORWHEEL_FRAME_MAX - 1;
        reverse = true;
      }
    }

    this->nextRunTime = make_timeout_time_ms(RAINBOW_CYCLE_MAX - cycleTime);
  }

  RGB color = RGB::wheel(this->currentFrame);
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type))
    {
      uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        //Non pressed simply sets the RGB color
        frame[ledIndex] = color;
      }
    }
  }
}

void RainbowSynced::CycleParameterChange() 
{
    int16_t cycleStep = 2;
    if(ButtonCaseEffectType == EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY)
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCaseCycleTime;
    else
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

    cycleTime = RAINBOW_CYCLE_MIN + (((RAINBOW_CYCLE_MAX - RAINBOW_CYCLE_MIN) / CYCLE_STEPS) * cycleStep);
}
////////////////////////////////////////////////////////////////////////////////////////////
RainbowRotate::RainbowRotate(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  CycleParameterChange();
}

void RainbowRotate::Animate(RGB (&frame)[FRAME_MAX]) 
{
  if (time_reached(this->nextRunTime)) 
  {
    currentFrame++;

    if (currentFrame >= 255) 
    {
      currentFrame = 0;
    }

    this->nextRunTime = make_timeout_time_ms(RAINBOW_CYCLE_MAX - cycleTime);
  }

  //the way this works is we offset the current frame by the distance from the top left of the grid
  int thisFrame = this->currentFrame;
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type))
    {
      int gridOffset = RGBLights->AllLights[lightIndex].Position.XPosition + RGBLights->AllLights[lightIndex].Position.YPosition;
      int thisLightFrame = (thisFrame + (gridOffset * RAINBOW_GRID_OFFSET_ADJUST)) % RAINBOW_COLORWHEEL_FRAME_MAX;
      RGB color = RGB::wheel(thisLightFrame);

      uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        //Non pressed simply sets the RGB color
        frame[ledIndex] = color;
      }
    }
  }
}

void RainbowRotate::CycleParameterChange() 
{
    int16_t cycleStep = 2;
    if(ButtonCaseEffectType == EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY)
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCaseCycleTime;
    else
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

    cycleTime = RAINBOW_CYCLE_MIN + (((RAINBOW_CYCLE_MAX - RAINBOW_CYCLE_MIN) / CYCLE_STEPS) * cycleStep);
}