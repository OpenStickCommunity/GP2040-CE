#ifndef _MOTIONPLUSEXTENSION_H_
#define _MOTIONPLUSEXTENSION_H_

#include "ExtensionBase.h"

class MotionPlusExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        bool calibrate(uint8_t *calibrationData) override;
        void process(uint8_t *inputData) override;

    private:
        struct CalibrationBlock {
            uint16_t yawZero = 0;
            uint16_t rollZero = 0;
            uint16_t pitchZero = 0;
            uint16_t yawScale = 1;
            uint16_t rollScale = 1;
            uint16_t pitchScale = 1;
            uint16_t degreeScale = 1;
        } __attribute__((packed));

        struct CalibrationData {
            CalibrationBlock fastCalibration;
            uint8_t fastUID;
            uint16_t crc32MSB;
            CalibrationBlock slowCalibration;
            uint8_t slowUID;
            uint16_t crc32LSB;
        } __attribute__((packed));

        int16_t initialYawValue;
        int16_t initialRollValue;
        int16_t initialPitchValue;
        bool initialRead = true;
        
        int16_t yawValue;
        int16_t rollValue;
        int16_t pitchValue;

        bool yawFastMode = false;
        bool rollFastMode = false;
        bool pitchFastMode = false;
        bool extensionConnected = false;

        CalibrationData calibration;

        uint8_t sampleSize = 10;
};

#endif