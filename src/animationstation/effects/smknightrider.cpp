#include "smwave.h"
#include "smknightrider.h"
#include "specialmovesystem.h"

SMKnightRider::SMKnightRider(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : SMWave(InRGBLights, InButtonCaseEffectType) 
{
}

void SMKnightRider::SetOptionalParams(uint32_t OptionalParams)
{
    //FORMAT
    //4bit - SpecialMoveAnimationDirection - Wave direction
    //4bit - SpecialMoveAnimationDuration - Wave animation time
    //4bit - int - Num wave loops
    //4bit - SpecialMoveAnimationDivider - divides lights into sections and only plays on that section
    //8bit - Animation::colors index - wave color

    WaveDirection = (SpecialMoveAnimationDirection)(OptionalParams & 0xF);
    WaveSpeed = (SpecialMoveAnimationDuration)((OptionalParams >> 4) & 0xF);
    WaveLoops = 1;
    KRLoops = ((OptionalParams >> 8) & 0xF);
    WaveDivider = (SpecialMoveAnimationDivider)((OptionalParams >> 12) & 0xF);
    ColorIndex = (uint8_t)((OptionalParams >> 16) & 0xFF);

    CurrentState = SMWaveState::SM_WAVE_STATE_PREFRAMES;
}

void SMKnightRider::Animate(RGB (&frame)[FRAME_MAX]) 
{
    switch(CurrentState)
    {
        case SMWaveState::SM_WAVE_STATE_POSTFRAMES:
        {
            //flip direction
            if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_LEFT)
                WaveDirection = SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_RIGHT;
            else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_RIGHT)
                WaveDirection = SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_LEFT;
            else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_UP)
                WaveDirection = SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_DOWN;
            else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_DOWN)
                WaveDirection = SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_UP;

            if(bOnReturnLeg == true)
            {
                KRLoops--;
            }
            bOnReturnLeg = !bOnReturnLeg;

            //restart wave if not finished
            if(KRLoops > 0)
            {
                //must call update wave here to make sure we put black on all the leds
                UpdateWave(frame);
                
                CurrentStateTime = 0.0f;
                CurrentState = SMWaveState::SM_WAVE_STATE_RUNNING;
                WaveValue = 0.0f;

                //dont fall through to super
                return;
            }
        } break;

        default:
        {
        } break;
    }

    SMWave::Animate(frame);
}