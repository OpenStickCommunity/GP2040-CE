#ifndef RUMBLE_H_
#define RUMBLE_H_

#include <vector>
#include <string>
#include "gpaddon.h"
#include "gamepad/GamepadState.h"
#include "gamepad/GamepadAuxState.h"

#ifndef DRV8833_RUMBLE_ENABLED
#define DRV8833_RUMBLE_ENABLED 0
#endif

#ifndef DRV8833_RUMBLE_LEFT_MOTOR_PIN
#define DRV8833_RUMBLE_LEFT_MOTOR_PIN -1 //26
#endif

#ifndef DRV8833_RUMBLE_RIGHT_MOTOR_PIN
#define DRV8833_RUMBLE_RIGHT_MOTOR_PIN -1 //27
#endif

#ifndef DRV8833_RUMBLE_MOTOR_SLEEP_PIN
#define DRV8833_RUMBLE_MOTOR_SLEEP_PIN -1 //22
#endif

#ifndef DRV8833_RUMBLE_PWM_FREQUENCY
#define DRV8833_RUMBLE_PWM_FREQUENCY 10000 // 10 kHz
#endif

#ifndef DRV8833_RUMBLE_DUTY_MIN
#define DRV8833_RUMBLE_DUTY_MIN 0.0f
#endif

#ifndef DRV8833_RUMBLE_DUTY_MAX
#define DRV8833_RUMBLE_DUTY_MAX 100.0f
#endif

// DRV8833 Rumble Module
#define DRV8833RumbleName "DRV8833Rumble"

// Scale uint8 to 0 -> 100 range
#define motorToDuty(m) (100.0f * (m/255.0f))
// Rescale from 0 -> 100 range to min -> max range
#define scaleDuty(in, min, max) ((in/100.0f) * (max-min) + min)

// Buzzer Speaker
class DRV8833RumbleAddon : public GPAddon
{
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return DRV8833RumbleName; }
private:
    uint32_t pwmSetFreqDuty(uint slice, uint channel, uint32_t frequency, float duty);
    bool compareRumbleState(Gamepad * gamepad);
    void setRumbleState(Gamepad * gamepad);
    void disableMotors();
    void enableMotors(Gamepad * gamepad);
    uint8_t leftMotorPin;
    uint8_t rightMotorPin;
    uint8_t motorSleepPin;
    uint8_t leftMotorPinSlice;
    uint8_t leftMotorPinChannel;
    uint8_t rightMotorPinSlice;
    uint8_t rightMotorPinChannel;
    uint32_t pwmFrequency;
    float dutyMin;
    float dutyMax;
    uint32_t sysClock;
    GamepadAuxHaptics currentRumbleState;
};

#endif
