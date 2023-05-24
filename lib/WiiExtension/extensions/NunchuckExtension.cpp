#include "ExtensionBase.h"
#include "NunchuckExtension.h"

#include "WiiExtension.h"

void NunchuckExtension::init(uint8_t dataType) {
    ExtensionBase::init(dataType);
    _extensionType = WII_EXTENSION_NUNCHUCK;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin      = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination = WII_ANALOG_PRECISION_3;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin      = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination = WII_ANALOG_PRECISION_3;

#if WII_EXTENSION_CALIBRATION==false
    // preseed calibration data with max ranges
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum   = 35;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center    = 128;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum   = 228;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum   = 27;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center    = 128;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum   = 220;
#endif
}

void NunchuckExtension::calibrate(uint8_t *calibrationData) {
#if WII_EXTENSION_CALIBRATION==true
//    _maxX1 = idRead[8];
//    _minX1 = idRead[9];
//    _cenX1 = idRead[10];
//
//    _maxY1 = idRead[11];
//    _minY1 = idRead[12];
//    _cenY1 = idRead[13];
//
//    _accelX0G = ((idRead[0] << 2) | ((idRead[3] >> 2) & 0x03));
//    _accelY0G = ((idRead[1] << 2) | ((idRead[3] >> 4) & 0x03));
//    _accelZ0G = ((idRead[2] << 2) | ((idRead[3] >> 6) & 0x03));
//
//    _accelX1G = ((idRead[4] << 2) | ((idRead[7] >> 2) & 0x03));
//    _accelY1G = ((idRead[5] << 2) | ((idRead[7] >> 4) & 0x03));
//    _accelZ1G = ((idRead[6] << 2) | ((idRead[7] >> 6) & 0x03));

#if WII_EXTENSION_DEBUG==true
    //printf("Calibration:\n");
    //printf("X0G: %d\n", _accelX0G);
    //printf("Y0G: %d\n", _accelY0G);
    //printf("Z0G: %d\n", _accelZ0G);
    //printf("X1G: %d\n", _accelX1G);
    //printf("Y1G: %d\n", _accelY1G);
    //printf("YZG: %d\n", _accelZ1G);
    //printf("X Min: %d\n", _minX);
    //printf("X Max: %d\n", _maxX);
    //printf("X Center: %d\n", _cenX);
    //printf("Y Min: %d\n", _minY);
    //printf("Y Max: %d\n", _maxY);
    //printf("Y Center: %d\n", _cenY);
#endif
#endif
}

void NunchuckExtension::process(uint8_t *inputData) {
    analogState[WiiAnalogs::WII_ANALOG_LEFT_X] = (inputData[0] & 0xFF);
    analogState[WiiAnalogs::WII_ANALOG_LEFT_Y] = (inputData[1] & 0xFF);

    buttons[WiiButtons::WII_BUTTON_Z]          = (!(inputData[5] & 0x01));
    buttons[WiiButtons::WII_BUTTON_C]          = (!(inputData[5] & 0x02));

    motionState[WiiMotions::WII_MOTION_X]      = (((inputData[2] << 2) | ((inputData[5] >> 2) & 0x03)));
    motionState[WiiMotions::WII_MOTION_Y]      = (((inputData[3] << 2) | ((inputData[5] >> 4) & 0x03)));
    motionState[WiiMotions::WII_MOTION_Z]      = (((inputData[4] << 2) | ((inputData[5] >> 6) & 0x03)));

#if WII_EXTENSION_DEBUG==true
    printf("Joy X=%4d Y=%4d   Acc X=%4d Y=%4d Z=%4d   Btn Z=%1d C=%1d\n", analogState[WiiAnalogs::ANALOG_LEFT_X], analogState[WiiAnalogs::ANALOG_LEFT_Y], motionState[WiiMotions::MOTION_X], motionState[WiiMotions::MOTION_Y], motionState[WiiMotions::MOTION_Z], buttons[WiiButtons::BUTTON_Z], buttons[WiiButtons::BUTTON_C]);
#endif
}