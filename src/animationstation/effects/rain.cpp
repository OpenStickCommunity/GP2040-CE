#include "rain.h"
#include <algorithm>

#define RAIN_CYCLE_INCREMENT   50
#define RAIN_CYCLE_MAX         500
#define RAIN_CYCLE_MIN         10

Rain::Rain(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType, ERainFrequency InRainFrequency) : Animation(InRGBLights, InButtonCaseEffectType) 
{
    RainFrequency = InRainFrequency;

    if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime == 0)
        AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = ((RAIN_CYCLE_MAX - RAIN_CYCLE_MIN) / 2) + RAIN_CYCLE_MIN;
    if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime > RAIN_CYCLE_MAX)
        AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAIN_CYCLE_MAX;
    if(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime < RAIN_CYCLE_MIN)
        AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = RAIN_CYCLE_MIN;

    for(int index = 0; index < MAX_RAIN_DROPS; ++index)
    {
        RainYCoords[index] = -1.0f;
        RainXCoords[index] = -1;
    }

    for(int index = 0; index < RAIN_DROP_NO_REPEAT_X_NUM; ++index)
    {
        previousRainDropXCoords[index] = -1;
    }

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
}

float Rain::GetTimeTillNextDrop()
{
    switch(RainFrequency)
    {
        case ERainFrequency::RAIN_LOW:
            return 3.5f;
            break;

        case ERainFrequency::RAIN_MEDIUM:
            return 2.5f;
            break;

        case ERainFrequency::RAIN_HIGH:
            return 1.0f;
            break;

        default:
            return 3.0f;
            break;
    }
}

bool Rain::RainHistoryContains(int newXCoord)
{
    for(int index = 0; index < RAIN_DROP_NO_REPEAT_X_NUM; ++index)
    {
        if(previousRainDropXCoords[index] == newXCoord)
            return true;
        if(newXCoord > 0 && (previousRainDropXCoords[index] == newXCoord-1))
            return true;
        if(newXCoord > 1 && (previousRainDropXCoords[index] == newXCoord-2))
            return true;
        if(newXCoord < MaxXCoord && (previousRainDropXCoords[index] == newXCoord+1))
            return true;
         if(newXCoord < MaxXCoord-1 && (previousRainDropXCoords[index] == newXCoord+2))
            return true;
   }

    return false;
}

int Rain::FindLightForCoord(int xCoord, int yCoord)
{
    //if we can find an exact match we return that. Otherwise we look for one to the left and then right of where we are. Max 1 spot away
    int backupLeftIndex = -1;
    int backupRightIndex = -1;

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
            continue;

        if(RGBLights->AllLights[lightIndex].Position.YPosition == yCoord)
        {
            int xPos = RGBLights->AllLights[lightIndex].Position.XPosition;
            if(xPos == xCoord)
                return lightIndex;
            else if (xPos+1 == xCoord)
                backupLeftIndex = lightIndex;
            else if (xPos-1 == xCoord)
                backupRightIndex = lightIndex;
        }       
    }    

    if(backupLeftIndex != -1)
        return backupLeftIndex;

    return backupRightIndex;
}

