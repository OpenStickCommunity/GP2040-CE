#include "smwave.h"
#include "specialmovesystem.h"

#define WAVE_PREWAIT_MS 150
#define WAVE_POSTWAIT_MS 150
#define WAVE_OFFSET_PER_COORD 0.1f
#define WAVE_LENGTH 1.0f
#define WAVE_HOLDLOOPS 2

SMWave::SMWave(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : Animation(InRGBLights, InButtonCaseEffectType) 
{
    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if(LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type) == false)
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

void SMWave::SetOptionalParams(uint32_t OptionalParams)
{
    //FORMAT
    //4bit - SpecialMoveAnimationDirection - Wave direction
    //4bit - SpecialMoveAnimationDuration - Wave animation time
    //4bit - int - Num wave loops
    //4bit - SpecialMoveAnimationDivider - divides lights into sections and only plays on that section
    //8bit - Animation::colors index - wave color

    WaveDirection = (SpecialMoveAnimationDirection)(OptionalParams & 0xF);
    WaveSpeed = (SpecialMoveAnimationDuration)((OptionalParams >> 4) & 0xF);
    WaveLoops = ((OptionalParams >> 8) & 0xF);
    WaveDivider = (SpecialMoveAnimationDivider)((OptionalParams >> 12) & 0xF);
    ColorIndex = (uint8_t)((OptionalParams >> 16) & 0xFF);

    CurrentState = SMWaveState::SM_WAVE_STATE_PREFRAMES;
}

float SMWave::GetWaveSpeedFromEnum()
{
    //rads per frame
    float speed = 0;
    switch(WaveSpeed)
    {
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYSHORT:
        {
            speed = 0.1;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_SHORT:
        {
            speed = 0.15;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_MEDIUM:
        {
            speed = 0.2;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_LONG:
        {
            speed = 0.25;
        } break;
        case SpecialMoveAnimationDuration::SPECIALMOVE_DURATION_VERYLONG:
        {
            speed = 0.3;
        } break;
        default:
        {
            speed = 0.1;
        } break;
    }

    return speed;
}

bool SMWave::IsFinished()
{
    return CurrentState == SMWaveState::SM_WAVE_STATE_FINISHED;
}

void SMWave::Animate(RGB (&frame)[FRAME_MAX]) 
{
    UpdateTime();

    CurrentStateTime += updateTimeInMs;

    switch(CurrentState)
    {
        case SMWaveState::SM_WAVE_STATE_PREFRAMES:
        {
            UpdateWave(frame);
            if(CurrentStateTime > WAVE_PREWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMWaveState::SM_WAVE_STATE_RUNNING;
            }         
        } break;

        case SMWaveState::SM_WAVE_STATE_RUNNING:
        {
            if(UpdateWave(frame))
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMWaveState::SM_WAVE_STATE_POSTFRAMES;
            }
        } break;

        case SMWaveState::SM_WAVE_STATE_POSTFRAMES:
        {
            UpdateWave(frame);
            if(CurrentStateTime > WAVE_POSTWAIT_MS)
            {
                CurrentStateTime = 0.0f;
                CurrentState = SMWaveState::SM_WAVE_STATE_FINISHED;
            }         
        } break;

        case SMWaveState::SM_WAVE_STATE_FINISHED:
        case SMWaveState::SM_WAVE_STATE_NOTSTARTED:
        default:
        {
        } break;
    }
}

bool SMWave::UpdateWave(RGB (&frame)[FRAME_MAX])
{
    bool bFinished = true;

    WaveValue += GetWaveSpeedFromEnum();

    for(unsigned int lightIndex = 0; lightIndex < RGBLights->AllLights.size(); ++lightIndex)
    {
        if (LightTypeIsForAnimation(RGBLights->AllLights[lightIndex].Type))
        {
            uint8_t firstLightIndex = RGBLights->AllLights[lightIndex].FirstLedIndex;
            uint8_t lastLightIndex = firstLightIndex + RGBLights->AllLights[lightIndex].LedsPerLight;

            for(uint8_t ledIndex = firstLightIndex; ledIndex < lastLightIndex; ++ledIndex)
            {
                //default all to black if not running
                if(CurrentState != SMWaveState::SM_WAVE_STATE_RUNNING)
                {
                    frame[ledIndex] = ColorBlack;
                    continue;
                }

                bool bShouldUseLight = CheckDividerForLight(lightIndex);
                if(!bShouldUseLight)
                {
                    frame[ledIndex] = ColorBlack;
                    continue;
                }

                //This light is in use. Get the strength
                float lightStr = GetStrengthForLight(RGBLights->AllLights[lightIndex].Position.XPosition, RGBLights->AllLights[lightIndex].Position.YPosition);
                if(lightStr < 0)
                    lightStr = 0.0f;
                else
                    bFinished = false;

                frame[ledIndex] = BlendColor(ColorBlack, GetColorForIndex(ColorIndex), lightStr);    
            }
        }
    }

    return bFinished;
}

bool SMWave::CheckDividerForLight(unsigned int lightIndex)
{
    if(WaveDivider == SpecialMoveAnimationDivider::SPECIALMOVE_DIVIDER_FIRSTHALF_LIGHTS)
    {
        if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_LEFT || WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_RIGHT)
        {
            if(RGBLights->AllLights[lightIndex].Position.YPosition >= MidYCoord)
                return false;
        }
        else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_UP || WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_DOWN)
        {
            if(RGBLights->AllLights[lightIndex].Position.XPosition >= MidXCoord)
                return false;
        }
    }
    else if(WaveDivider == SpecialMoveAnimationDivider::SPECIALMOVE_DIVIDER_SECONDHALF_LIGHTS)
    {
        if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_LEFT || WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_RIGHT)
        {
            if(RGBLights->AllLights[lightIndex].Position.YPosition < MidYCoord)
                return false;
        }
        else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_UP || WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_DOWN)
        {
            if(RGBLights->AllLights[lightIndex].Position.XPosition < MidXCoord)
                return false;
        }
    }

    return true;
}

