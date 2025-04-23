#ifndef _SM_PULSE_COLOR_H_
#define _SM_PULSE_COLOR_H_

#include "../animation.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animationstation.h"
#include "../specialmovesystem.h"

typedef enum
{
  SM_PULSE_COLOR_STATE_NOTSTARTED,
  SM_PULSE_COLOR_STATE_PREFRAMES,
  SM_PULSE_COLOR_STATE_FADEIN,
  SM_PULSE_COLOR_STATE_HOLD,
  SM_PULSE_COLOR_STATE_FADEOUT,
  SM_PULSE_COLOR_STATE_POSTFRAMES,
  SM_PULSE_COLOR_STATE_FINISHED,
} SMPulseColorState;

class SMPulseColor : public Animation {
public:
    SMPulseColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    ~SMPulseColor() { };

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;

    virtual void SetOptionalParams(uint32_t OptionalParams) override;
    virtual bool IsFinished() override; 

protected:

    uint32_t GetFadeTimeFromEnum(SpecialMoveAnimationColorFadeSpeed FadeEnum);
    uint32_t GetHoldTimeFromEnum(SpecialMoveAnimationDuration FadeEnum);

    SpecialMoveAnimationColorFadeSpeed FadeInSpeed;
    SpecialMoveAnimationDuration HoldColorTime;
    SpecialMoveAnimationColorFadeSpeed FadeOutSpeed;
    uint8_t ColorIndex;
    
    uint32_t CurrentStateTime = 0;
    SMPulseColorState CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_NOTSTARTED;
};

#endif
