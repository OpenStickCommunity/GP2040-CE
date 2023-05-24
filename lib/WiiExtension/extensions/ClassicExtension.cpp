#include "ExtensionBase.h"
#include "ClassicExtension.h"

#include "WiiExtension.h"

void ClassicExtension::init(uint8_t dataType) {
    ExtensionBase::init(dataType);
    if (getDataType() == WII_DATA_TYPE_1) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].origin                     = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].origin                    = WII_ANALOG_PRECISION_0;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].origin       = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].destination  = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].origin      = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].destination = WII_ANALOG_PRECISION_3;
    } else if (getDataType() == WII_DATA_TYPE_2) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].origin                     = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].origin                    = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].origin       = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].destination  = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].origin      = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].destination = WII_ANALOG_PRECISION_3;
    } else if (getDataType() == WII_DATA_TYPE_3) {
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].origin                          = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_X].destination                     = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].origin                          = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_RIGHT_Y].destination                     = WII_ANALOG_PRECISION_3;

        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].origin                     = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT].destination                = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].origin                    = WII_ANALOG_PRECISION_2;
        _analogPrecision[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT].destination               = WII_ANALOG_PRECISION_2;

        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].origin       = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_LEFT].destination  = WII_ANALOG_PRECISION_3;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].origin      = WII_ANALOG_PRECISION_1;
        _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION_RIGHT].destination = WII_ANALOG_PRECISION_3;
    }

#if WII_EXTENSION_CALIBRATION==false
    // preseed calibration data with max ranges
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum   = 7;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center    = 31;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum   = 57;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum   = 7;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center    = 32;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum   = 57;
    
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].minimum   = 4;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].center    = 16;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].maximum   = 29;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].minimum   = 2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].center    = 15;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].maximum   = 27;
#endif
}

