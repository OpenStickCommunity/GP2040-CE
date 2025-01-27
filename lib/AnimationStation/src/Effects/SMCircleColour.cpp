#include "SMCircleColour.hpp"
#include "SpecialMoveSystem.hpp"
#include <math.h>

#define CIRCLE_PREWAIT_MS 150
#define CIRCLE_POSTWAIT_MS 150
#define TWOCIRCLES 12.5666
#define ONECIRCLE (TWOCIRCLES / 2.0f)

SMCircleColour::SMCircleColour(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type) == false)
            continue;

        if(RGBLights->AllLights[lightIndex].Position.XPosition > MaxXCoord)
            MaxXCoord = RGBLights->AllLights[lightIndex].Position.XPosition;
        if(RGBLights->AllLights[lightIndex].Position.YPosition > MaxYCoord)
            MaxYCoord = RGBLights->AllLights[lightIndex].Position.YPosition;

        if(lightIndex == 0 || RGBLights->AllLights[lightIndex].Position.XPosition < MinXCoord)
            MinXCoord = RGBLights->AllLights[lightIndex].Position.XPosition;
        if(lightIndex == 0 || RGBLights->AllLights[lightIndex].Position.YPosition < MinYCoord)
            MinYCoord = RGBLights->AllLights[lightIndex].Position.YPosition; 
    }

    MidXCoord = MinXCoord + ((MaxXCoord - MinXCoord) / 2);
    MidYCoord = MinYCoord + ((MaxYCoord - MinYCoord) / 2);
}

void SMCircleColour::SetOptionalParams(uint32_t OptionalParams)
{
    //FORMAT
    //4bit - bool - Clockwise
    //4bit - SpecialMoveAnimationDuration - Circle animation time
    //4bit - int - Num circle loops
    //8bit - Animation::colors index - Circle colour
    //8bit - Animation::colors index - Second Circle colour

    bClockwise = (OptionalParams & 0xF) != 0;
    CircleSpeed = (SpecialMoveAnimationDuration)((OptionalParams >> 4) & 0xF);
    CircleLoops = ((OptionalParams >> 8) & 0xF);
    ColourIndex = (uint8_t)((OptionalParams >> 12) & 0xFF);
    SecondColourIndex = (uint8_t)((OptionalParams >> 20) & 0xFF);

    CurrentState = SMCircleColourState::SM_CIRCLE_STATE_PREFRAMES;
}

float SMCircleColour::GetCircleSpeedFromEnum()
{
    //rads per frame
    float speed = 0;
    switch(CircleSpeed)
    {
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYSHORT:
        {
            speed = 0.8;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_SHORT:
        {
            speed = 0.6;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_MEDIUM:
        {
            speed = 0.4;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_LONG:
        {
            speed = 0.2;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYLONG:
        {
            speed = 0.1;
        } break;
        default:
        {
            speed = 0.1;
        } break;
    }

    return speed;
}

bool SMCircleColour::IsFinished()
{
    return CurrentState == SMCircleColourState::SM_CIRCLE_STATE_FINISHED;
}

void SMCircleColour::Animate(RGB (&frame)[100]) 
{
    UpdateTime();

    CurrentStateTime += updateTimeInMs;

    switch(CurrentState)
    {
        case SMCircleColourState::SM_CIRCLE_STATE_PREFRAMES:
        {
            UpdateCircle(frame);
            if(CurrentStateTime > CIRCLE_PREWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMCircleColourState::SM_CIRCLE_STATE_RUNNING;
            }         
        } break;

        case SMCircleColourState::SM_CIRCLE_STATE_RUNNING:
        {
            if(UpdateCircle(frame))
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMCircleColourState::SM_CIRCLE_STATE_POSTFRAMES;
            }
        } break;

        case SMCircleColourState::SM_CIRCLE_STATE_POSTFRAMES:
        {
            UpdateCircle(frame);
            if(CurrentStateTime > CIRCLE_POSTWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMCircleColourState::SM_CIRCLE_STATE_FINISHED;
            }         
        } break;

        case SMCircleColourState::SM_CIRCLE_STATE_FINISHED:
        case SMCircleColourState::SM_CIRCLE_STATE_NOTSTARTED:
        default:
        {
        } break;
    }
}

bool SMCircleColour::UpdateCircle(RGB (&frame)[100])
{
    if(CurrentState == SMCircleColourState::SM_CIRCLE_STATE_RUNNING)
        CurrentAngle += GetCircleSpeedFromEnum();

    bool inSecondCircle = CurrentAngle > ONECIRCLE;
    float currentAngleModOneCircle = inSecondCircle ? CurrentAngle - ONECIRCLE : CurrentAngle;

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if (LightTypeIsForNonPressedAnimation(RGBLights->AllLights[lightIndex].Type))
        {
            uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

            //Work out angle to light
            bool useSecondColour;
            float xToLight = RGBLights->AllLights[lightIndex].Position.XPosition - MidXCoord;
            float yToLight = RGBLights->AllLights[lightIndex].Position.YPosition - MidYCoord;
            float magnitude = std::sqrt((xToLight * xToLight) + (yToLight * yToLight));
            float angleBetweenUpAndLight = std::acos(-yToLight / magnitude); //negate Y here as the button coords are 0,0 in the top left
            if(xToLight < 0)
            {
                //its on left side of center
                angleBetweenUpAndLight = ONECIRCLE - angleBetweenUpAndLight;
            }

            //should we light this light or turn it to the "off" colour?
            useSecondColour = inSecondCircle ? false : true; 
            if(bClockwise)
            {
                if(angleBetweenUpAndLight < currentAngleModOneCircle)
                    useSecondColour = !useSecondColour; 
            }
            else
            {
                if(angleBetweenUpAndLight > ONECIRCLE - currentAngleModOneCircle)
                    useSecondColour = !useSecondColour; 
            }

            RGB colToUse = ColorBlack; //black
            if(NumLoopsDone == 0 && !inSecondCircle && useSecondColour)
                colToUse = ColorBlack;
            else if(NumLoopsDone == (CircleLoops-1) && inSecondCircle && useSecondColour)
                colToUse = ColorBlack;
            else if(useSecondColour)
                colToUse = GetColorForIndex(SecondColourIndex);
            else
                colToUse = GetColorForIndex(ColourIndex);

            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
                //default all to black if not running
                if(CurrentState != SMCircleColourState::SM_CIRCLE_STATE_RUNNING)
                {
                    frame[ledIndex] = ColorBlack;
                    continue;
                }

                frame[ledIndex] = colToUse;
            }
        }
    }

    if(CurrentAngle > TWOCIRCLES)
    {
        CurrentAngle = 0;
        NumLoopsDone++;
    }

    return NumLoopsDone == CircleLoops;
}