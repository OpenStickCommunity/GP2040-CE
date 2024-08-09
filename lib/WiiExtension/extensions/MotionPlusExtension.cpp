#include <cstring>
#include "CRC32.h"
#include "ExtensionBase.h"
#include "MotionPlusExtension.h"

#include "WiiExtension.h"

void MotionPlusExtension::init(uint8_t dataType) {
    ExtensionBase::init(dataType);

    yawValue = 0;
    rollValue = 0;
    pitchValue = 0;

    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].origin                           = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_X].destination                      = WII_ANALOG_PRECISION_3;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].origin                           = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_LEFT_Y].destination                      = WII_ANALOG_PRECISION_3;

    // preseed calibration data with max ranges
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].minimum                        = WII_NUNCHUCK_GATE_CENTER-WII_NUNCHUCK_GATE_SIZE;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].center                         = WII_NUNCHUCK_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_X].maximum                        = WII_NUNCHUCK_GATE_CENTER+WII_NUNCHUCK_GATE_SIZE;

    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].minimum                        = WII_NUNCHUCK_GATE_CENTER-WII_NUNCHUCK_GATE_SIZE;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].center                         = WII_NUNCHUCK_GATE_CENTER;
    _analogCalibration[WiiAnalogs::WII_ANALOG_LEFT_Y].maximum                        = WII_NUNCHUCK_GATE_CENTER+WII_NUNCHUCK_GATE_SIZE;

    _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].origin            = WII_ANALOG_PRECISION_2;
    _analogPrecision[WiiAnalogs::WII_ANALOG_CALIBRATION_PRECISION].destination       = WII_ANALOG_PRECISION_3;
}

bool MotionPlusExtension::calibrate(uint8_t *calibrationData) {
    bool result = false;
    // Motion Plus bypasses the base calibration since it does its own
#if WII_EXTENSION_CALIBRATION==true
    uint32_t crc32;
    uint8_t calibrationCheck[28];

    // copy the parts minus their CRC32 bytes for checksum validation
    memcpy(&calibrationCheck[0], &calibrationData[0], 14);
    memcpy(&calibrationCheck[14], &calibrationData[16], 14);

    initialYawValue = 0;
    initialRollValue = 0;
    initialPitchValue = 0;

    calibration = {
        .fastCalibration = {
            .yawZero = (calibrationData[0] << 8) | (calibrationData[1] << 0),
            .rollZero = (calibrationData[2] << 8) | (calibrationData[3] << 0),
            .pitchZero = (calibrationData[4] << 8) | (calibrationData[5] << 0),
            .yawScale = (calibrationData[6] << 8) | (calibrationData[7] << 0),
            .rollScale = (calibrationData[8] << 8) | (calibrationData[9] << 0),
            .pitchScale = (calibrationData[10] << 8) | (calibrationData[11] << 0),
            .degreeScale = calibrationData[12],
        },
        .fastUID = calibrationData[13],
        .crc32MSB = (calibrationData[14] << 8) | (calibrationData[15] << 0),
        .slowCalibration = {
            .yawZero = (calibrationData[16] << 8) | (calibrationData[17] << 0),
            .rollZero = (calibrationData[18] << 8) | (calibrationData[19] << 0),
            .pitchZero = (calibrationData[20] << 8) | (calibrationData[21] << 0),
            .yawScale = (calibrationData[22] << 8) | (calibrationData[23] << 0),
            .rollScale = (calibrationData[24] << 8) | (calibrationData[25] << 0),
            .pitchScale = (calibrationData[26] << 8) | (calibrationData[27] << 0),
            .degreeScale = calibrationData[28],
        },
        .slowUID = calibrationData[29],
        .crc32LSB = (calibrationData[30] << 8) | (calibrationData[31] << 0)
    };

    crc32 = CRC32::calculate(calibrationCheck, 28);
    result = (((calibration.crc32MSB << 16) | calibration.crc32LSB) == crc32);

#if WII_EXTENSION_DEBUG==true
//    for (uint8_t i = 0; i < 32; i++) {
//        printf("%02x ", calibrationData[i]);
//        if (((i+1) % 8) == 0) printf("\n");
//    }
//    printf("Fast (%d):\n", calibration.fastUID);
//    printf("Yaw 0=%d Scale=%d\n", calibration.fastCalibration.yawZero, calibration.fastCalibration.yawScale);
//    printf("Roll 0=%d Scale=%d\n", calibration.fastCalibration.rollZero, calibration.fastCalibration.rollScale);
//    printf("Pitch 0=%d Scale=%d\n", calibration.fastCalibration.pitchZero, calibration.fastCalibration.pitchScale);
//    printf("Degree Scale=%d\n", calibration.fastCalibration.degreeScale);
//    printf("Slow (%d):\n", calibration.slowUID);
//    printf("Yaw 0=%d Scale=%d\n", calibration.slowCalibration.yawZero, calibration.slowCalibration.yawScale);
//    printf("Roll 0=%d Scale=%d\n", calibration.slowCalibration.rollZero, calibration.slowCalibration.rollScale);
//    printf("Pitch 0=%d Scale=%d\n", calibration.slowCalibration.pitchZero, calibration.slowCalibration.pitchScale);
//    printf("Degree Scale=%d\n", calibration.slowCalibration.degreeScale);
//    printf("%08x = %08x\n", (calibration.crc32MSB << 16) | calibration.crc32LSB, crc32);
#endif
#endif
    return result;
}

