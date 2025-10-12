#include "smpulsecolor.h"
#include "specialmovesystem.h"

#define PULSE_PREWAIT_MS 150
#define PULSE_POSTWAIT_MS 150

SMPulseColor::SMPulseColor(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
}

void SMPulseColor::SetOptionalParams(uint32_t OptionalParams)
{
    //FORMAT
    //4bit - SpecialMoveAnimationColorFadeSpeed - fade in speed
    //4bit - SpecialMoveAnimationDuration - Hold color time
    //4bit - SpecialMoveAnimationColorFadeSpeed - fade out speed
    //8bit - Animation::colors index - pulse color

    FadeInSpeed = (SpecialMoveAnimationColorFadeSpeed)(OptionalParams & 0xF);
    HoldColorTime = (SpecialMoveAnimationDuration)((OptionalParams >> 4) & 0xF);
    FadeOutSpeed = (SpecialMoveAnimationColorFadeSpeed)((OptionalParams >> 8) & 0xF);
    ColorIndex = (uint8_t)((OptionalParams >> 16) & 0xFF);

    CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_PREFRAMES;
}

uint32_t SMPulseColor::GetFadeTimeFromEnum(SpecialMoveAnimationColorFadeSpeed FadeEnum)
{
    //in Ms
    uint32_t fadeTime = 0;
    switch(FadeEnum)
    {
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_INSTANT:
        {
            fadeTime = 0;
        } break;
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_VERYFAST:
        {
            fadeTime = 100;
        } break;
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_FAST:
        {
            fadeTime = 200;
        } break;
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_MEDIUM:
        {
            fadeTime = 500;
        } break;
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_SLOW:
        {
            fadeTime = 750;
        } break;
        case SpecialMoveAnimationColorFadeSpeed::SPECIALMOVE_COLORFADE_VERYSLOW:
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

uint32_t SMPulseColor::GetHoldTimeFromEnum(SpecialMoveAnimationDuration FadeEnum)
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

bool SMPulseColor::IsFinished()
{
    return CurrentState == SMPulseColorState::SM_PULSE_COLOR_STATE_FINISHED;
}

void SMPulseColor::Animate(RGB (&frame)[FRAME_MAX]) 
{
    UpdateTime();

    CurrentStateTime += updateTimeInMs;

    float thisFrameFade;
    switch(CurrentState)
    {
        case SMPulseColorState::SM_PULSE_COLOR_STATE_PREFRAMES:
        {
            thisFrameFade = 0.0f;
            if(CurrentStateTime > PULSE_PREWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_FADEIN;
            }         
        } break;

        case SMPulseColorState::SM_PULSE_COLOR_STATE_FADEIN:
        {
            uint32_t fadeInTime = GetFadeTimeFromEnum(FadeInSpeed);
            if(CurrentStateTime > fadeInTime)
            {
                thisFrameFade = 1.0f;
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_HOLD;
            }
            else
            {
                thisFrameFade = (float)CurrentStateTime / (float)fadeInTime;
            }
        } break;

        case SMPulseColorState::SM_PULSE_COLOR_STATE_HOLD:
        {
            thisFrameFade = 1.0f;
            uint32_t holdTime = GetHoldTimeFromEnum(HoldColorTime);
            if(CurrentStateTime > holdTime)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_FADEOUT;
            }
        } break;

        case SMPulseColorState::SM_PULSE_COLOR_STATE_FADEOUT:
        {
            uint32_t fadeOutTime = GetFadeTimeFromEnum(FadeOutSpeed);
            if(CurrentStateTime > fadeOutTime)
            {
                thisFrameFade = 0.0f;
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_POSTFRAMES;
            }
            else
            {
                thisFrameFade = 1.0f - ((float)CurrentStateTime / (float)fadeOutTime);
            }
        } break;

        case SMPulseColorState::SM_PULSE_COLOR_STATE_POSTFRAMES:
        {
            thisFrameFade = 0.0f;
            if(CurrentStateTime > PULSE_POSTWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMPulseColorState::SM_PULSE_COLOR_STATE_FINISHED;
            }         
        } break;

        case SMPulseColorState::SM_PULSE_COLOR_STATE_FINISHED:
        case SMPulseColorState::SM_PULSE_COLOR_STATE_NOTSTARTED:
        default:
        {
            thisFrameFade = 0.0f;
        } break;
    }

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if (LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type))
        {
            uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
                frame[ledIndex] = BlendColor(ColorBlack, GetColorForIndex(ColorIndex), thisFrameFade);    
            }
        }
    }
}