#ifndef _EXTENSIONBASE_H_
#define _EXTENSIONBASE_H_

#include <cstdint>
#include <cstdio>

typedef enum {
    WII_BUTTON_A,
    WII_BUTTON_B,
    WII_BUTTON_C,
    WII_BUTTON_X,
    WII_BUTTON_Y,
    WII_BUTTON_Z,
    WII_BUTTON_L,
    WII_BUTTON_R,
    WII_BUTTON_ZL,
    WII_BUTTON_ZR,
    WII_BUTTON_MINUS,
    WII_BUTTON_HOME,
    WII_BUTTON_PLUS,
    WII_BUTTON_UP,
    WII_BUTTON_DOWN,
    WII_BUTTON_LEFT,
    WII_BUTTON_RIGHT,
    WII_MAX_BUTTONS
} WiiButtons;

typedef enum {
    WII_ANALOG_LEFT_X,
    WII_ANALOG_LEFT_Y,
    WII_ANALOG_RIGHT_X,
    WII_ANALOG_RIGHT_Y,
    WII_ANALOG_LEFT_TRIGGER,
    WII_ANALOG_RIGHT_TRIGGER,
    WII_ANALOG_CALIBRATION_PRECISION,
    WII_MAX_ANALOGS
} WiiAnalogs;

typedef enum {
    WII_JOYSTICK_NONE,
    WII_JOYSTICK_LEFT_ANALOG,
    WII_JOYSTICK_RIGHT_ANALOG,
    WII_JOYSTICK_DPAD
} WiiJoystickModes;

typedef enum {
    WII_MOTION_X,
    WII_MOTION_Y,
    WII_MOTION_Z,
    WII_MAX_MOTIONS
} WiiMotions;

typedef struct {
    uint16_t origin = 0;
    uint16_t destination = 0;
} WiiAnalogPrecision;

typedef struct {
    uint16_t minimum = 0;
    uint16_t center = 0;
    uint16_t maximum = 0;
    bool useOffset = true;
} WiiAnalogCalibration;

class ExtensionBase {
    protected:
        uint8_t _dataType;
        uint8_t _extensionType;
        bool _hasCalibrationData = false;

        WiiAnalogPrecision _analogPrecision[WiiAnalogs::WII_MAX_ANALOGS];
        WiiAnalogCalibration _analogCalibration[WiiAnalogs::WII_MAX_ANALOGS];
        
        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
        uint16_t bounds(uint16_t x, uint16_t out_min, uint16_t out_max);
    private:
        uint16_t applyCalibration(uint16_t pos, uint16_t min, uint16_t max, uint16_t cen);
    public:
        bool buttons[WiiButtons::WII_MAX_BUTTONS];
        uint16_t analogState[WiiAnalogs::WII_MAX_ANALOGS];
        uint16_t motionState[WiiMotions::WII_MAX_MOTIONS];

        uint16_t initialAnalogState[WiiAnalogs::WII_MAX_ANALOGS];
        bool isFirstRead = true;

        virtual void init(uint8_t dataType);
        virtual bool calibrate(uint8_t *calibrationData);
        virtual void process(uint8_t *inputData) = 0;
        virtual void postProcess();

#if WII_EXTENSION_DEBUG==true
        uint8_t _lastRead[16] = {0xFF};
#endif

        void setDataType(uint8_t dataType);
        uint8_t getDataType() { return _dataType; };

        void setExtensionType(uint8_t extensionType);
        uint8_t getExtensionType() { return _extensionType; };
};

#endif
