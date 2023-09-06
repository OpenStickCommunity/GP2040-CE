#include "ExtensionBase.h"

#include <cstring>
#include <cstdio>

#include "WiiExtension.h"

void ExtensionBase::init(uint8_t dataType) {
    uint8_t i;

    setDataType(dataType);
    isFirstRead = true;
    
    _hasCalibrationData = false;
    for (i = 0; i < WiiButtons::WII_MAX_BUTTONS; ++i) buttons[i] = 0;
    for (i = 0; i < WiiMotions::WII_MAX_MOTIONS; ++i) motionState[i] = 0;
    for (i = 0; i < WiiAnalogs::WII_MAX_ANALOGS; ++i) {
        analogState[i] = 0;
        initialAnalogState[i] = 0;
        _analogCalibration[i].minimum = 0;
        _analogCalibration[i].center = 0;
        _analogCalibration[i].maximum = 1;
    }
}

bool ExtensionBase::calibrate(uint8_t *calibrationData) {
    // stores calibration data. no calculation is done here.
    uint8_t checksum = WII_CHECKSUM_MAGIC;
    uint8_t i = 0;
    uint8_t checksumBytes[2] = {0x00,0x00};
    bool result = false;

#if WII_EXTENSION_DEBUG==true
    for (int i = 0; i < 16; ++i) {
        printf("%02x", calibrationData[i]);
    }
    printf("\n");
#endif

    for (i = 0; i < (WII_CALIBRATION_SIZE-WII_CALIBRATION_CHECKSUM_SIZE); ++i) {
        checksum += calibrationData[i];
        checksumBytes[0] = checksum;
    }
    checksum += WII_CHECKSUM_MAGIC;
    checksumBytes[1] = checksum;

#if WII_EXTENSION_DEBUG==true
    printf("Checksum: %02x%02x=%02x%02x\n", checksumBytes[0], checksumBytes[1], calibrationData[WII_CALIBRATION_SIZE-2], calibrationData[WII_CALIBRATION_SIZE-1]);
#endif

    // if the checksum passes, the extensions can use the data. if not, it will fall back to the extension defaults
    result = ((checksumBytes[0] == calibrationData[WII_CALIBRATION_SIZE-2]) && (checksumBytes[1] == calibrationData[WII_CALIBRATION_SIZE-1]));

    _hasCalibrationData = result;

    return result;
}

void ExtensionBase::postProcess() {
    uint8_t i;

    uint16_t minVal, maxVal, cenVal, outVal;
    int16_t centerOffset = 0;

    for (i = 0; i < WiiAnalogs::WII_MAX_ANALOGS; ++i) {
        // scale calibration values before using
        if (i != WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION) {
            minVal = map(_analogCalibration[i].minimum, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin-1, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination-1);
            maxVal = map(_analogCalibration[i].maximum, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin-1, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination-1);
            cenVal = map(_analogCalibration[i].center, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin-1, 0, _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination-1);

            if (isFirstRead) {
                // stash the first read as the initial orientation. will reset on hotswap.
                initialAnalogState[i] = map(analogState[i], 0, (_analogPrecision[i].origin-1), 0,(_analogPrecision[i].destination-1));

                if (!_hasCalibrationData) {
                    cenVal = initialAnalogState[i];
                }
            }

            if (_analogCalibration[i].useOffset) {
                centerOffset = cenVal-initialAnalogState[i];
            } else {
                centerOffset = 0;
            }

            outVal = map(analogState[i], 0, (_analogPrecision[i].origin-1), 0,(_analogPrecision[i].destination-1));
            if ((i != WiiAnalogs::WII_ANALOG_LEFT_TRIGGER) && (i != WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER)) {
                outVal = bounds(outVal, minVal, maxVal);
                outVal = map(outVal+centerOffset, minVal+centerOffset, maxVal+centerOffset, 0, (_analogPrecision[i].destination-1));
                outVal = bounds(outVal, minVal, maxVal);
            } else {
                outVal = bounds(outVal, 0, (_analogPrecision[i].destination-1));
            }

            applyCalibration(outVal, minVal, maxVal, cenVal);

#if WII_EXTENSION_DEBUG==true
            if (i == WiiAnalogs::WII_ANALOG_RIGHT_Y) {
                //printf("cur:%5d min=%5d:%5d max=%5d:%5d cen=%5d:%5d out:%5d off:%5d\n", analogState[i], _analogCalibration[i].minimum, minVal, _analogCalibration[i].maximum, maxVal, _analogCalibration[i].center, cenVal, outVal, centerOffset);
            }
#endif

            analogState[i] = outVal;
        }
    }

    if (isFirstRead) isFirstRead = false;

#if WII_EXTENSION_DEBUG==true
    //if ((_lastRead[0] != regRead[0]) || (_lastRead[1] != regRead[1]) || (_lastRead[2] != regRead[2]) || (_lastRead[3] != regRead[3])) {
    //    printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d\n", joy1X, joy1Y, joy2X, joy2Y);
    //}
    //printf("C Joy1 X=%5d Y=%5d  Joy2 X=%5d Y=%5d  U=%1d D=%1d L=%1d R=%1d TL=%5d TR=%4d\n", analogState[WiiAnalogs::WII_ANALOG_LEFT_X], analogState[WiiAnalogs::WII_ANALOG_LEFT_Y], analogState[WiiAnalogs::WII_ANALOG_RIGHT_X], analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y], directionalPad[WiiDirectionalPad::WII_DIRECTION_UP], directionalPad[WiiDirectionalPad::WII_DIRECTION_DOWN], directionalPad[WiiDirectionalPad::WII_DIRECTION_LEFT], directionalPad[WiiDirectionalPad::WII_DIRECTION_RIGHT], analogState[WiiAnalogs::WII_ANALOG_TRIGGER_LEFT], analogState[WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT]);
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

uint16_t ExtensionBase::bounds(uint16_t x, uint16_t out_min, uint16_t out_max) {
    if (x > out_max) x = out_max;
    if (x < out_min) x = out_min;
    return x;
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