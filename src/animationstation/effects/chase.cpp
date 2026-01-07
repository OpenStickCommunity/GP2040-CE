#include "chase.h"
#include <algorithm>

#define CHASE_CYCLE_INCREMENT   50
#define CHASE_CYCLE_MAX         500
#define CHASE_CYCLE_MIN         10
#define CHASE_SECOND_LIGHT_OFFSET 0.5f

Chase::Chase(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType, ChaseTypes InChaseType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  ChaseTypeInUse = InChaseType;

  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime == 0)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = ((CHASE_CYCLE_MAX - CHASE_CYCLE_MIN) / 2) + CHASE_CYCLE_MIN;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > CHASE_CYCLE_MAX)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = CHASE_CYCLE_MAX;
  if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < CHASE_CYCLE_MIN)
    AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = CHASE_CYCLE_MIN;

  ChaseTimes[0] = 1.0f;
  ChaseTimes[1] = 1.0f + CHASE_SECOND_LIGHT_OFFSET;

  OrderedLights.clear();

  //Get max x and y coords
  for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
  {
    if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
      continue;

    if(RGBLights->AllLights[lightIndex].Position.XPosition > MaxXCoord)
      MaxXCoord = RGBLights->AllLights[lightIndex].Position.XPosition;
    if(RGBLights->AllLights[lightIndex].Position.YPosition > MaxYCoord)
      MaxYCoord = RGBLights->AllLights[lightIndex].Position.YPosition;

    if(lightIndex == 0 || RGBLights->AllLights[lightIndex].Position.XPosition < MinXCoord)
      MinXCoord = RGBLights->AllLights[lightIndex].Position.XPosition;
    if(lightIndex == 0 || RGBLights->AllLights[lightIndex].Position.YPosition < MinYCoord)
      MinYCoord = RGBLights->AllLights[lightIndex].Position.YPosition;   
  }

  //store off all lights in sequential order used on this animation
  for(int yCoord = 0; yCoord <= MaxYCoord; ++yCoord)
  {
    for(int xCoord = 0; xCoord <= MaxXCoord; ++xCoord)
    {
      for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
      {
        if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) && 
            RGBLights->AllLights[lightIndex].Position.XPosition == xCoord && 
            RGBLights->AllLights[lightIndex].Position.YPosition == yCoord)
          OrderedLights.push_back(lightIndex);
      }    
    }
  }

  //pick starting type if using one of the multi types
  switch(ChaseTypeInUse)
  {
    case ChaseTypes::CHASETYPES_RANDOM:
    {
      RandomChaseType = (SingleChaseTypes)(rand() % (int)SINGLECHASETYPES_MAX);
    } break;

    case ChaseTypes::CHASETYPES_TESTLAYOUT:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
    } break;
    
    case ChaseTypes::CHASETYPES_SEQUENTIAL_PINGPONG:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL;
    } break;

    case ChaseTypes::CHASETYPES_HORIZONTAL_PINGPONG:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
    } break;
  
    case ChaseTypes::CHASETYPES_VERTICAL_PINGPONG:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM;
    } break;

    case ChaseTypes::CHASETYPES_TOP_TO_BOTTOM:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM;
    } break;    
    
    case ChaseTypes::CHASETYPES_BOTTOM_TO_TOP:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP;
    } break;    
    
    case ChaseTypes::CHASETYPES_LEFT_TO_RIGHT:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
    } break;
    
    case ChaseTypes::CHASETYPES_RIGHT_TO_LEFT:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT;
    } break;
    
    default:
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL;
      break;
  }
}

