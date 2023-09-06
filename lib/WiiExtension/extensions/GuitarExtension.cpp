#include "ExtensionBase.h"
#include "GuitarExtension.h"

#include "WiiExtension.h"

void GuitarExtension::init(uint8_t dataType) {
    ExtensionBase::init(dataType);
    _guitarType = WII_GUITAR_UNSET;

    if (getDataType() == WII_DATA_TYPE_1) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].origin                     = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].origin                    = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin            = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination       = WII_ANALOG_PRECISION_3;
    } else if (getDataType() == WII_DATA_TYPE_2) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].origin                     = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].origin                    = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin            = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination       = WII_ANALOG_PRECISION_3;
    } else if (getDataType() == WII_DATA_TYPE_3) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].origin                     = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].origin                    = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin            = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination       = WII_ANALOG_PRECISION_3;
    }

    // preseed calibration data with max ranges
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum          = WII_GUITAR_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center           = WII_GUITAR_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum          = WII_GUITAR_GATE_CENTER+WII_GUITAR_GATE_SIZE;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum          = WII_GUITAR_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center           = WII_GUITAR_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum          = WII_GUITAR_GATE_CENTER+WII_GUITAR_GATE_SIZE;
    
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].minimum         = 15;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].center          = WII_GUITAR_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].maximum         = WII_GUITAR_GATE_CENTER*2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].useOffset       = false;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].minimum         = 0;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].center          = WII_GUITAR_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].maximum         = WII_GUITAR_GATE_CENTER*2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].useOffset       = false;
}

void GuitarExtension::process(uint8_t *inputData) {
    // on first read, check the status of the guitar flag
    if (_guitarType == WII_GUITAR_UNSET) {
        if (((inputData[0] & 0x80) >> 7) == 0) {
            _guitarType = WII_GUITAR_GHWT;
        } else {
            _guitarType = WII_GUITAR_GH3;
        }
        // force the data type to 1 when a World Tour guitar is detected
        if ((_guitarType == WII_GUITAR_GHWT) && (getDataType() != WII_DATA_TYPE_1)) {
            setDataType(WII_DATA_TYPE_1);
            _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_1;
            _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
            _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_1;
            _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
            _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_0;
            _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
            _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_0;
            _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;
        }
    }
    if (_guitarType != WII_GUITAR_UNSET) {
        // as defined works for GH3 guitar
        if (getDataType() == WII_DATA_TYPE_1) {
            analogState[WiiAnalogs::WII_ANALOG_LEFT_X]                  = (inputData[0] & 0x3F);
            analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]                  = (inputData[1] & 0x3F);

            touchBar                                                    = ((_guitarType == WII_GUITAR_GHWT) ? (inputData[2] & 0x1F) : 0);

            whammyBar                                                   = (inputData[3] & 0x1F);
            analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]                 = (inputData[3] & 0x1F);

            buttons[WiiButtons::WII_BUTTON_DOWN]                        = !((inputData[4] & 0x40) >> 6);
            buttons[WiiButtons::WII_BUTTON_MINUS]                       = !((inputData[4] & 0x10) >> 4);
            buttons[WiiButtons::WII_BUTTON_PLUS]                        = !((inputData[4] & 0x04) >> 2);

            buttons[GuitarButtons::GUITAR_ORANGE]                       = !((inputData[5] & 0x80) >> 7);
            buttons[GuitarButtons::GUITAR_RED]                          = !((inputData[5] & 0x40) >> 6);
            buttons[GuitarButtons::GUITAR_BLUE]                         = !((inputData[5] & 0x20) >> 5);
            buttons[GuitarButtons::GUITAR_GREEN]                        = !((inputData[5] & 0x10) >> 4);
            buttons[GuitarButtons::GUITAR_YELLOW]                       = !((inputData[5] & 0x08) >> 3);
            buttons[GuitarButtons::GUITAR_PEDAL]                        = !((inputData[5] & 0x04) >> 2);
            buttons[WiiButtons::WII_BUTTON_UP]                          = !((inputData[5] & 0x01) >> 0);

            isTouched                                                   = (touchBar != WII_GUITAR_TOUCHPAD_NONE);

            // process the touch bar for button states
            // touch only seems to exist in GHWT, and GHWT always reports data type 1 format regardless of setting
            if (isTouched) {
                // touched
                buttons[GuitarButtons::GUITAR_GREEN]                    = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_GREEN,WII_GUITAR_TOUCHPAD_RED));
                buttons[GuitarButtons::GUITAR_RED]                      = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_RED,WII_GUITAR_TOUCHPAD_YELLOW));
                buttons[GuitarButtons::GUITAR_YELLOW]                   = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_YELLOW,WII_GUITAR_TOUCHPAD_BLUE));
                buttons[GuitarButtons::GUITAR_BLUE]                     = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_BLUE,WII_GUITAR_TOUCHPAD_ORANGE));
                buttons[GuitarButtons::GUITAR_ORANGE]                   = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_ORANGE,WII_GUITAR_TOUCHPAD_MAX));
                buttons[WiiButtons::WII_BUTTON_DOWN]                    = isTouched;
            }
        } else if (getDataType() == WII_DATA_TYPE_2) {
            analogState[WiiAnalogs::WII_ANALOG_LEFT_X]                  = ((inputData[0] << 2) | ((inputData[4] & 0x03) >> 0));
            analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]                  = ((inputData[2] << 2) | ((inputData[4] & 0x30) >> 4));

            touchBar                                                    = 0;

            whammyBar                                                   = (inputData[6] & 0xFF);
            analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]                 = (inputData[6] & 0xFF);

            buttons[WiiButtons::WII_BUTTON_DOWN]                        = !((inputData[7] & 0x40) >> 6);
            buttons[WiiButtons::WII_BUTTON_MINUS]                       = !((inputData[7] & 0x10) >> 4);
            buttons[WiiButtons::WII_BUTTON_PLUS]                        = !((inputData[7] & 0x04) >> 2);

            buttons[GuitarButtons::GUITAR_ORANGE]                       = !((inputData[8] & 0x80) >> 7);
            buttons[GuitarButtons::GUITAR_RED]                          = !((inputData[8] & 0x40) >> 6);
            buttons[GuitarButtons::GUITAR_BLUE]                         = !((inputData[8] & 0x20) >> 5);
            buttons[GuitarButtons::GUITAR_GREEN]                        = !((inputData[8] & 0x10) >> 4);
            buttons[GuitarButtons::GUITAR_YELLOW]                       = !((inputData[8] & 0x08) >> 3);
            buttons[GuitarButtons::GUITAR_PEDAL]                        = !((inputData[8] & 0x04) >> 2);
            buttons[WiiButtons::WII_BUTTON_UP]                          = !((inputData[8] & 0x01) >> 0);
        } else if (getDataType() == WII_DATA_TYPE_3) {
            analogState[WiiAnalogs::WII_ANALOG_LEFT_X]                  = (inputData[0] & 0xFF);
            analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]                  = (inputData[2] & 0xFF);

            touchBar                                                    = 0;

            whammyBar                                                   = (inputData[5] & 0xFF);
            analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]                 = (inputData[5] & 0xFF);

            buttons[WiiButtons::WII_BUTTON_DOWN]                        = !((inputData[6] & 0x40) >> 6);
            buttons[WiiButtons::WII_BUTTON_MINUS]                       = !((inputData[6] & 0x10) >> 4);
            buttons[WiiButtons::WII_BUTTON_PLUS]                        = !((inputData[6] & 0x04) >> 2);

            buttons[GuitarButtons::GUITAR_ORANGE]                       = !((inputData[7] & 0x80) >> 7);
            buttons[GuitarButtons::GUITAR_RED]                          = !((inputData[7] & 0x40) >> 6);
            buttons[GuitarButtons::GUITAR_BLUE]                         = !((inputData[7] & 0x20) >> 5);
            buttons[GuitarButtons::GUITAR_GREEN]                        = !((inputData[7] & 0x10) >> 4);
            buttons[GuitarButtons::GUITAR_YELLOW]                       = !((inputData[7] & 0x08) >> 3);
            buttons[GuitarButtons::GUITAR_PEDAL]                        = !((inputData[7] & 0x04) >> 2);
            buttons[WiiButtons::WII_BUTTON_UP]                          = !((inputData[7] & 0x01) >> 0);
        }
    }
