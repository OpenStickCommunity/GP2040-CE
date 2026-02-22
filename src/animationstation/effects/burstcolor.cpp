#include "burstcolor.h"

#define BURST_CYCLE_INCREMENT   10
#define BURST_CYCLE_MAX         100
#define BURST_CYCLE_MIN         10

BurstColor::BurstColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
}

BurstColor::BurstColor(Lights& InRGBLights, bool bInRandomColor, bool bInSmallBurst, std::vector<int32_t> &InPressedPins, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
    isButtonAnimation = true;
    pressedPins = InPressedPins;

    bRandomColor = bInRandomColor;

    bSmallBurst = bInSmallBurst;

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(RGBLights->AllLights[lightIndex].Type != LightType::LightType_ActionButton && RGBLights->AllLights[lightIndex].Type != LightType::LightType_Case)
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

    CycleParameterChange();
}

void BurstColor::NewPressForPin(int lightIndex)
{
    //find empty burst data and create new burst
    for(int index = 0; index < MAX_BURSTS; ++index)
    {
        if(RunningBursts[index].RunningTime < 0)
        {
            RunningBursts[index].RunningTime = 0.0f;
            int pickedCol = (rand() % (colors.size()-1)) + 1;
            RunningBursts[index].StartColor = bRandomColor ? colors[pickedCol] : GetPressedColorForLight(lightIndex);
            RunningBursts[index].XPos = RGBLights->AllLights[lightIndex].Position.XPosition;
            RunningBursts[index].YPos = RGBLights->AllLights[lightIndex].Position.YPosition;
            return;
        }
    }
}

void BurstColor::Animate(RGB (&frame)[FRAME_MAX]) 
{
    UpdateTime();
    UpdatePresses();

    std::vector<FGridEntry> OneLineGrid;
    OneLineGrid.assign(MaxYCoord+1, FGridEntry());
    std::vector<std::vector<FGridEntry>> FullGrid;
    FullGrid.assign(MaxXCoord+1, OneLineGrid);

    //get each grid positions color and strength
    for(int burstIndex = 0; burstIndex < MAX_BURSTS; ++burstIndex)
    {
        if(RunningBursts[burstIndex].RunningTime < 0.0f)
            continue;
    
        RunningBursts[burstIndex].RunningTime += (((float)cycleTime) / 1000.0f);
        float travelledDist = RunningBursts[burstIndex].RunningTime * BURST_DISTANCE_PER_SEC;

        //is this the last frame?
        float largestCoord = MAX(1 + (MaxXCoord - MinXCoord), 1 + (MaxYCoord - MinYCoord));
        float distanceToTravel = bSmallBurst ? (largestCoord / 13.0f) * (float)BURST_DISTANCE : largestCoord; //Burst_distance was designed with my setup for my T16 in mind which was 0-12 xcoord
        if(travelledDist > (float)distanceToTravel + 4.0f)
            RunningBursts[burstIndex].RunningTime = -1.0f;

        int xStart = RunningBursts[burstIndex].XPos - distanceToTravel;
        if(xStart < MinXCoord)
            xStart = MinXCoord;
        int yStart = RunningBursts[burstIndex].YPos - distanceToTravel;
        if(yStart < MinYCoord)
            yStart = MinYCoord;
        int xEnd = RunningBursts[burstIndex].XPos + distanceToTravel;
        if(xEnd > MaxXCoord)
            xEnd = MaxXCoord;
        int yEnd = RunningBursts[burstIndex].YPos + distanceToTravel;
        if(yEnd > MaxYCoord)
            yEnd = MaxYCoord;

        for(int xCoord = xStart; xCoord <= xEnd; ++xCoord)
        {
            for(int yCoord = yStart; yCoord <= yEnd; ++yCoord)
            {
                //This is a square expansion. Furthest coord defines the distance
                int distanceFromCenter = abs(yCoord - RunningBursts[burstIndex].YPos);
                if(abs(xCoord - RunningBursts[burstIndex].XPos) > distanceFromCenter)
                    distanceFromCenter = abs(xCoord - RunningBursts[burstIndex].XPos);

                float Strength = 0.0f;
                //2 wide here, up then down
                if((int)travelledDist < distanceFromCenter || (int)travelledDist > distanceFromCenter + 3)
                    continue;

                if((int)travelledDist < distanceFromCenter+1)
                    Strength = travelledDist - (float)((int)travelledDist);
                else if((int)travelledDist < distanceFromCenter+3)
                    Strength = 1.0f;
                else
                    Strength = 1.0f - (travelledDist - (float)((int)travelledDist));

                //update grid pos
                //strength is highest applied to this point
                if(Strength > FullGrid[xCoord][yCoord].Strength)
                    FullGrid[xCoord][yCoord].Strength = Strength;
                int redToApply = (float)(RunningBursts[burstIndex].StartColor.r) * Strength;
                if((int)FullGrid[xCoord][yCoord].Color.r + redToApply > 0xFF)
                    FullGrid[xCoord][yCoord].Color.r = 0xFF;
                else
                    FullGrid[xCoord][yCoord].Color.r += redToApply;

                int greenToApply = (float)(RunningBursts[burstIndex].StartColor.g) * Strength;
                if((int)FullGrid[xCoord][yCoord].Color.g + greenToApply > 0xFF)
                    FullGrid[xCoord][yCoord].Color.g = 0xFF;
                else
                    FullGrid[xCoord][yCoord].Color.g += greenToApply;

                int blueToApply = (float)(RunningBursts[burstIndex].StartColor.b) * Strength;
                if((int)FullGrid[xCoord][yCoord].Color.b + blueToApply > 0xFF)
                    FullGrid[xCoord][yCoord].Color.b = 0xFF;
                else
                    FullGrid[xCoord][yCoord].Color.b += blueToApply;
            }
        }
    }

    //now apply those values to lights
    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
            continue;

        uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
        uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

        for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
        {
            frame[ledIndex] = BlendColor(FullGrid[RGBLights->AllLights[lightIndex].Position.XPosition][RGBLights->AllLights[lightIndex].Position.YPosition].Color,
                        frame[ledIndex],
                        (1.0f - FullGrid[RGBLights->AllLights[lightIndex].Position.XPosition][RGBLights->AllLights[lightIndex].Position.YPosition].Strength));    
        }
    }

    // Count down the timer
    DecrementFadeCounters();
}

void BurstColor::CycleParameterChange() 
{
    int16_t cycleStep = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].basePressedCycleTime;
    cycleTime = BURST_CYCLE_MIN + (((BURST_CYCLE_MAX - BURST_CYCLE_MIN) / CYCLE_STEPS) * cycleStep);
}
