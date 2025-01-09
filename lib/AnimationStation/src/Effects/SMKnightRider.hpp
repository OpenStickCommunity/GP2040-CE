#ifndef _SM_KNIGHTRIDER_H_
#define _SM_KNIGHTRIDER_H_

#include "../Animation.hpp"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../AnimationStation.hpp"
#include "../SpecialMoveSystem.hpp"
#include "SMWave.hpp"

class SMKnightRider : public SMWave {
public:
    SMKnightRider(Lights& InRGBLights);
    ~SMKnightRider() { };

    virtual void Animate(RGB (&frame)[100]) override;
    virtual void SetOptionalParams(uint32_t OptionalParams) override;
 
protected:

    int KRLoops = 0;
    bool bOnReturnLeg = false;
};

#endif
