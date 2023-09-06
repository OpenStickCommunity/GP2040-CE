#include "ExtensionBase.h"
#include "TurntableExtension.h"

#include "WiiExtension.h"

void TurntableExtension::init(uint8_t dataType) {
    ExtensionBase::init(dataType);
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_1;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_1;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_1;
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_1;
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].origin                     = WII_ANALOG_PRECISION_0; // 32
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].destination                = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].origin                    = WII_ANALOG_PRECISION_0; // 32
    _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].destination               = WII_ANALOG_PRECISION_2;

    _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin            = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination       = WII_ANALOG_PRECISION_3;


    // preseed calibration data with max ranges
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum          = WII_TURNTABLE_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center           = WII_TURNTABLE_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum          = WII_TURNTABLE_GATE_CENTER+WII_TURNTABLE_GATE_SIZE;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum          = WII_TURNTABLE_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center           = WII_TURNTABLE_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum          = WII_TURNTABLE_GATE_CENTER+WII_TURNTABLE_GATE_SIZE;
    
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].minimum         = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].center          = WII_TURNTABLE_TRIGGER_MAX*2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].maximum         = 256;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].useOffset       = false;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].minimum         = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].center          = WII_TURNTABLE_TRIGGER_MAX*2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].maximum         = 256;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].useOffset       = false;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].minimum    = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].center     = WII_TURNTABLE_TRIGGER_MAX/2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].maximum    = WII_TURNTABLE_TRIGGER_MAX;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].minimum   = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].center    = WII_TURNTABLE_TRIGGER_MAX/2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].maximum   = WII_TURNTABLE_TRIGGER_MAX;
}

void TurntableExtension::process(uint8_t *inputData) {
    // turntable accepts three data modes, but will only report as type 1, much like GHWT
    analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0x3F);
    analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[1] & 0x3F);

    analogState[TurntableAnalogs::TURNTABLE_LEFT]          = (((inputData[4] & 0x00) << 5) | ((inputData[3] & 0x1F) >> 0));
    analogState[TurntableAnalogs::TURNTABLE_RIGHT]         = (((inputData[0] & 0xC0) >> 3) | ((inputData[1] & 0xC0) >> 5) | ((inputData[2] & 0x80) >> 7));
    analogState[TurntableAnalogs::TURNTABLE_EFFECTS]       = (((inputData[2] & 0x60) >> 2) | ((inputData[3] & 0xE0) >> 5));
    analogState[TurntableAnalogs::TURNTABLE_CROSSFADE]     = ((inputData[2] & 0x1E) >> 1);

    // turntables report 0-5 for CW, 27-32 for CCW (as unsigned)
    analogState[TurntableAnalogs::TURNTABLE_LEFT]          = ((analogState[TurntableAnalogs::TURNTABLE_LEFT] < 15) ? WII_TURNTABLE_MAX_PRECISION-analogState[TurntableAnalogs::TURNTABLE_LEFT] : WII_TURNTABLE_MAX_PRECISION+(WII_TURNTABLE_MAX_PRECISION-analogState[TurntableAnalogs::TURNTABLE_LEFT]));
    analogState[TurntableAnalogs::TURNTABLE_RIGHT]         = ((analogState[TurntableAnalogs::TURNTABLE_RIGHT] < 15) ? WII_TURNTABLE_MAX_PRECISION-analogState[TurntableAnalogs::TURNTABLE_RIGHT] : WII_TURNTABLE_MAX_PRECISION+(WII_TURNTABLE_MAX_PRECISION-analogState[TurntableAnalogs::TURNTABLE_RIGHT]));

    buttons[WiiButtons::WII_BUTTON_MINUS]                  = !((inputData[4] & 0x10) >> 4);
    buttons[WiiButtons::WII_BUTTON_PLUS]                   = !((inputData[4] & 0x04) >> 2);

    buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN]       = !((inputData[5] & 0x20) >> 5);                       
    buttons[TurntableButtons::TURNTABLE_RIGHT_RED]         = !((inputData[4] & 0x02) >> 1);
    buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE]        = !((inputData[5] & 0x04) >> 2);

    buttons[TurntableButtons::TURNTABLE_EUPHORIA]          = !((inputData[5] & 0x10) >> 4);

    buttons[TurntableButtons::TURNTABLE_LEFT_GREEN]        = !((inputData[5] & 0x08) >> 3);
    buttons[TurntableButtons::TURNTABLE_LEFT_RED]          = !((inputData[4] & 0x20) >> 5);
    buttons[TurntableButtons::TURNTABLE_LEFT_BLUE]         = !((inputData[5] & 0x80) >> 7);

#if WII_EXTENSION_DEBUG==true
    //printf("LR=%1d LG=%1d LB=%1d\n", buttons[TurntableButtons::TURNTABLE_LEFT_RED], buttons[TurntableButtons::TURNTABLE_LEFT_GREEN], buttons[TurntableButtons::TURNTABLE_LEFT_BLUE]);
    //printf("RR=%1d RG=%1d RB=%1d\n", buttons[TurntableButtons::TURNTABLE_RIGHT_RED], buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN], buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE]);
    //printf("-=%1d EU=%1d +=%1d\n", buttons[WiiButtons::BUTTON_MINUS], buttons[TurntableButtons::TURNTABLE_EUPHORIA], buttons[WiiButtons::BUTTON_PLUS]);
    //printf("LTT=%4d RTT=%4d CF=%4d ED=%4d\n", analogState[TurntableAnalogs::TURNTABLE_LEFT], analogState[TurntableAnalogs::TURNTABLE_RIGHT], analogState[TurntableAnalogs::TURNTABLE_CROSSFADE], analogState[TurntableAnalogs::TURNTABLE_EFFECTS]);
    //printf("X=%4d Y=%4d\n", analogState[WiiAnalogs::WII_ANALOG_LEFT_X], analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]);

    //for (int i = 0; i < result; ++i) {
    //    if (_lastRead[i] != inputData[i]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", i, BYTE_TO_BINARY(inputData[i]));
    //}
#endif
}