float SMWave::GetStrengthForLight(int XPos, int YPos)
{
    float thisWaveVal = WaveValue;
    if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_RIGHT)
    {    
        int xOffset = XPos - MinXCoord;
        thisWaveVal -= (WAVE_OFFSET_PER_COORD * xOffset);
    }
    else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_LEFT)
    {    
        int xOffset = MaxXCoord - XPos;
        thisWaveVal -= (WAVE_OFFSET_PER_COORD * xOffset);
    }
    else if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_UP)
    {    
        int yOffset = MaxYCoord - YPos;
        thisWaveVal -= (WAVE_OFFSET_PER_COORD * yOffset);
    }
    else// if(WaveDirection == SpecialMoveAnimationDirection::SPECIALMOVE_DIRECTION_DOWN)
    {    
        int yOffset = YPos - MinYCoord;
        thisWaveVal -= (WAVE_OFFSET_PER_COORD * yOffset);
    }

    int holdLoopsToUse = WaveLoops <= 1 ? WAVE_HOLDLOOPS : 0;
    int pulseLoopsToUse = WaveLoops - 1;
    int totalLoops = holdLoopsToUse + pulseLoopsToUse;
    float halfWaveLength = (WAVE_LENGTH / 2.0f);
    if(thisWaveVal < 0)
        return 0.0f; //not ready yet
    else if(thisWaveVal < WAVE_LENGTH)
        return thisWaveVal / WAVE_LENGTH; //fade in
    else if (thisWaveVal < (WAVE_LENGTH * (holdLoopsToUse)))
        return 1.0f;
    else if (thisWaveVal < (WAVE_LENGTH * totalLoops))
    {
        thisWaveVal -= WAVE_LENGTH;
        while(thisWaveVal > WAVE_LENGTH)
            thisWaveVal -= WAVE_LENGTH;
        float mul;
        if(thisWaveVal < halfWaveLength)
            mul = 1.0f - ((thisWaveVal / halfWaveLength) * 0.5f);
        else
            mul = 0.5f + (((thisWaveVal - halfWaveLength) / halfWaveLength) * 0.5f);

        return mul;
    }
    else if (thisWaveVal < (WAVE_LENGTH * (totalLoops + 1)))
        return 1.0 - ((thisWaveVal - (WAVE_LENGTH * totalLoops)) / WAVE_LENGTH);//in fade down
    
    //finished this light
    return -1.0f;
}