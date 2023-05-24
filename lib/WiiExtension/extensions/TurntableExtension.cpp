#include "ExtensionBase.h"
#include "TurntableExtension.h"

#include "WiiExtension.h"

void TurntableExtension::process(uint8_t *inputData) {
    // turntable accepts three data modes, but will only report as type 1, much like GHWT
    analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0x3F);
    analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[1] & 0x3F);

    analogState[TurntableAnalogs::TURNTABLE_LEFT]      = ((inputData[4] & 0x01) << 5) | ((inputData[3] & 0x1F) >> 0);
    analogState[TurntableAnalogs::TURNTABLE_RIGHT]     = ((inputData[0] & 0xC0) >> 3) | ((inputData[1] & 0xC0) >> 5) | ((inputData[2] & 0x80) >> 7);
    analogState[TurntableAnalogs::TURNTABLE_EFFECTS]   = (((inputData[2] & 0x60) >> 2) | ((inputData[3] & 0xE0) >> 5));
    analogState[TurntableAnalogs::TURNTABLE_CROSSFADE] = ((inputData[2] & 0x1E) >> 1);

    buttons[TurntableButtons::TURNTABLE_LEFT_RED]      = !((inputData[4] & 0x20) >> 5);
    buttons[WiiButtons::WII_BUTTON_MINUS]                  = !((inputData[4] & 0x10) >> 4);
    buttons[WiiButtons::WII_BUTTON_PLUS]                   = !((inputData[4] & 0x04) >> 2);
    buttons[TurntableButtons::TURNTABLE_RIGHT_RED]     = !((inputData[4] & 0x02) >> 1);

    buttons[TurntableButtons::TURNTABLE_LEFT_BLUE]     = !((inputData[5] & 0x80) >> 7);
    buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN]   = !((inputData[5] & 0x20) >> 5);                       
    buttons[TurntableButtons::TURNTABLE_EUPHORIA]      = !((inputData[5] & 0x10) >> 4);
    buttons[TurntableButtons::TURNTABLE_LEFT_GREEN]    = !((inputData[5] & 0x08) >> 3);
    buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE]    = !((inputData[5] & 0x04) >> 2);

#if WII_EXTENSION_DEBUG==true
    //printf("LR=%1d LG=%1d LB=%1d\n", buttons[TurntableButtons::TURNTABLE_LEFT_RED], buttons[TurntableButtons::TURNTABLE_LEFT_GREEN], buttons[TurntableButtons::TURNTABLE_LEFT_BLUE]);
    //printf("RR=%1d RG=%1d RB=%1d\n", buttons[TurntableButtons::TURNTABLE_RIGHT_RED], buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN], buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE]);
    //printf("-=%1d EU=%1d +=%1d\n", buttons[WiiButtons::BUTTON_MINUS], buttons[TurntableButtons::TURNTABLE_EUPHORIA], buttons[WiiButtons::BUTTON_PLUS]);
    //printf("LTT=%4d RTT=%4d CF=%4d ED=%4d\n", analogState[TurntableAnalogs::TURNTABLE_LEFT], analogState[TurntableAnalogs::TURNTABLE_RIGHT], analogState[TurntableAnalogs::TURNTABLE_CROSSFADE], analogState[TurntableAnalogs::TURNTABLE_EFFECTS]);

    //for (int i = 0; i < result; ++i) {
    //    if (_lastRead[i] != inputData[i]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", i, BYTE_TO_BINARY(inputData[i]));
    //}
#endif
}