void MotionPlusExtension::process(uint8_t *inputData) {
    bool readMotionPlusData = ((inputData[5] & 0x02) >> 1);

    extensionConnected = ((inputData[4] & 0x01) >> 0);

    if (readMotionPlusData) {
        skipPostProcess = true;
        yawFastMode = ((inputData[3] & 0x01) >> 0);
        rollFastMode = ((inputData[3] & 0x02) >> 1);
        pitchFastMode = ((inputData[4] & 0x02) >> 1);

        yawValue = inputData[0] | ((inputData[3] >> 2) << 8);
        rollValue = inputData[1] | ((inputData[4] >> 2) << 8);
        pitchValue = inputData[2] | ((inputData[5] >> 2) << 8);

        if (((rollValue > 5000) && (pitchValue > 5000) && (yawValue > 5000) && (rollValue < 0x3FFF) && (pitchValue < 0x3FFF) && (yawValue < 0x3FFF) && !initialRollValue && !initialPitchValue && !initialYawValue)) {
            initialYawValue = yawValue;
            initialRollValue = rollValue;
            initialPitchValue = pitchValue;
        } else {
            motionState[WiiMotions::WII_GYROSCOPE_YAW]   = yawValue - initialYawValue;
            motionState[WiiMotions::WII_GYROSCOPE_ROLL]  = rollValue - initialRollValue;
            motionState[WiiMotions::WII_GYROSCOPE_PITCH] = pitchValue - initialPitchValue;
        }
#if WII_EXTENSION_DEBUG==true
//        printf("\x1B[0;0H");
//        printf("                Roll  Pitch    Yaw\n");
//        printf("Calib       - %6d %6d %6d\n", (rollFastMode ? calibration.fastCalibration.rollZero : calibration.slowCalibration.rollZero), (pitchFastMode ? calibration.fastCalibration.pitchZero : calibration.slowCalibration.pitchZero), (yawFastMode ? calibration.fastCalibration.yawZero : calibration.slowCalibration.yawZero));
//        printf("Calib Scale - %6d %6d %6d\n", (rollFastMode ? calibration.fastCalibration.rollScale : calibration.slowCalibration.rollScale), (pitchFastMode ? calibration.fastCalibration.pitchScale : calibration.slowCalibration.pitchScale), (yawFastMode ? calibration.fastCalibration.yawScale : calibration.slowCalibration.yawScale));
//        printf("Initial     - %6d %6d %6d\n", initialRollValue, initialPitchValue, initialYawValue);
//        printf("Curr        - %6d %6d %6d\n", rollValue, pitchValue, yawValue);
//        printf("Output      - %6d %6d %6d", motionState[WiiMotions::WII_GYROSCOPE_ROLL], motionState[WiiMotions::WII_GYROSCOPE_PITCH], motionState[WiiMotions::WII_GYROSCOPE_YAW]);
//        printf("\x1B[0J");
#endif
    } else {
        if (extensionConnected) {
            skipPostProcess = false;
#if WII_EXTENSION_DEBUG==true
//            printf("MotionPlusExtension::process\n");
//            for (uint8_t i = 0; i < 16; i++) {
//                printf("%02x ", inputData[i]);
//                if (((i+1) % 8) == 0) printf("\n");
//            }
            //printf("inputData[5] "BYTE_TO_BINARY_PATTERN" \n", BYTE_TO_BINARY(inputData[5]));
#endif
            if (getDataType() == WII_DATA_TYPE_4) {
                // no extension attached
            } else if (getDataType() == WII_DATA_TYPE_5) {
                // nunchuck attached
                analogState[WiiAnalogs::WII_ANALOG_LEFT_X] = (inputData[0] & 0xFF);
                analogState[WiiAnalogs::WII_ANALOG_LEFT_Y] = (inputData[1] & 0xFF);

                buttons[WiiButtons::WII_BUTTON_Z]          = (!((inputData[5] & 0x04) >> 2));
                buttons[WiiButtons::WII_BUTTON_C]          = (!((inputData[5] & 0x08) >> 3));

                //printf("X: %d, Y: %d\n", analogState[WiiAnalogs::WII_ANALOG_LEFT_X], analogState[WiiAnalogs::WII_ANALOG_LEFT_Y]);

                motionState[WiiMotions::WII_ACCELEROMETER_X]      = (((inputData[2] << 2) | ((inputData[5] >> 4) & 0x01)));
                motionState[WiiMotions::WII_ACCELEROMETER_Y]      = (((inputData[3] << 2) | ((inputData[5] >> 5) & 0x01)));
                motionState[WiiMotions::WII_ACCELEROMETER_Z]      = ((((inputData[4] & 0xFE) << 3) | ((inputData[5] >> 6) & 0x03)));
            } else if (getDataType() == WII_DATA_TYPE_7) {
                // classic/other attached
            }
        }
    }

#if WII_EXTENSION_DEBUG==true
//    printf("Roll: %5d Pitch: %5d Yaw: %5d\n", motionState[WiiMotions::WII_GYROSCOPE_ROLL], motionState[WiiMotions::WII_GYROSCOPE_PITCH], motionState[WiiMotions::WII_GYROSCOPE_YAW]);
#endif
}