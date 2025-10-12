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

#define MAX_BURSTS 16
#define BURST_DISTANCE 6
#define MIN_TIME_BETWEEN_BURSTS_ON_BUTTON_IN_MS 100
#define BURST_DISTANCE_PER_SEC 10.0f

class BurstColor : public Animation {

public:
    BurstColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    BurstColor(Lights& InRGBLights, bool bInRandomColor, bool bInSmallBurst, std::vector<int32_t> &InPressedPins, EButtonCaseEffectType InButtonCaseEffectType);
    ~BurstColor() { };

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;
 
  virtual void PressParameterUp() override;
  virtual void PressParameterDown() override;

protected:

    virtual int32_t GetFadeTime() { return MIN_TIME_BETWEEN_BURSTS_ON_BUTTON_IN_MS; }

    virtual void NewPressForPin(int lightIndex) override;

    FBurstData RunningBursts[MAX_BURSTS];

    bool bRandomColor;

    bool bSmallBurst;

    int MinXCoord = 0;
    int MinYCoord = 0;
    int MaxXCoord = 0;
    int MaxYCoord = 0;
};

#endif