void Rain::Animate(RGB (&frame)[FRAME_MAX]) 
{
    UpdateTime();

    //reset all lights first to ensure that if pressed lights are unpressed they can blend back to correct color
    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
            continue;

        RGB lightCol = GetNonPressedColorForLight(lightIndex);
        uint8_t firstLedIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
        uint8_t lastLedIndex = firstLedIndex + RGBLights->AllLights[lightIndex].LedsPerLight;
        for(uint8_t ledIndex = firstLedIndex; ledIndex < lastLedIndex; ++ledIndex)
        {
            frame[ledIndex] = lightCol;
        }
    }
    
    //if there isnt enough lights for this effect then do nothing
    if((MaxXCoord - MinXCoord) < (RAIN_DROP_NO_REPEAT_X_NUM + 2))
        return;

    //update times and spawn next drop if required
    TimeTillNextRain -= (((float)AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime) / 1000.0f);
    if(TimeTillNextRain < 0.0f)
    {
        //reduce history by 1
        for(int index = 0; index < (RAIN_DROP_NO_REPEAT_X_NUM - 1); ++index)
        {
            previousRainDropXCoords[index] = previousRainDropXCoords[index + 1];
        }
        previousRainDropXCoords[RAIN_DROP_NO_REPEAT_X_NUM - 1] = -1;

        //generate new X coord that isnt in the history
        int newRainXCoord = -1;
        int infGuard = 10;
        while (infGuard > 0 && (newRainXCoord == -1 || RainHistoryContains(newRainXCoord)))
        {
            newRainXCoord = rand() % ((MaxXCoord - MinXCoord) + 1);
            newRainXCoord += MinXCoord;
            infGuard--;
        }

        if(infGuard != 0)
        {
            //spawn new drop
            for(int rainIndex = 0; rainIndex < MAX_RAIN_DROPS; ++rainIndex)
            {        
                if(RainXCoords[rainIndex] < 0)
                {
                    RainXCoords[rainIndex] = newRainXCoord;
                    previousRainDropXCoords[RAIN_DROP_NO_REPEAT_X_NUM - 1] = newRainXCoord;
                    RainYCoords[rainIndex] = ((float)MinYCoord) - 1.0f;
                    break;
                }
            }
        }

        TimeTillNextRain += GetTimeTillNextDrop();
    }

    //update rain drops
    for(int rainIndex = 0; rainIndex < MAX_RAIN_DROPS; ++rainIndex)
    {
        if(RainXCoords[rainIndex] < 0)
            continue;

        //advance rain drop vertical
        RainYCoords[rainIndex] += (DefaultRainSpeed * (((float)AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime) / 1000.0f));

        //is it finished (off the bottom)
        if(RainYCoords[rainIndex] > (float)(MaxYCoord + 1))
        {
            RainXCoords[rainIndex] = -1;
            continue;
        }

        //Find closest light to this and the next y coord
        int firstYIndex = (int)RainYCoords[rainIndex];
        int firstLightIndex = FindLightForCoord(RainXCoords[rainIndex], firstYIndex);
        int secondLightIndex = FindLightForCoord(RainXCoords[rainIndex], firstYIndex + 1);

        float firstLightAlpha = 1.0f - (RainYCoords[rainIndex] - (float)firstYIndex);
        float secondLightAlpha = 1.0f - firstLightAlpha;

        RGB specialLightCol = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor;
        
        if(firstLightIndex >= 0)
        {
            RGB firstLightCol = GetNonPressedColorForLight(firstLightIndex);
            uint8_t firstLedIndex = RGBLights->AllLights[firstLightIndex].FirstLedIndex;
            uint8_t lastLedIndex = firstLedIndex + RGBLights->AllLights[firstLightIndex].LedsPerLight;
            for(uint8_t ledIndex = firstLedIndex; ledIndex < lastLedIndex; ++ledIndex)
            {
                frame[ledIndex] = BlendColor(firstLightCol,
                                            specialLightCol, 
                                            firstLightAlpha);
            }
        }

        if(secondLightIndex >= 0)
        {
            RGB secondLightCol = GetNonPressedColorForLight(secondLightIndex);
            uint8_t firstLedIndex = RGBLights->AllLights[secondLightIndex].FirstLedIndex;
            uint8_t lastLedIndex = firstLedIndex + RGBLights->AllLights[secondLightIndex].LedsPerLight;
            for(uint8_t ledIndex = firstLedIndex; ledIndex < lastLedIndex; ++ledIndex)
            {
                frame[ledIndex] = BlendColor(secondLightCol,
                                            specialLightCol, 
                                            secondLightAlpha);
            }
        }
    }
}

void Rain::ParameterUp() 
{
  int32_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;
  cycleTime = cycleTime + RAIN_CYCLE_INCREMENT;

  if (cycleTime > RAIN_CYCLE_MAX) 
  {
    cycleTime = RAIN_CYCLE_MAX;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}

void Rain::ParameterDown() 
{
  int16_t cycleTime = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime;
  cycleTime = cycleTime - RAIN_CYCLE_INCREMENT;

  if (cycleTime < RAIN_CYCLE_MIN) 
  {
    cycleTime = RAIN_CYCLE_MIN;
  }

  AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].baseCycleTime = cycleTime;
}
