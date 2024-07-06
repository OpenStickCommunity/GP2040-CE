#ifndef RUMBLE_H_
#define RUMBLE_H_

#include <vector>
#include <string>
#include "gpaddon.h"
#include "gamepad/GamepadState.h"

#ifndef RUMBLE_ENABLED
#define RUMBLE_ENABLED 1
#endif

#ifndef LEFT_MOTOR_PIN
#define LEFT_MOTOR_PIN 26
#endif

#ifndef RIGHT_MOTOR_PIN
#define RIGHT_MOTOR_PIN 27
#endif

#ifndef MOTOR_SLEEP_PIN
#define MOTOR_SLEEP_PIN 22
#endif

#ifndef PWM_FREQUENCY
#define PWM_FREQUENCY 10000 // 10 kHz
#endif

#ifndef RUMBLE_DUTY_MIN
#define RUMBLE_DUTY_MIN 20.0f
#endif

#ifndef RUMBLE_DUTY_MAX
#define RUMBLE_DUTY_MAX 100.0f
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
	float rumbleDutyMin;
	float rumbleDutyMax;
	uint32_t sysClock;
	GamepadRumbleState currentRumbleState;
};

#endif
