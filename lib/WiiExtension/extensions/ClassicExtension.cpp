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
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum          = WII_CLASSIC_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center           = WII_CLASSIC_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum          = WII_CLASSIC_GATE_CENTER+WII_CLASSIC_GATE_SIZE;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum          = WII_CLASSIC_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center           = WII_CLASSIC_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum          = WII_CLASSIC_GATE_CENTER+WII_CLASSIC_GATE_SIZE;
    
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].minimum         = WII_CLASSIC_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].center          = WII_CLASSIC_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].maximum         = WII_CLASSIC_GATE_CENTER+WII_CLASSIC_GATE_SIZE;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].minimum         = WII_CLASSIC_ANALOG_GAP;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].center          = WII_CLASSIC_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].maximum         = WII_CLASSIC_GATE_CENTER+WII_CLASSIC_GATE_SIZE;
    
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].minimum    = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].center     = WII_CLASSIC_TRIGGER_MAX/2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].maximum    = WII_CLASSIC_TRIGGER_MAX;

    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].minimum   = 1;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].center    = WII_CLASSIC_TRIGGER_MAX/2;
    _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].maximum   = WII_CLASSIC_TRIGGER_MAX;
}

bool ClassicExtension::calibrate(uint8_t *calibrationData) {
#if WII_EXTENSION_CALIBRATION==true
    if (ExtensionBase::calibrate(calibrationData)) {
        // calibration passed checksum
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum   = calibrationData[0];
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum   = calibrationData[1];
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center    = calibrationData[2];

        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum   = calibrationData[3];
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum   = calibrationData[4];
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center    = calibrationData[5];
        
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].maximum   = calibrationData[6];
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].minimum   = calibrationData[7];
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_X].center    = calibrationData[8];

        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].maximum   = calibrationData[9];
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].minimum   = calibrationData[10];
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_Y].center    = calibrationData[11];
        
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].maximum   = calibrationData[12];
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].minimum   = 1;
        _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER].center    = calibrationData[12]/2;

        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].maximum   = calibrationData[13];
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].minimum   = 1;
        _analogCalibration[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER].center    = calibrationData[12]/2;
        return true;
    } else {
    }
#endif
    return false;
}

void ClassicExtension::process(uint8_t *inputData) {
    // write data format to return
    // see wiki for data types
    if (getDataType() == WII_DATA_TYPE_1) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0x3F);
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[1] & 0x3F);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = ((inputData[0] & 0xC0) >> 3) | ((inputData[1] & 0xC0) >> 5) | ((inputData[2] & 0x80) >> 7);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = (inputData[2] & 0x1F);

        analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER]       = (((inputData[2] & 0x60) >> 2) | ((inputData[3] & 0xE0) >> 5));
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER]      = ((inputData[3] & 0x1F) >> 0);

        buttons[WiiButtons::WII_BUTTON_RIGHT]                  = !((inputData[4] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_DOWN]                   = !((inputData[4] & 0x40) >> 6);
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
        buttons[WiiButtons::WII_BUTTON_LEFT]                   = !((inputData[5] & 0x02) >> 1);
        buttons[WiiButtons::WII_BUTTON_UP]                     = !((inputData[5] & 0x01) >> 0);
    } else if (getDataType() == WII_DATA_TYPE_2) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = ((inputData[0] << 2) | ((inputData[4] & 0x03) >> 0));
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = ((inputData[2] << 2) | ((inputData[4] & 0x30) >> 4));
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = ((inputData[1] << 2) | ((inputData[4] & 0x0C) >> 2));
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = ((inputData[3] << 2) | ((inputData[4] & 0xC0) >> 6));

        analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER]       = (inputData[5] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER]      = (inputData[6] & 0xFF);

        buttons[WiiButtons::WII_BUTTON_RIGHT]                  = !((inputData[7] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_DOWN]                   = !((inputData[7] & 0x40) >> 6);
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
        buttons[WiiButtons::WII_BUTTON_LEFT]                   = !((inputData[8] & 0x02) >> 1);
        buttons[WiiButtons::WII_BUTTON_UP]                     = !((inputData[8] & 0x01) >> 0);
    } else if (getDataType() == WII_DATA_TYPE_3) {
        analogState[WiiAnalogs::WII_ANALOG_LEFT_X]             = (inputData[0] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]             = (inputData[2] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_X]            = (inputData[1] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y]            = (inputData[3] & 0xFF);

        analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER]       = (inputData[4] & 0xFF);
        analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER]      = (inputData[5] & 0xFF);

        buttons[WiiButtons::WII_BUTTON_RIGHT]                  = !((inputData[6] & 0x80) >> 7);
        buttons[WiiButtons::WII_BUTTON_DOWN]                   = !((inputData[6] & 0x40) >> 6);
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
        buttons[WiiButtons::WII_BUTTON_LEFT]                   = !((inputData[7] & 0x02) >> 1);
        buttons[WiiButtons::WII_BUTTON_UP]                     = !((inputData[7] & 0x01) >> 0);
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

uint8_t ClassicExtension::prepareOutput() {
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

        controllerData[0x00] = ((rx & 0x18) << 3) | (lx & 0x3F);
        controllerData[0x01] = ((rx & 0x06) << 5) | (ly & 0x3F);
        controllerData[0x02] = ((rx & 0x01) << 7) | ((lt & 0x0C) << 5) | (ry & 0x1F);
        controllerData[0x03] = ((lt & 0x07) << 5) | (rt & 0x0F);
        controllerData[0x04] = (1 |
            ((!(buttons[WiiButtons::WII_BUTTON_R] & 0x01)     << 1)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_PLUS] & 0x01)  << 2)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_HOME] & 0x01)  << 3)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_MINUS] & 0x01) << 4)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_L] & 0x01)     << 5)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_DOWN] & 0x01)  << 6)) | 
            ((!(buttons[WiiButtons::WII_BUTTON_RIGHT] & 0x01) << 7))
        );
        controllerData[0x05] = (
            ((!(buttons[WiiButtons::WII_BUTTON_UP] & 0x01))   << 0) | 
            ((!(buttons[WiiButtons::WII_BUTTON_LEFT] & 0x01)) << 1) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZR] & 0x01))   << 2) | 
            ((!(buttons[WiiButtons::WII_BUTTON_X] & 0x01))    << 3) | 
            ((!(buttons[WiiButtons::WII_BUTTON_A] & 0x01))    << 4) | 
            ((!(buttons[WiiButtons::WII_BUTTON_Y] & 0x01))    << 5) | 
            ((!(buttons[WiiButtons::WII_BUTTON_B] & 0x01))    << 6) | 
            ((!(buttons[WiiButtons::WII_BUTTON_ZL] & 0x01))   << 7)
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
