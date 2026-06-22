#include "idletimeout.h"

IdleTimeout::IdleTimeout(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType) : StaticColor(InRGBLights, InButtonCaseEffectType) 
{
}

IdleTimeout::IdleTimeout(Lights& InRGBLights, std::vector<int32_t> &InPressedPins) : StaticColor(InRGBLights, EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_ONLY) 
{
}

RGB IdleTimeout::GetNonPressedColorForLight(uint32_t LightIndex)
{
    return ColorBlack;
}
