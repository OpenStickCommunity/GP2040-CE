#include "rainbow.h"
#include "storagemanager.h"

#define RAINBOW_COLORWHEEL_FRAME_MAX 255

// clamp rainbowCycleTime to [1 ... INT16_MAX]
#define RAINBOW_CYCLE_INCREMENT   5
#define RAINBOW_CYCLE_MAX         60
#define RAINBOW_CYCLE_MIN         2

//grid distance to frame offset value
#define RAINBOW_GRID_OFFSET_ADJUST  30

RainbowSynced::RainbowSynced(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime == 0)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = ((RAINBOW_CYCLE_MAX - RAINBOW_CYCLE_MIN) / 2) + RAINBOW_CYCLE_MIN;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > RAINBOW_CYCLE_MAX)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MAX;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < RAINBOW_CYCLE_MIN )
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MIN;
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

    this->nextRunTime = make_timeout_time_ms(RAINBOW_CYCLE_MAX - AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime);
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

void RainbowSynced::ParameterUp() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  cycleTime = cycleTime + RAINBOW_CYCLE_INCREMENT;
  
  if (cycleTime > RAINBOW_CYCLE_MAX) 
  {
    cycleTime = RAINBOW_CYCLE_MAX;
  } 

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void RainbowSynced::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  cycleTime = cycleTime - RAINBOW_CYCLE_INCREMENT;

  if (cycleTime < RAINBOW_CYCLE_MIN) 
  {
    cycleTime = RAINBOW_CYCLE_MIN;
  } 

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

////////////////////////////////////////////////////////////////////////////////////////////
RainbowRotate::RainbowRotate(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime == 0)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = ((RAINBOW_CYCLE_MAX - RAINBOW_CYCLE_MIN) / 2) + RAINBOW_CYCLE_MIN;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > RAINBOW_CYCLE_MAX)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MAX;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < RAINBOW_CYCLE_MIN)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MIN;}

void RainbowRotate::Animate(RGB (&frame)[FRAME_MAX]) 
{
  if (time_reached(this->nextRunTime)) 
  {
    currentFrame++;

    if (currentFrame >= 255) 
    {
      currentFrame = 0;
    }

    this->nextRunTime = make_timeout_time_ms(RAINBOW_CYCLE_MAX - AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime);
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

void RainbowRotate::ParameterUp() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  cycleTime = cycleTime + RAINBOW_CYCLE_INCREMENT;
  
  if (cycleTime > RAINBOW_CYCLE_MAX) 
  {
    cycleTime = RAINBOW_CYCLE_MAX;
  } 

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void RainbowRotate::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  cycleTime = cycleTime - RAINBOW_CYCLE_INCREMENT;

  if (cycleTime < RAINBOW_CYCLE_MIN) 
  {
    cycleTime = RAINBOW_CYCLE_MIN;
  } 

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}