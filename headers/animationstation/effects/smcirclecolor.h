#ifndef _SM_CIRCLECOLOR_H_
#define _SM_CIRCLECOLOR_H_

#include "../animation.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animationstation.h"
#include "../specialmovesystem.h"

typedef enum
{
  SM_CIRCLE_STATE_NOTSTARTED,
  SM_CIRCLE_STATE_PREFRAMES,
  SM_CIRCLE_STATE_RUNNING,
  SM_CIRCLE_STATE_POSTFRAMES,
  SM_CIRCLE_STATE_FINISHED,
} SMCircleColorState;

class SMCircleColor : public Animation {
public:
    SMCircleColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    ~SMCircleColor() { };

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;

    virtual void SetOptionalParams(uint32_t OptionalParams) override;
    virtual bool IsFinished() override; 

protected:

    float GetCircleSpeedFromEnum();
    bool UpdateCircle(RGB (&frame)[FRAME_MAX]);

    bool bClockwise;
    SpecialMoveAnimationDuration CircleSpeed;
    int CircleLoops = 0;
    uint8_t ColorIndex;
    uint8_t SecondColorIndex;
   
    int MinXCoord = 0;
    int MinYCoord = 0;
    int MaxXCoord = 0;
    int MaxYCoord = 0;
    int MidXCoord = 0;
    int MidYCoord = 0;

    float CurrentAngle = 0.0f;

    int NumLoopsDone = 0;

    uint32_t CurrentStateTime = 0;
    SMCircleColorState CurrentState = SMCircleColorState::SM_CIRCLE_STATE_NOTSTARTED;
};

#endif
