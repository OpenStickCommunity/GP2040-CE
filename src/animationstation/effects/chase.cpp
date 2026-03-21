#include "chase.h"
#include <algorithm>
#include <math.h>

#define CHASE_CYCLE_MAX         1000
#define CHASE_CYCLE_MIN         50
#define CHASE_LIGHT_DELAY       1.0f

#define CHASE_RAINBOW_FRAME_CHANGE_PER_TICK 10

Chase::Chase(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType, ChaseTypes InChaseType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
  ChaseTypeInUse = InChaseType;

  CycleParameterChange();

  NextLightTimer = CHASE_LIGHT_DELAY;

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

  //store off all lights in required order used on this animation type (only for some animation types)
  OrderLights();

  //pick starting type if using one of the multi types
  switch(ChaseTypeInUse)
  {
    case ChaseTypes::CHASETYPES_RANDOM:
    {
      RandomChaseType = (SingleChaseTypes)(rand() % (int)SINGLECHASETYPES_MAX_RANDOM_TYPES);
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
    
    case ChaseTypes::CHASETYPES_CIRCLE_CLOCKWISE:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_CIRCLE_CLOCKWISE;
    } break;
    
    case ChaseTypes::CHASETYPES_CIRCLE_ANTICLOCKWISE:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE;
    } break;
    
    case ChaseTypes::CHASETYPES_CIRCLE_PINGPONG:
    {
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE;
    } break;
    
    default:
      RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL;
      break;
  }

  SetStartLight();
}

void Chase::OrderLights()
{
  bool bIsCircleBased = ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_CLOCKWISE || ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_ANTICLOCKWISE || ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_PINGPONG;

  if(!bIsCircleBased)
  {
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
  }
  else if(ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_CLOCKWISE || ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_ANTICLOCKWISE || ChaseTypeInUse == ChaseTypes::CHASETYPES_CIRCLE_PINGPONG)
  {
    float midX = MinXCoord + (MaxXCoord - MinXCoord) / 2;
    float midY = MinYCoord + (MaxYCoord - MinYCoord) / 2;
    float angleResult[FRAME_MAX];
    float resultIndex[FRAME_MAX];
    unsigned int lightsFound = 0;
    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
      if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
        continue;

      //calculate rotational angle
      float xOffset = RGBLights->AllLights[lightIndex].Position.XPosition - midX;
      float yOffset = (RGBLights->AllLights[lightIndex].Position.YPosition - midY) * -1; //flip this so that the start point is the top of the controller
      float thisDot = (xOffset * 0) + (yOffset * -1);
      float thisDet = (xOffset * -1) - (yOffset * 0);
      float angle = atan2(thisDet, thisDot);

      //Slot it into array
      for(unsigned int sortIndex = 0; sortIndex <= lightsFound; ++sortIndex)
      {
        if(sortIndex == lightsFound)
        {
          angleResult[sortIndex] = angle;
          resultIndex[sortIndex] = lightIndex;
          break;
        }
        else if(angle < angleResult[sortIndex])
        {
          for(unsigned int offsetIndex = lightsFound; offsetIndex > sortIndex; --offsetIndex)
          {
            angleResult[offsetIndex] = angleResult[offsetIndex - 1];
            resultIndex[offsetIndex] = resultIndex[offsetIndex - 1];
          }
          angleResult[sortIndex] = angle;
          resultIndex[sortIndex] = lightIndex;
          break;
        }
      }

      lightsFound++;
    }

    //Now build the ordered list
    for(unsigned int sortedIndex = 0; sortedIndex < lightsFound; ++sortedIndex)
    {
      OrderedLights.push_back(resultIndex[sortedIndex]);
    }
  }
}

void Chase::GetSpecialColors(RGB& chaseCol, RGB& caseChaseCol)
{
  chaseCol = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor;
  caseChaseCol = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].caseSpecialColor;
  bool buttonIsRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bNonPressedSpecialColorIsRainbow;
  bool caseIsRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bCaseSpecialColorIsRainbow;
  if(buttonIsRainbow || caseIsRainbow)
  {
    if(!RainbowWheelReversed)
    {
      RainbowWheelFrame += CHASE_RAINBOW_FRAME_CHANGE_PER_TICK;
      if(RainbowWheelFrame >= 255)
      {
        RainbowWheelReversed = true;
        RainbowWheelFrame = 255;
      }
    }
    else
    {
      RainbowWheelFrame -= CHASE_RAINBOW_FRAME_CHANGE_PER_TICK;
      if(RainbowWheelFrame <= 0)
      {
        RainbowWheelReversed = false;
        RainbowWheelFrame = 0;
      }
    }

    if(buttonIsRainbow)
      chaseCol = RGB::wheel(RainbowWheelFrame);
    if (caseIsRainbow)
      caseChaseCol = RGB::wheel(RainbowWheelFrame);
  }
}

void Chase::Animate(RGB (&frame)[FRAME_MAX]) 
{
  UpdateTime();

  //dont do anything if there aren't enough lights
  if(OrderedLights.size() < 2)
    return;

  //update times and move to the next light(s) if required
  NextLightTimer -= (((float)cycleTime) / 1000.0f);

  //Get Special light colors
  RGB chaseCol, caseChaseCol;
  GetSpecialColors(chaseCol, caseChaseCol);

  //Blend the lights
  for(unsigned int lightIndex = 0; lightIndex < OrderedLights.size(); ++lightIndex)
  {
      uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[lightIndex]].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[lightIndex]].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        frame[ledIndex] = BlendColor(GetNonPressedColorForLight(OrderedLights[lightIndex]),
                                      RGBLights->AllLights[OrderedLights[lightIndex]].Type == LightType::LightType_ActionButton ? chaseCol : caseChaseCol, 
                                      ChaseBlendValues[ledIndex]);
      }
  }

  CheckToAdvanceLight();
}

