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

uint8_t DrumExtension::prepareOutput() {
    uint16_t lx = 0;
    uint16_t ly = 0;
    uint16_t rx = 0;
    uint16_t ry = 0;
    uint16_t lt = 0;
    uint16_t rt = 0;
    // expect all source analog sticks at 16 bit and scale down
    // expect all source analog triggers at 8 bit and scale down
    if (getDataType() == WII_DATA_TYPE_1) {
        lx = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_LEFT_X] * 0x3F) / 0xFFFF);
        ly = (uint8_t)(0x3F - ((analogState[WiiAnalogs::WII_ANALOG_LEFT_Y] * 0x3F) / 0xFFFF));
        rx = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_RIGHT_X] * 0x1F) / 0xFFFF);
        ry = (uint8_t)(0x1F - ((analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y] * 0x1F) / 0xFFFF));
        lt = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER] * 0x1F) / 0xFF);
        rt = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER] * 0x1F) / 0xFF);

        controllerData[0x00] = 0xC0 | ((lx+1) & 0x3F);
        controllerData[0x01] = 0xC0 | (ly & 0x3F);
        controllerData[0x02] = 0xFF;
        controllerData[0x03] = 0xFF;
        controllerData[0x04] = (
            ((1                                                 << 0)) | 
            ((1                                                 << 1)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_PLUS] & 0x01)    << 2)) | 
            ((1                                                 << 3)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_MINUS] & 0x01)   << 4)) | 
            ((1                                                 << 5)) | 
            ((1                                                 << 6)) | 
            ((1                                                 << 7))
        );
        controllerData[0x05] = (
            ((1                                                 << 0)) | 
            ((1                                                 << 1)) | 
            ((!(buttons[GuitarButtons::GUITAR_PEDAL] & 0x01)    << 2)) | 
            ((!(buttons[GuitarButtons::GUITAR_YELLOW] & 0x01)   << 3)) | 
            ((!(buttons[GuitarButtons::GUITAR_GREEN] & 0x01)    << 4)) | 
            ((!(buttons[GuitarButtons::GUITAR_BLUE] & 0x01)     << 5)) | 
            ((!(buttons[GuitarButtons::GUITAR_RED] & 0x01)      << 6)) | 
            ((!(buttons[GuitarButtons::GUITAR_ORANGE] & 0x01)   << 7))
        );
        return 6;
    } else if (getDataType() == WII_DATA_TYPE_2) {
        lx = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_LEFT_X] * 0x3FF) / 0xFFFF);
        ly = (uint8_t)(0x3F - ((analogState[WiiAnalogs::WII_ANALOG_LEFT_Y] * 0x3FF) / 0xFFFF));
        rx = (uint8_t)((analogState[WiiAnalogs::WII_ANALOG_RIGHT_X] * 0x3FF) / 0xFFFF);
        ry = (uint8_t)(0x1F - ((analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y] * 0x3FF) / 0xFFFF));
        lt = analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER];
        rt = analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER];

        controllerData[0x00] = ((lx & 0x03FC) >> 2);
        controllerData[0x01] = ((rx & 0x03FC) >> 2);
        controllerData[0x02] = ((ly & 0x03FC) >> 2);
        controllerData[0x03] = ((ry & 0x03FC) >> 2);
        controllerData[0x04] = ((lx & 0x03) << 0) | ((rx & 0x03) << 2) | ((ly & 0x03) << 4) | ((ry & 0x03) << 6);
        controllerData[0x05] = lt;
        controllerData[0x06] = rt;
        controllerData[0x07] = (1 |
            ((!(buttons[WiiButtons::WII_BUTTON_R] & 0x01)     << 1)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_PLUS] & 0x01)  << 2)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_HOME] & 0x01)  << 3)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_MINUS] & 0x01) << 4)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_L] & 0x01)     << 5)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_DOWN] & 0x01)  << 6)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_RIGHT] & 0x01) << 7))
        );
        controllerData[0x08] = (
            ((!(buttons[WiiButtons::WII_BUTTON_UP] & 0x01))   << 0) | 
            ((!(buttons[WiiButtons::WII_BUTTON_LEFT] & 0x01)) << 1) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZR] & 0x01))   << 2) | 
            ((!(buttons[WiiButtons::WII_BUTTON_X] & 0x01))    << 3) | 
            ((!(buttons[WiiButtons::WII_BUTTON_A] & 0x01))    << 4) | 
            ((!(buttons[WiiButtons::WII_BUTTON_Y] & 0x01))    << 5) | 
            ((!(buttons[WiiButtons::WII_BUTTON_B] & 0x01))    << 6) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZL] & 0x01))   << 7)
        );
        return 9;
    } else if (getDataType() == WII_DATA_TYPE_3) {
        controllerData[0x00] = analogState[WiiAnalogs::WII_ANALOG_LEFT_X];
        controllerData[0x01] = analogState[WiiAnalogs::WII_ANALOG_RIGHT_X];
        controllerData[0x02] = analogState[WiiAnalogs::WII_ANALOG_LEFT_Y];
        controllerData[0x03] = analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y];
        controllerData[0x04] = analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER];
        controllerData[0x05] = analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER];
        controllerData[0x06] = (1 |
            ((!(buttons[WiiButtons::WII_BUTTON_R] & 0x01)     << 1)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_PLUS] & 0x01)  << 2)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_HOME] & 0x01)  << 3)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_MINUS] & 0x01) << 4)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_L] & 0x01)     << 5)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_DOWN] & 0x01)  << 6)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_RIGHT] & 0x01) << 7))
        );
        controllerData[0x07] = (
            ((!(buttons[WiiButtons::WII_BUTTON_UP] & 0x01))   << 0) | 
            ((!(buttons[WiiButtons::WII_BUTTON_LEFT] & 0x01)) << 1) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZR] & 0x01))   << 2) | 
            ((!(buttons[WiiButtons::WII_BUTTON_X] & 0x01))    << 3) | 
            ((!(buttons[WiiButtons::WII_BUTTON_A] & 0x01))    << 4) | 
            ((!(buttons[WiiButtons::WII_BUTTON_Y] & 0x01))    << 5) | 
            ((!(buttons[WiiButtons::WII_BUTTON_B] & 0x01))    << 6) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZL] & 0x01))   << 7)
        );
        return 8;
    } else {
    }

    return 0;
}