#if WII_EXTENSION_DEBUG==true
    //if (_lastRead[0] != regRead[0]) {
    //    printf("GH: %d Byte0 = %d\n", _guitarType, (regRead[0]));
    //}
    //if (_lastRead[1] != regRead[1]) {
    //    printf("GH: %d Byte1 = %d\n", _guitarType, (regRead[1]));
    //}
        //printf("Byte2    " BYTE_TO_BINARY_PATTERN " : " BYTE_TO_BINARY_PATTERN " = %02x\n", BYTE_TO_BINARY(regRead[2]), BYTE_TO_BINARY(regRead[2]^0x1F), regRead[2]);
        //printf("G=%1d R=%1d Y=%1d B=%1d O=%1d Touched=%1d\n", fretGreen, fretRed, fretYellow, fretBlue, fretOrange, isTouched);
    //}
//    for (int i = 0; i < result; ++i) {
//        if ((i != 0) && (i != 1) && (i != 3)) {
//            if (_lastRead[i] != regRead[i]) printf("Byte%2d    " BYTE_TO_BINARY_PATTERN "\n", i, BYTE_TO_BINARY(regRead[i]));
//        }
//    }
//    printf("Joy1 X=%4d Y=%4d  Whammy=%4d  U=%1d D=%1d -=%1d +=%1d\n", joy1X, joy1Y, whammyBar, directionUp, directionDown, buttonMinus, buttonPlus);
//    printf("Whammy=%4d\n", analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]);
//    printf("O=%1d B=%1d Y=%1d R=%1d G=%1d\n", buttons[GuitarButtons::GUITAR_ORANGE], buttons[GuitarButtons::GUITAR_BLUE], buttons[GuitarButtons::GUITAR_YELLOW], buttons[GuitarButtons::GUITAR_RED], buttons[GuitarButtons::GUITAR_GREEN]);
#endif
}