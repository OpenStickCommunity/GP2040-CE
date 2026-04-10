#ifndef _BURST_COLOR_H_
#define _BURST_COLOR_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animation.h"
#include "../animationstation.h"

struct FBurstData
{
    int XPos;
    int YPos;
    RGB StartColor;
    float RunningTime = -1.0f;
};

struct FGridEntry
{
    RGB Color;
    float Strength = 0.0f;

    FGridEntry()
    {
        Color = RGB(0,0,0);
    };
};

#define MAX_BURSTS 32
#define DEFAULT_BURST_TAIL_PROP 0.5f
#define MIN_TIME_BETWEEN_BURSTS_ON_BUTTON_IN_MS 50
#define BURST_DISTANCE_PER_SEC 10.0f

class BurstColor : public Animation {

public:
    BurstColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    BurstColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins, EButtonCaseEffectType InButtonCaseEffectType, bool IsSmall = false);
    ~BurstColor() { };

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;
 
  virtual void CycleParameterChange() override;

protected:

    virtual int32_t GetFadeTime() { return MIN_TIME_BETWEEN_BURSTS_ON_BUTTON_IN_MS; }

    virtual void NewPressForPin(int lightIndex) override;

    FBurstData RunningBursts[MAX_BURSTS];

    bool bRandomColor;

    int BurstLength;

    int BurstTailLength;

    int MinXCoord = 0;
    int MinYCoord = 0;
    int MaxXCoord = 0;
    int MaxYCoord = 0;
};

#endif
