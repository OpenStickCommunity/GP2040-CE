#include "rainbow.h"
#include "storagemanager.h"

<<<<<<< HEAD:src/animationstation/effects/rainbow.cpp
Rainbow::Rainbow(PixelMatrix &matrix) : Animation(matrix) {
}

bool Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return false;
  }

  UpdateTime();
  UpdatePresses(frame);

  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      // Count down the timer
      DecrementFadeCounter(pixel.index);

      RGB color = RGB::wheel(this->currentFrame);
      for (auto &pos : pixel.positions)
        frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
    }
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 1;
      reverse = false;
    }
  } else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  this->nextRunTime = make_timeout_time_ms(animationOptions.rainbowCycleTime);

  return true;
}
=======
#define RAINBOW_COLORWHEEL_FRAME_MAX 255
>>>>>>> LED_ConvertToGIPOWithNewAnimations:lib/AnimationStation/src/Effects/Rainbow.cpp

// clamp rainbowCycleTime to [1 ... INT16_MAX]
#define RAINBOW_CYCLE_INCREMENT   10
#define RAINBOW_CYCLE_MAX         INT16_MAX - RAINBOW_CYCLE_INCREMENT
#define RAINBOW_CYCLE_MIN         1         + RAINBOW_CYCLE_INCREMENT

<<<<<<< HEAD:src/animationstation/effects/rainbow.cpp
void Rainbow::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime < RAINBOW_CYCLE_MAX) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime + RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = INT16_MAX;
  }
}

void Rainbow::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime > RAINBOW_CYCLE_MIN) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime - RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = 1;
=======
//grid distance to frame offset value
#define RAINBOW_GRID_OFFSET_ADJUST  30

RainbowSynced::RainbowSynced(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime == 0)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = ((RAINBOW_CYCLE_MAX - RAINBOW_CYCLE_MIN) / 2) + RAINBOW_CYCLE_MIN;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > RAINBOW_CYCLE_MAX + RAINBOW_CYCLE_INCREMENT)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MAX + RAINBOW_CYCLE_INCREMENT;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < RAINBOW_CYCLE_MIN - RAINBOW_CYCLE_INCREMENT)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MIN - RAINBOW_CYCLE_INCREMENT;
}

void RainbowSynced::Animate(RGB (&frame)[100]) 
{
  if (!time_reached(this->nextRunTime)) 
  {
    return;
>>>>>>> LED_ConvertToGIPOWithNewAnimations:lib/AnimationStation/src/Effects/Rainbow.cpp
  }

  RGB color = RGB::wheel(this->currentFrame);
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
    {
      uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        //Non pressed simply sets the RGB colour
        frame[ledIndex] = color;
      }
    }
  }

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

  this->nextRunTime = make_timeout_time_ms(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime);
}

void RainbowSynced::ParameterUp() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  if (cycleTime < RAINBOW_CYCLE_MAX) 
  {
    cycleTime = cycleTime + RAINBOW_CYCLE_INCREMENT;
  } 
  else 
  {
    cycleTime = INT16_MAX;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void RainbowSynced::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  if (cycleTime > RAINBOW_CYCLE_MIN) 
  {
    cycleTime = cycleTime - RAINBOW_CYCLE_INCREMENT;
  } 
  else
  {
    cycleTime = 1;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

////////////////////////////////////////////////////////////////////////////////////////////
RainbowRotate::RainbowRotate(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > RAINBOW_CYCLE_MAX)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MAX;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < RAINBOW_CYCLE_MIN)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAINBOW_CYCLE_MIN;}

void RainbowRotate::Animate(RGB (&frame)[100]) 
{
  if (!time_reached(this->nextRunTime)) 
  {
    return;
  }

  //the way this works is we offset the current frame by the distance from the top left of the grid
  int thisFrame = this->currentFrame;
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
    {
      int gridOffset = RGBLights->AllLights[lightIndex].Position.XPosition + RGBLights->AllLights[lightIndex].Position.YPosition;
      int thisLightFrame = (thisFrame + (gridOffset * RAINBOW_GRID_OFFSET_ADJUST)) % RAINBOW_COLORWHEEL_FRAME_MAX;
      RGB color = RGB::wheel(thisLightFrame);

      uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        //Non pressed simply sets the RGB colour
        frame[ledIndex] = color;
      }
    }
  }

  currentFrame++;

  if (currentFrame >= 255) 
  {
    currentFrame = 0;
  }

  this->nextRunTime = make_timeout_time_ms(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime);
}

void RainbowRotate::ParameterUp() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  if (cycleTime < RAINBOW_CYCLE_MAX) 
  {
    cycleTime = cycleTime + RAINBOW_CYCLE_INCREMENT;
  } 
  else 
  {
    cycleTime = INT16_MAX;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void RainbowRotate::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

  if (cycleTime > RAINBOW_CYCLE_MIN) 
  {
    cycleTime = cycleTime - RAINBOW_CYCLE_INCREMENT;
  } 
  else 
  {
    cycleTime = 1;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}