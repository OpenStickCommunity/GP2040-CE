#include "jiggletwostaticcolor.h"
#include "staticcolor.h"

#define JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK 1

JiggleTwoStaticColor::JiggleTwoStaticColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : StaticColor(InRGBLights, InButtonCaseEffectType) 
{
}

JiggleTwoStaticColor::JiggleTwoStaticColor(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : StaticColor(InRGBLights, InPressedPins) 
{
}

RGB JiggleTwoStaticColor::AdjustColor(RGB InColor)
{
    RGB otherColor;
    
    //calculate other color and advance rainbow effect if required
    bool bUseRainbow = false;
    if(isButtonAnimation)
    {
        otherColor = RGB(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].pressedSpecialColor);
        bUseRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bPressedSpecialColorIsRainbow;
    }
    else
    {
        otherColor = RGB(AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].nonPressedSpecialColor);
        bUseRainbow = AnimationStation::options.profiles[AnimationStation::options.baseProfileIndex].bNonPressedSpecialColorIsRainbow;
    }
    if(bUseRainbow)
    {
        if(!RainbowWheelReversed)
        {
        RainbowWheelFrame += JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK;
        if(RainbowWheelFrame == 255)
            RainbowWheelReversed = true;
        }
        else
        {
        RainbowWheelFrame -= JIGGLE_RAINBOW_FRAME_CHANGE_PER_TICK;
        if(RainbowWheelFrame == 0)
            RainbowWheelReversed = false;
        }

        otherColor = RGB::wheel(RainbowWheelFrame);
    }

    RGB outColor;
    float rDiff = (float)otherColor.r - (float)InColor.r;
    float newR = (float)InColor.r + (rDiff * (((float)(rand() % 100) / 100.0f)));
    if(newR > 255.0f)
        newR = 255;
    if(newR < 0)
        newR = 0;
    outColor.r = (int)newR;

    float gDiff = (float)otherColor.g - (float)InColor.g;
    float newG = (float)InColor.g + (gDiff * (((float)(rand() % 100) / 100.0f)));
    if(newG > 255.0f)
        newG = 255;
    if(newG < 0)
        newG = 0;
    outColor.g = (int)newG;

    float bDiff = (float)otherColor.b - (float)InColor.b;
    float newB = (float)InColor.b + (bDiff * (((float)(rand() % 100) / 100.0f)));
    if(newB > 255.0f)
        newB = 255;
    if(newB < 0)
        newB = 0;
    outColor.b = (int)newB;

    return outColor;
}
 