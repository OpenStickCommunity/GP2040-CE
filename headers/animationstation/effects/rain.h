#ifndef _RAIN_H_
#define _RAIN_H_

#include "../animation.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../animationstation.h"

//drips light down the grid. Matrix/rain

typedef enum
{
  RAIN_LOW,
  RAIN_MEDIUM,
  RAIN_HIGH,
} ERainFrequency;

#define MAX_RAIN_DROPS 20
#define RAIN_DROP_NO_REPEAT_X_NUM 3

class Rain : public Animation {
public:
    Rain(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType, ERainFrequency InRainFrequency = ERainFrequency::RAIN_MEDIUM);
    ~Rain() {};

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;

    virtual void ParameterUp() override;
    virtual void ParameterDown() override;

protected:
    float GetTimeTillNextDrop();
    bool RainHistoryContains(int newXCoord);
    int FindLightForCoord(int xCoord, int yCoord);

    float RainYCoords[MAX_RAIN_DROPS];
    int RainXCoords[MAX_RAIN_DROPS];

    int previousRainDropXCoords[RAIN_DROP_NO_REPEAT_X_NUM];

    float TimeTillNextRain = 0.0f;
    float DefaultRainSpeed = 1.5f;

    ERainFrequency RainFrequency = ERainFrequency::RAIN_MEDIUM;

    std::vector<uint32_t> OrderedLights;

    int MinXCoord = 0;
    int MinYCoord = 0;
    int MaxXCoord = 0;
    int MaxYCoord = 0;

    absolute_time_t NextRunTime = nil_time;
};

#endif
