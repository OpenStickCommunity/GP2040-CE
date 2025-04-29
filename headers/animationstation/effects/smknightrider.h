#ifndef _SM_KNIGHTRIDER_H_
#define _SM_KNIGHTRIDER_H_

#include "../animation.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../animationstation.h"
#include "../specialmovesystem.h"
#include "smwave.h"

class SMKnightRider : public SMWave {
public:
    SMKnightRider(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
    ~SMKnightRider() { };

    virtual void Animate(RGB (&frame)[FRAME_MAX]) override;
    virtual void SetOptionalParams(uint32_t OptionalParams) override;
 
protected:

    int KRLoops = 0;
    bool bOnReturnLeg = false;
};

#endif
