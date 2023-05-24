#include "ExtensionBase.h"

#include <cstring>
#include <cstdio>

#include "WiiExtension.h"

void ExtensionBase::init(uint8_t dataType) {
    uint8_t i;

    setDataType(dataType);
    
    for (i = 0; i < WiiDirectionalPad::WII_MAX_DIRECTIONS; ++i) directionalPad[i] = 0;
    for (i = 0; i < WiiButtons::WII_MAX_BUTTONS; ++i) buttons[i] = 0;
    for (i = 0; i < WiiMotions::WII_MAX_MOTIONS; ++i) motionState[i] = 0;
    for (i = 0; i < WiiAnalogs::WII_MAX_ANALOGS; ++i) {
        analogState[i] = 0;
        _analogCalibration[i].minimum = 0;
        _analogCalibration[i].center = 0;
        _analogCalibration[i].maximum = 0;
    }
}

void ExtensionBase::calibrate(uint8_t *calibrationData) {
}

void ExtensionBase::postProcess() {
    uint8_t i;

    for (i = 0; i < WiiAnalogs::WII_MAX_ANALOGS; ++i) {
        analogState[i] = map(
            applyCalibration(analogState[i], _analogCalibration[i].minimum, _analogCalibration[i].maximum, _analogCalibration[i].center),
            0+_analogCalibration[i].minimum,
            (_analogPrecision[i].origin-_analogCalibration[i].maximum),
            0,
            (_analogPrecision[i].destination-1)
        );
    }

#if WII_EXTENSION_DEBUG==true
    //if ((_lastRead[0] != regRead[0]) || (_lastRead[1] != regRead[1]) || (_lastRead[2] != regRead[2]) || (_lastRead[3] != regRead[3])) {
    //    printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d\n", joy1X, joy1Y, joy2X, joy2Y);
    //}
    //printf("C Joy1 X=%5d Y=%5d  Joy2 X=%5d Y=%5d  U=%1d D=%1d L=%1d R=%1d TL=%5d TR=%4d\n", analogState[WiiAnalogs::ANALOG_LEFT_X], analogState[WiiAnalogs::ANALOG_LEFT_Y], analogState[WiiAnalogs::ANALOG_RIGHT_X], analogState[WiiAnalogs::ANALOG_RIGHT_Y], directionalPad[WiiDirectionalPad::DIRECTION_UP], directionalPad[WiiDirectionalPad::DIRECTION_DOWN], directionalPad[WiiDirectionalPad::DIRECTION_LEFT], directionalPad[WiiDirectionalPad::DIRECTION_RIGHT], analogState[WiiAnalogs::ANALOG_TRIGGER_LEFT], analogState[WiiAnalogs::ANALOG_TRIGGER_RIGHT]);
    //printf("A=%1d B=%1d X=%1d Y=%1d ZL=%1d ZR=%1d LT=%1d RT=%1d -=%1d H=%1d +=%1d\n", buttonA, buttonB, buttonX, buttonY, buttonZL, buttonZR, buttonLT, buttonRT, buttonMinus, buttonHome, buttonPlus);
#endif
}

void ExtensionBase::setDataType(uint8_t dataType) { 
    _dataType = dataType;
}

void ExtensionBase::setExtensionType(uint8_t extensionType) { 
    _extensionType = extensionType;
}

uint16_t ExtensionBase::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t ExtensionBase::applyCalibration(uint16_t pos, uint16_t min, uint16_t max, uint16_t cen) {
    uint16_t result;

#if WII_EXTENSION_CALIBRATION==true
    if (pos >= min && pos <= max) {
        result = pos;
    } else {
        if (pos < min) {
            result = min;
        } else if (pos > max) {
            result = max;
        } else {
            result = cen;
        }
    }
#else
    result = pos;
#endif

    return result;
}