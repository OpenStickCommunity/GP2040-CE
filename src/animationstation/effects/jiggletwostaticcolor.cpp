#include "jiggletwostaticcolor.h"
#include "jigglestaticcolor.h"

#define JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK 1

JiggleTwoStaticColor::JiggleTwoStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : JiggleStaticColor(InRGBLights, InButtonCaseEffectType) 
{
    for(int index = 0; index < FRAME_MAX; ++index)
    {
        RainbowWheelFrame[index] = rand() % 255;
        RainbowWheelReversed[index] = (rand() % 100) > 50;
    }
}

JiggleTwoStaticColor::JiggleTwoStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : JiggleStaticColor(InRGBLights, InPressedPins) 
{
    for(int index = 0; index < FRAME_MAX; ++index)
    {
        RainbowWheelFrame[index] = rand() % 255;
        RainbowWheelReversed[index] = (rand() % 100) > 50;
    }
}

RGB JiggleTwoStaticColor::AdjustColor(int ledIndex, RGB InColor, LightType lightType)
{
    RGB otherColor;
    
    //calculate other color and advance rainbow effect if required
    bool bUseRainbow = false;
    if(isButtonAnimation)
    {
        otherColor = RGB(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].pressedSpecialColor);
        bUseRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bPressedSpecialColorIsRainbow;
    }
    else if(lightType == LightType::LightType_ActionButton)
    {
        otherColor = RGB(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor);
        bUseRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bNonPressedSpecialColorIsRainbow;
    }
    else
    {
        otherColor = RGB(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].caseSpecialColor);
        bUseRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bCaseSpecialColorIsRainbow;
    }

    if(bUseRainbow)
    {
        if(!RainbowWheelReversed[ledIndex])
        {
            RainbowWheelFrame[ledIndex] += JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK;
            if(RainbowWheelFrame[ledIndex] >= 255)
            {
                RainbowWheelReversed[ledIndex] = true;
                RainbowWheelFrame[ledIndex] = 255;
            }
        }
        else
        {
            RainbowWheelFrame[ledIndex] -= JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK;
            if(RainbowWheelFrame[ledIndex] <= 0)
            {
                RainbowWheelReversed[ledIndex] = false;
                RainbowWheelFrame[ledIndex] = 0;
            }
        }

        otherColor = RGB::wheel(RainbowWheelFrame[ledIndex]);
    }

    return AdvanceColor(ledIndex, InColor, otherColor);
}
 