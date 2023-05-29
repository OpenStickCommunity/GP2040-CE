#include "ExtensionBase.h"
#include "DrumExtension.h"

#include "WiiExtension.h"

void DrumExtension::process(uint8_t *inputData) {
    // drums accept three data modes, but will only report as type 1, much like GHWT
    analogState[WiiAnalogs::WII_ANALOG_LEFT_X] = (inputData[0] & 0x3F);
    analogState[WiiAnalogs::WII_ANALOG_LEFT_Y] = (inputData[1] & 0x3F);

    buttons[WiiButtons::WII_BUTTON_MINUS]      = !((inputData[4] & 0x10) >> 4);
    buttons[WiiButtons::WII_BUTTON_PLUS]       = !((inputData[4] & 0x04) >> 2);

    buttons[DrumButtons::DRUM_ORANGE]          = !((inputData[5] & 0x80) >> 7);
    buttons[DrumButtons::DRUM_RED]             = !((inputData[5] & 0x40) >> 6);
    buttons[DrumButtons::DRUM_YELLOW]          = !((inputData[5] & 0x20) >> 5);
    buttons[DrumButtons::DRUM_GREEN]           = !((inputData[5] & 0x10) >> 4);
    buttons[DrumButtons::DRUM_BLUE]            = !((inputData[5] & 0x08) >> 3);
    buttons[DrumButtons::DRUM_PEDAL]           = !((inputData[5] & 0x04) >> 2);

#if WII_EXTENSION_DEBUG==true
    //printf("O=%1d R=%1d Y=%1d G=%1d B=%1d P=%1d\n", buttons[DrumButtons::DRUM_ORANGE], buttons[DrumButtons::DRUM_RED], buttons[DrumButtons::DRUM_YELLOW], buttons[DrumButtons::DRUM_GREEN], buttons[DrumButtons::DRUM_BLUE], buttons[DrumButtons::DRUM_PEDAL]);
    //printf("-=%1d +=%1d Joy X=%4d Y=%4d\n", buttons[WiiButtons::BUTTON_MINUS], buttons[WiiButtons::BUTTON_PLUS], analogState[WiiAnalogs::ANALOG_LEFT_X], analogState[WiiAnalogs::ANALOG_LEFT_Y]);
    //if (_lastRead[2] != inputData[2]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", 2, BYTE_TO_BINARY(inputData[2]));
    //if (_lastRead[3] != inputData[3]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", 3, BYTE_TO_BINARY(inputData[3]));
    //if (_lastRead[5] != inputData[5]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", 5, BYTE_TO_BINARY(inputData[5]));
    //for (int i = 0; i < 8; ++i) {
    //    //if (_lastRead[i] != inputData[i]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", i, BYTE_TO_BINARY(inputData[i]));
    //    _lastRead[i] = inputData[i];
    //}
#endif
}