void ClassicExtension::calibrate(uint8_t *calibrationData) {
#if WII_EXTENSION_CALIBRATION==true
    if (getDataType() == WII_DATA_TYPE_1) {
        _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision1To   = WII_ANALOG_PRECISION_1;
        _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision2To   = WII_ANALOG_PRECISION_0;
    } else if (getDataType() == WII_DATA_TYPE_2) {
        _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision1To   = WII_ANALOG_PRECISION_3;
        _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision2To   = WII_ANALOG_PRECISION_3;
    } else if (getDataType() == WII_DATA_TYPE_3) {
        _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision1To   = WII_ANALOG_PRECISION_2;
        _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
        _calibrationPrecision2To   = WII_ANALOG_PRECISION_2;
    }

    _maxX1 = map(idRead[0],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    _minX1 = map(idRead[1],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    _cenX1 = map(idRead[2],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    
    _maxY1 = map(idRead[3],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    _minY1 = map(idRead[4],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    _cenY1 = map(idRead[5],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
    
    _maxX2 = map(idRead[6],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
    _minX2 = map(idRead[7],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
    _cenX2 = map(idRead[8],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
    
    _maxY2 = map(idRead[9],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
    _minY2 = map(idRead[10],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
    _cenY2 = map(idRead[11],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));

#if WII_EXTENSION_DEBUG==true
//    printf("X1 Min: %d\n", _minX1);
//    printf("X1 Max: %d\n", _maxX1);
//    printf("X1 Center: %d\n", _cenX1);
//    printf("Y1 Min: %d\n", _minY1);
//    printf("Y1 Max: %d\n", _maxY1);
//    printf("Y1 Center: %d\n", _cenY1);
//    printf("X2 Min: %d\n", _minX2);
//    printf("X2 Max: %d\n", _maxX2);
//    printf("X2 Center: %d\n", _cenX2);
//    printf("Y2 Min: %d\n", _minY2);
//    printf("Y2 Max: %d\n", _maxY2);
//    printf("Y2 Center: %d\n", _cenY2);
#endif
#endif
}

void ClassicExtension::process(uint8_t *inputData) {
    // write data format to return
    // see wiki for data types
    if (getDataType() == WII_DATA_TYPE_1) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0x3F);
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[1] & 0x3F);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = ((inputData[0] & 0xC0) >> 3) | ((inputData[1] & 0xC0) >> 5) | ((inputData[2] & 0x80) >> 7);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = (inputData[2] & 0x1F);

        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT]       = (((inputData[2] & 0x60) >> 2) | ((inputData[3] & 0xE0) >> 5));
        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT]      = ((inputData[3] & 0x1F) >> 0);

        directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT] = !((inputData[4] & 0x80) >> 7);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN]  = !((inputData[4] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_L]                      = !((inputData[4] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_MINUS]                  = !((inputData[4] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_HOME]                   = !((inputData[4] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_PLUS]                   = !((inputData[4] & 0x04) >> 2);
        buttons[WiiButtons::WII_BUTTON_R]                      = !((inputData[4] & 0x02) >> 1);

        buttons[WiiButtons::WII_BUTTON_ZL]                     = !((inputData[5] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_B]                      = !((inputData[5] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_Y]                      = !((inputData[5] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_A]                      = !((inputData[5] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_X]                      = !((inputData[5] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_ZR]                     = !((inputData[5] & 0x04) >> 2);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT]  = !((inputData[5] & 0x02) >> 1);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_UP]    = !((inputData[5] & 0x01) >> 0);
    } else if (getDataType() == WII_DATA_TYPE_2) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = ((inputData[0] << 2) | ((inputData[4] & 0x03) >> 0));
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = ((inputData[2] << 2) | ((inputData[4] & 0x30) >> 4));
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = ((inputData[1] << 2) | ((inputData[4] & 0x0C) >> 2));
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = ((inputData[3] << 2) | ((inputData[4] & 0xC0) >> 6));

        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT]       = (inputData[5] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT]      = (inputData[6] & 0xFF);

        directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT] = !((inputData[7] & 0x80) >> 7);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN]  = !((inputData[7] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_L]                      = !((inputData[7] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_MINUS]                  = !((inputData[7] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_HOME]                   = !((inputData[7] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_PLUS]                   = !((inputData[7] & 0x04) >> 2);
        buttons[WiiButtons::WII_BUTTON_R]                      = !((inputData[7] & 0x02) >> 1);

        buttons[WiiButtons::WII_BUTTON_ZL]                     = !((inputData[8] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_B]                      = !((inputData[8] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_Y]                      = !((inputData[8] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_A]                      = !((inputData[8] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_X]                      = !((inputData[8] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_ZR]                     = !((inputData[8] & 0x04) >> 2);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT]  = !((inputData[8] & 0x02) >> 1);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_UP]    = !((inputData[8] & 0x01) >> 0);
    } else if (getDataType() == WII_DATA_TYPE_3) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[2] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = (inputData[1] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = (inputData[3] & 0xFF);

        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT]       = (inputData[4] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT]      = (inputData[5] & 0xFF);

        directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT] = !((inputData[6] & 0x80) >> 7);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN]  = !((inputData[6] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_L]                      = !((inputData[6] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_MINUS]                  = !((inputData[6] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_HOME]                   = !((inputData[6] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_PLUS]                   = !((inputData[6] & 0x04) >> 2);
        buttons[WiiButtons::WII_BUTTON_R]                      = !((inputData[6] & 0x02) >> 1);

        buttons[WiiButtons::WII_BUTTON_ZL]                     = !((inputData[7] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_B]                      = !((inputData[7] & 0x40) >> 6);
        buttons[WiiButtons::WII_BUTTON_Y]                      = !((inputData[7] & 0x20) >> 5);
        buttons[WiiButtons::WII_BUTTON_A]                      = !((inputData[7] & 0x10) >> 4);
        buttons[WiiButtons::WII_BUTTON_X]                      = !((inputData[7] & 0x08) >> 3);
        buttons[WiiButtons::WII_BUTTON_ZR]                     = !((inputData[7] & 0x04) >> 2);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT]  = !((inputData[7] & 0x02) >> 1);
        directionalPad[WiiDirectionalPad::WII_DIRECTION_UP]    = !((inputData[7] & 0x01) >> 0);
    } else {
        // unknown
    }

#if WII_EXTENSION_DEBUG==true
    //if ((_lastRead[0] != inputData[0]) || (_lastRead[1] != inputData[1]) || (_lastRead[2] != inputData[2]) || (_lastRead[3] != inputData[3])) {
    //    printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d\n", joy1X, joy1Y, joy2X, joy2Y);
    //}
    //printf("Joy1 X=%5d Y=%5d  Joy2 X=%5d Y=%5d  U=%1d D=%1d L=%1d R=%1d TL=%5d TR=%5d\n", analogState[WiiAnalogs::ANALOG_LEFT_X], analogState[WiiAnalogs::ANALOG_LEFT_Y], analogState[WiiAnalogs::ANALOG_RIGHT_X], analogState[WiiAnalogs::ANALOG_RIGHT_Y], directionalPad[WiiDirectionalPad::DIRECTION_UP], directionalPad[WiiDirectionalPad::DIRECTION_DOWN], directionalPad[WiiDirectionalPad::DIRECTION_LEFT], directionalPad[WiiDirectionalPad::DIRECTION_RIGHT], analogState[WiiAnalogs::ANALOG_TRIGGER_LEFT], analogState[WiiAnalogs::ANALOG_TRIGGER_RIGHT]);
    //printf("A=%1d B=%1d X=%1d Y=%1d ZL=%1d ZR=%1d LT=%1d RT=%1d -=%1d H=%1d +=%1d\n", buttonA, buttonB, buttonX, buttonY, buttonZL, buttonZR, buttonLT, buttonRT, buttonMinus, buttonHome, buttonPlus);
#endif    
}