#ifndef _SM_PULSE_COLOUR_H_
#define _SM_PULSE_COLOUR_H_

#include "../Animation.hpp"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../AnimationStation.hpp"
#include "../SpecialMoveSystem.hpp"

typedef enum
{
  SM_PULSE_COLOUR_STATE_NOTSTARTED,
  SM_PULSE_COLOUR_STATE_PREFRAMES,
  SM_PULSE_COLOUR_STATE_FADEIN,
  SM_PULSE_COLOUR_STATE_HOLD,
  SM_PULSE_COLOUR_STATE_FADEOUT,
  SM_PULSE_COLOUR_STATE_POSTFRAMES,
  SM_PULSE_COLOUR_STATE_FINISHED,
} SMPulseColourState;

class SMPulseColour : public Animation {
public:
    SMPulseColour(Lights& InRGBLights);
    ~SMPulseColour() { };

    virtual void Animate(RGB (&frame)[100]) override;

    virtual void SetOptionalParams(uint32_t OptionalParams) override;
    virtual bool IsFinished() override; 

protected:

    uint32_t GetFadeTimeFromEnum(SpecialMoveAnimationColourFadeSpeed FadeEnum);
    uint32_t GetHoldTimeFromEnum(SpecialMoveAnimationDuration FadeEnum);

    SpecialMoveAnimationColourFadeSpeed FadeInSpeed;
    SpecialMoveAnimationDuration HoldColourTime;
    SpecialMoveAnimationColourFadeSpeed FadeOutSpeed;
    uint8_t ColourIndex;
    
    uint32_t CurrentStateTime = 0;
    SMPulseColourState CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_NOTSTARTED;
};

#endif