void Chase::Animate(RGB (&frame)[FRAME_MAX]) 
{
  UpdateTime();

  //dont do anything if there aren't enough lights
  if(OrderedLights.size() < 2)
    return;

  //update times and move to the next light(s) if required
  ChaseTimes[0] -= (((float)AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime) / 1000.0f);
  ChaseTimes[1] -= (((float)AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime) / 1000.0f);
  if(ChaseTimes[0] < 0.0f)
  {
    ChaseTimes[0] = 0.0f;
  }

  //reset all lights first to ensure that if pressed lights are unpressed they can blend back to correct color
  for(unsigned int lightIndex = 0; lightIndex < OrderedLights.size(); ++lightIndex)
  {
      uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        frame[ledIndex] = GetNonPressedColorForLight(OrderedLights[lightIndex]);
      }
  }

  //caclulate fade times here as these are used for all cases below
  float fadeTimeOne = 0.0f;
  if(ChaseTimes[0] < 1.0f && ChaseTimes[0] >= 0.8f)
    fadeTimeOne = 1.0f - ((ChaseTimes[0] - 0.8f) / 0.2f);
  else if(ChaseTimes[0] >= 0.2f && ChaseTimes[0] < 0.8f)
    fadeTimeOne = 1.0f;
  else if(ChaseTimes[0] < 0.2f && ChaseTimes[0] >= 0.0f)
    fadeTimeOne = (ChaseTimes[0] / 0.2f);
  float fadeTimeTwo = 0.0f;
  if(ChaseTimes[1] < 1.0f && ChaseTimes[1] >= 0.8f)
    fadeTimeTwo = 1.0f - ((ChaseTimes[1] - 0.8f) / 0.2f);
  else if(ChaseTimes[1] >= 0.2f && ChaseTimes[1] < 0.8f)
    fadeTimeTwo = 1.0f;
  else if(ChaseTimes[1] < 0.2f && ChaseTimes[1] >= 0.0f)
    fadeTimeTwo = (ChaseTimes[1] / 0.2f);

  //get this and next light
  int currentLightReverseAdjusted = CurrentLight;
  int nextLightReverseAdjusted = currentLightReverseAdjusted + 1;
 
  //now light the correct lights
  switch(RandomChaseType)
  {
    case SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL:
    {
      //if we're on the way back then invert the index
      if(Reversed)
      {
        currentLightReverseAdjusted = (OrderedLights.size()-1) - CurrentLight;
        nextLightReverseAdjusted = currentLightReverseAdjusted - 1;
      } 

      uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[currentLightReverseAdjusted]),
                                      AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                      fadeTimeOne);
      }      

      if((unsigned int)(nextLightReverseAdjusted) < OrderedLights.size())
      {         
        uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[nextLightReverseAdjusted]].FirstLedIndex;
        uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[nextLightReverseAdjusted]].LedsPerLight;
        for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
        {
          frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[nextLightReverseAdjusted]),
                                        AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                        fadeTimeTwo);
        }          
      } 
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT:
      //reverse the order and fall through
      currentLightReverseAdjusted = (MaxXCoord) - (CurrentLight - MinXCoord);
      nextLightReverseAdjusted = currentLightReverseAdjusted - 1;
      //Fall through (no break)

    case SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT:
    { 
      for(unsigned int lightIndex = 0; lightIndex < OrderedLights.size(); ++lightIndex)
      {
        if(RGBLights->AllLights[OrderedLights[lightIndex]].Position.XPosition == currentLightReverseAdjusted)
        {
          uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
          uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
          for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
          {
            frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[lightIndex]),
                                          AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                          fadeTimeOne);
          }      
        }

        if((CurrentLight+1) <= MaxXCoord)
        {      
          if(RGBLights->AllLights[OrderedLights[lightIndex]].Position.XPosition == nextLightReverseAdjusted)
          {   
            uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
              frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[lightIndex]),
                                            AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                            fadeTimeTwo);
            }          
          }
        } 
      }
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP:
      //reverse the order and fall through
      currentLightReverseAdjusted = (MaxYCoord) - (CurrentLight - MinYCoord);
      nextLightReverseAdjusted = currentLightReverseAdjusted - 1;
      //Fall through (no break)

    case SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM:
    { 
      for(unsigned int lightIndex = 0; lightIndex < OrderedLights.size(); ++lightIndex)
      {
        if(RGBLights->AllLights[OrderedLights[lightIndex]].Position.YPosition == currentLightReverseAdjusted)
        {
          uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
          uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
          for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
          {
            frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[lightIndex]),
                                          AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                          fadeTimeOne);
          }      
        }

        if((CurrentLight+1) <= MaxYCoord)
        {      
          if(RGBLights->AllLights[OrderedLights[lightIndex]].Position.YPosition == nextLightReverseAdjusted)
          {   
            uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
              frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[lightIndex]),
                                            AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor, 
                                            fadeTimeTwo);
            }          
          }
        } 
      }
    } break;

    default:
      break;
  }

  if(ChaseTimes[0] <= 0.0f)
  {
    CurrentLight++;
    ChaseTimes[0] = ChaseTimes[1];
    ChaseTimes[1] += CHASE_SECOND_LIGHT_OFFSET;
    CheckForEndOfSequence();
  }
}

void Chase::CheckForEndOfSequence()
{
  bool hasEnded = false;

  switch(RandomChaseType)
  {
    case SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL:
    {
      if((unsigned int)CurrentLight >= OrderedLights.size())
      {
        hasEnded = true;
        CurrentLight = 0;
        ChaseTimes[0] = 1.0f;
        ChaseTimes[1] = 1.0f + CHASE_SECOND_LIGHT_OFFSET;
      }
    } break;
 
    case SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT:
    case SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT:
    {
      if(CurrentLight > MaxXCoord)
      {
        hasEnded = true;
        CurrentLight = MinXCoord;
        ChaseTimes[0] = 1.0f;
        ChaseTimes[1] = 1.0f + CHASE_SECOND_LIGHT_OFFSET;
      }
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM:
    case SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP:
    {
      if(CurrentLight > MaxYCoord)
      {
        hasEnded = true;
        CurrentLight = MinYCoord;
        ChaseTimes[0] = 1.0f;
        ChaseTimes[1] = 1.0f + CHASE_SECOND_LIGHT_OFFSET;
      }
    } break;

    default:
      break;
  }

  if(hasEnded)
  {
    //if a multi type then decide next type
    switch(ChaseTypeInUse)
    {
      case ChaseTypes::CHASETYPES_SEQUENTIAL_PINGPONG:
      {
        Reversed = !Reversed;
      } break;
  
      case ChaseTypes::CHASETYPES_HORIZONTAL_PINGPONG:
      {
        if(RandomChaseType == SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT)
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT;
        else
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
      } break;

      case ChaseTypes::CHASETYPES_VERTICAL_PINGPONG:
      {
        if(RandomChaseType == SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM)
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP;
        else
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM;
      } break;

      case ChaseTypes::CHASETYPES_RANDOM:
      {
        RandomChaseType = (SingleChaseTypes)(rand() % (int)SINGLECHASETYPES_MAX);
      } break;
      
      case ChaseTypes::CHASETYPES_TESTLAYOUT:
      {
        TestLayoutFlipFlop = !TestLayoutFlipFlop;
        RandomChaseType = TestLayoutFlipFlop ? SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM : SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
      } break;
      
      default:
        break;
    }
  }
}

void Chase::ParameterUp() 
{
  int32_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;
  cycleTime = cycleTime + CHASE_CYCLE_INCREMENT;

  if (cycleTime > CHASE_CYCLE_MAX) 
  {
    cycleTime = CHASE_CYCLE_MAX;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void Chase::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;
  cycleTime = cycleTime - CHASE_CYCLE_INCREMENT;

  if (cycleTime < CHASE_CYCLE_MIN) 
  {
    cycleTime = CHASE_CYCLE_MIN;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}
