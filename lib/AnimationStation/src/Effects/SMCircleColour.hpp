#ifndef _SM_CIRCLECOLOUR_H_
#define _SM_CIRCLECOLOUR_H_

#include "../Animation.hpp"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../AnimationStation.hpp"
#include "../SpecialMoveSystem.hpp"

typedef enum
{
  SM_CIRCLE_STATE_NOTSTARTED,
  SM_CIRCLE_STATE_PREFRAMES,
  SM_CIRCLE_STATE_RUNNING,
  SM_CIRCLE_STATE_POSTFRAMES,
  SM_CIRCLE_STATE_FINISHED,
} SMCircleColourState;

class SMCircleColour : public Animation {
public:
    SMCircleColour(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    ~SMCircleColour() { };

    virtual void Animate(RGB (&frame)[100]) override;

    virtual void SetOptionalParams(uint32_t OptionalParams) override;
    virtual bool IsFinished() override; 

protected:

    float GetCircleSpeedFromEnum();
    bool UpdateCircle(RGB (&frame)[100]);

    bool bClockwise;
    SpecialMoveAnimationDuration CircleSpeed;
    int CircleLoops = 0;
    uint8_t ColourIndex;
    uint8_t SecondColourIndex;
   
    int MinXCoord = 0;
    int MinYCoord = 0;
    int MaxXCoord = 0;
    int MaxYCoord = 0;
    int MidXCoord = 0;
    int MidYCoord = 0;

    float CurrentAngle = 0.0f;

    int NumLoopsDone = 0;

    uint32_t CurrentStateTime = 0;
    SMCircleColourState CurrentState = SMCircleColourState::SM_CIRCLE_STATE_NOTSTARTED;
};

#endif
