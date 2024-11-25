#include "SMPulseColour.hpp"
#include "SpecialMoveSystem.hpp"

#define PULSE_PREWAIT_MS 150
#define PULSE_POSTWAIT_MS 150

SMPulseColour::SMPulseColour(Lights& InRGBLights) : Animation(InRGBLights) 
{
}

void SMPulseColour::SetOptionalParams(uint32_t OptionalParams)
{
    //FORMAT
    //4bit - SpecialMoveAnimationColourFadeSpeed - fade in speed
    //4bit - SpecialMoveAnimationDuration - Hold colour time
    //4bit - SpecialMoveAnimationColourFadeSpeed - fade out speed
    //8bit - Animation::colors index - pulse colour

    FadeInSpeed = (SpecialMoveAnimationColourFadeSpeed)(OptionalParams & 0xF);
    HoldColourTime = (SpecialMoveAnimationDuration)((OptionalParams >> 4) & 0xF);
    FadeOutSpeed = (SpecialMoveAnimationColourFadeSpeed)((OptionalParams >> 8) & 0xF);
    ColourIndex = (uint8_t)((OptionalParams >> 16) & 0xFF);

    CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_PREFRAMES;
}

uint32_t SMPulseColour::GetFadeTimeFromEnum(SpecialMoveAnimationColourFadeSpeed FadeEnum)
{
    //in Ms
    uint32_t fadeTime = 0;
    switch(FadeEnum)
    {
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_INSTANT:
        {
            fadeTime = 0;
        } break;
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_VERYFAST:
        {
            fadeTime = 100;
        } break;
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_FAST:
        {
            fadeTime = 200;
        } break;
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_MEDIUM:
        {
            fadeTime = 500;
        } break;
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_SLOW:
        {
            fadeTime = 750;
        } break;
        case SpecialMoveAnimationColourFadeSpeed::SPECIALMOVE_COLOURFADE_VERYSLOW:
        {
            fadeTime = 1000;
        } break;
        default:
        {
            fadeTime = 0;
        } break;
    }

    return fadeTime;
}

uint32_t SMPulseColour::GetHoldTimeFromEnum(SpecialMoveAnimationDuration FadeEnum)
{
    //in Ms
    uint32_t holdTime = 0;
    switch(FadeEnum)
    {
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYSHORT:
        {
            holdTime = 250;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_SHORT:
        {
            holdTime = 500;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_MEDIUM:
        {
            holdTime = 750;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_LONG:
        {
            holdTime = 1000;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYLONG:
        {
            holdTime = 1250;
        } break;
        default:
        {
            holdTime = 0;
        } break;
    }

    return holdTime;
}

bool SMPulseColour::IsFinished()
{
    return CurrentState == SMPulseColourState::SM_PULSE_COLOUR_STATE_FINISHED;
}

void SMPulseColour::Animate(RGB (&frame)[100]) 
{
    UpdateTime();

    CurrentStateTime += updateTimeInMs;

    float thisFrameFade;
    switch(CurrentState)
    {
        case SMPulseColourState::SM_PULSE_COLOUR_STATE_PREFRAMES:
        {
            thisFrameFade = 0.0f;
            if(CurrentStateTime > PULSE_PREWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_FADEIN;
            }         
        } break;

        case SMPulseColourState::SM_PULSE_COLOUR_STATE_FADEIN:
        {
            uint32_t fadeInTime = GetFadeTimeFromEnum(FadeInSpeed);
            if(CurrentStateTime > fadeInTime)
            {
                thisFrameFade = 1.0f;
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_HOLD;
            }
            else
            {
                thisFrameFade = (float)CurrentStateTime / (float)fadeInTime;
            }
        } break;

        case SMPulseColourState::SM_PULSE_COLOUR_STATE_HOLD:
        {
            thisFrameFade = 1.0f;
            uint32_t holdTime = GetHoldTimeFromEnum(HoldColourTime);
            if(CurrentStateTime > holdTime)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_FADEOUT;
            }
        } break;

        case SMPulseColourState::SM_PULSE_COLOUR_STATE_FADEOUT:
        {
            uint32_t fadeOutTime = GetFadeTimeFromEnum(FadeOutSpeed);
            if(CurrentStateTime > fadeOutTime)
            {
                thisFrameFade = 0.0f;
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_POSTFRAMES;
            }
            else
            {
                thisFrameFade = 1.0f - ((float)CurrentStateTime / (float)fadeOutTime);
            }
        } break;

        case SMPulseColourState::SM_PULSE_COLOUR_STATE_POSTFRAMES:
        {
            thisFrameFade = 0.0f;
            if(CurrentStateTime > PULSE_POSTWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColourState::SM_PULSE_COLOUR_STATE_FINISHED;
            }         
        } break;

        case SMPulseColourState::SM_PULSE_COLOUR_STATE_FINISHED:
        case SMPulseColourState::SM_PULSE_COLOUR_STATE_NOTSTARTED:
        default:
        {
            thisFrameFade = 0.0f;
        } break;
    }

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if (LightTypeIsForSpecialMoveAnimation(RGBLights->AllLights[lightIndex].Type))
        {
            uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
                frame[ledIndex] = BlendColor(ColorBlack, colors[ColourIndex], thisFrameFade);    
            }
        }
    }
}