void Chase::AssignThisFrameValues()
{
  int currentLightReverseAdjusted = CurrentLight;

   //now light the correct lights
  switch(RandomChaseType)
  {
    case SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL:
    {
      //if we're on the way back then invert the index
      if(Reversed)
      {
        currentLightReverseAdjusted = (OrderedLights.size()-1) - CurrentLight;
      } 

      uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        ChaseBlendValues[ledIndex] = 1.0f;
      }      
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT:
      //reverse the order and fall through
      currentLightReverseAdjusted = (MaxXCoord) - (CurrentLight - MinXCoord);
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
            ChaseBlendValues[ledIndex] = 1.0f;
          }      
        }
      }
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP:
      //reverse the order and fall through
      currentLightReverseAdjusted = (MaxYCoord) - (CurrentLight - MinYCoord);
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
            ChaseBlendValues[ledIndex] = 1.0f;
          }      
        }
      }
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE:
      //reverse the order and fall through
      currentLightReverseAdjusted = (OrderedLights.size()-1) - CurrentLight;
      //Fall through (no break)
    case SingleChaseTypes::SINGLECHASETYPES_CIRCLE_CLOCKWISE:
    {
      uint8_t firstLightIndex = RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].FirstLedIndex;
      uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[OrderedLights[currentLightReverseAdjusted]].LedsPerLight;
      for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
      {
        ChaseBlendValues[ledIndex] = 1.0f;
      }      
    } break;

    default:
      break;
  } 
}

void Chase::CheckToAdvanceLight()
{
  if(NextLightTimer <= 0.0f)
  {
    //Apply fade for all lights if we're moving to the next light
    for(unsigned int chaseBlendIndex = 0; chaseBlendIndex < FRAME_MAX; ++chaseBlendIndex)
    {
      float tailLength = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedEffectContextParam;
      if(tailLength == 0)
        tailLength = CHASE_DEFAULT_TAIL_LENGTH;
      float fadePerLight = 1.0f / tailLength;
      ChaseBlendValues[chaseBlendIndex] -= fadePerLight;
      if(ChaseBlendValues[chaseBlendIndex] < 0)
        ChaseBlendValues[chaseBlendIndex] = 0;
    }

    CurrentLight++;
    CheckForEndOfSequence();
    NextLightTimer += CHASE_LIGHT_DELAY;
    AssignThisFrameValues();
  }
}

void Chase::CheckForEndOfSequence()
{
  bool hasEnded = false;

  switch(RandomChaseType)
  {
    case SingleChaseTypes::SINGLECHASETYPES_SEQUENTIAL:
    case SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE:
    case SingleChaseTypes::SINGLECHASETYPES_CIRCLE_CLOCKWISE:
    {
      if((unsigned int)CurrentLight >= OrderedLights.size())
      {
        hasEnded = true;
      }
    } break;
 
    case SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT:
    case SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT:
    {
      if(CurrentLight > MaxXCoord)
      {
        hasEnded = true;
      }
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM:
    case SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP:
    {
      if(CurrentLight > MaxYCoord)
      {
        hasEnded = true;
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

      case ChaseTypes::CHASETYPES_CIRCLE_PINGPONG:
      {
        if(RandomChaseType == SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE)
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_CIRCLE_CLOCKWISE;
        else
          RandomChaseType = SingleChaseTypes::SINGLECHASETYPES_CIRCLE_ANTICLOCKWISE;
      } break;
      
      case ChaseTypes::CHASETYPES_RANDOM:
      {
        RandomChaseType = (SingleChaseTypes)(rand() % (int)SINGLECHASETYPES_MAX_RANDOM_TYPES);
      } break;
      
      case ChaseTypes::CHASETYPES_TESTLAYOUT:
      {
        TestLayoutFlipFlop = !TestLayoutFlipFlop;
        RandomChaseType = TestLayoutFlipFlop ? SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM : SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT;
      } break;
      
      default:
        break;
    }

    SetStartLight();
  }
}

void Chase::SetStartLight()
{
  CurrentLight = 0;
  switch(RandomChaseType)
  {
    case SingleChaseTypes::SINGLECHASETYPES_LEFT_TO_RIGHT:
    case SingleChaseTypes::SINGLECHASETYPES_RIGHT_TO_LEFT:
    {
      CurrentLight = MinXCoord;
    } break;

    case SingleChaseTypes::SINGLECHASETYPES_TOP_TO_BOTTOM:
    case SingleChaseTypes::SINGLECHASETYPES_BOTTOM_TO_TOP:
    {
      CurrentLight = MinYCoord;
    } break;

    default:
      break;
  }

  AssignThisFrameValues();
}

void Chase::CycleParameterChange() 
{
    int16_t cycleStep;
    if(ButtonCaseEffectType == EButtonCaseEffectType::BUTTONCASELIGHTTYPE_CASE_ONLY)
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCaseCycleTime;
    else
      cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;

    cycleTime = CHASE_CYCLE_MIN + (((CHASE_CYCLE_MAX - CHASE_CYCLE_MIN) / CYCLE_STEPS) * cycleStep);
}
