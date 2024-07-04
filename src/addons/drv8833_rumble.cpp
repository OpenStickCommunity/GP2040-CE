#include "hardware/pwm.h"
#include "addons/drv8833_rumble.h"
#include "storagemanager.h"
#include "peripheralmanager.h"
#include "usbdriver.h"
#include "math.h"
#include "helper.h"
#include "config.pb.h"

#include <stdio.h>
#include "pico/stdlib.h"

bool DRV8833RumbleAddon::available() {
	// TODO: use Storage class
	return RUMBLE_ENABLED && (isValidPin(LEFT_MOTOR_PIN) && isValidPin(RIGHT_MOTOR_PIN));
}

void DRV8833RumbleAddon::setup() {
	// TODO: use Storage class
	leftMotorPin = LEFT_MOTOR_PIN;
	rightMotorPin = RIGHT_MOTOR_PIN;
	motorSleepPin = MOTOR_SLEEP_PIN;
	pwmFrequency = PWM_FREQUENCY;
	// TODO: More robust clock check. Currently just assumes 120 MHz if USB Enabled, 125 MHz otherwise.
	if ( PeripheralManager::getInstance().isUSBEnabled(0) )
		sysClock = 120000000;
	else
		sysClock = 125000000;

	gpio_set_function(leftMotorPin, GPIO_FUNC_PWM);
	gpio_set_function(rightMotorPin, GPIO_FUNC_PWM);
	leftMotorPinSlice = pwm_gpio_to_slice_num (leftMotorPin);
	leftMotorPinChannel = pwm_gpio_to_channel (leftMotorPin);
	rightMotorPinSlice = pwm_gpio_to_slice_num (rightMotorPin);
	rightMotorPinChannel = pwm_gpio_to_channel (rightMotorPin);
	pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, 0);
	pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, 0);
	pwm_set_enabled(leftMotorPinSlice, true);
	pwm_set_enabled(rightMotorPinSlice, true);

	if(isValidPin(motorSleepPin)) {
		gpio_init(motorSleepPin);
		gpio_set_dir(motorSleepPin, GPIO_OUT);
		// turn on sleep mode
		gpio_put(motorSleepPin, false);
	}

	stdio_init_all();
}

bool DRV8833RumbleAddon::compareRumbleState(Gamepad * gamepad) {
	if (currentRumbleState.leftMotor == gamepad->rumbleState.leftMotor && currentRumbleState.rightMotor == gamepad->rumbleState.leftMotor)
		return true;

	return false;

}

void DRV8833RumbleAddon::setRumbleState(Gamepad * gamepad) {
	currentRumbleState.leftMotor = gamepad->rumbleState.leftMotor;
	currentRumbleState.rightMotor = gamepad->rumbleState.rightMotor;
}

void DRV8833RumbleAddon::disableMotors() {
	// if motorSleepPin set and all motors are off, enable motor driver sleep mode
	if (isValidPin(motorSleepPin))
		gpio_put(motorSleepPin, false);

	pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, 0);
	pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, 0);
}

void DRV8833RumbleAddon::enableMotors(Gamepad * gamepad) {
	pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, motorToDuty(gamepad->rumbleState.leftMotor));
	pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, motorToDuty(gamepad->rumbleState.rightMotor));

	// if motorSleepPin set and any motors are on, disable motor driver sleep mode
	if (isValidPin(motorSleepPin))
		gpio_put(motorSleepPin, true);
}

void DRV8833RumbleAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();

	if (!compareRumbleState(gamepad)) {
		setRumbleState(gamepad);
		if (!(gamepad->rumbleState.leftMotor || gamepad->rumbleState.rightMotor)) {
			disableMotors();
			return;
		}
		enableMotors(gamepad);
	}
}

uint32_t DRV8833RumbleAddon::pwmSetFreqDuty(uint slice, uint channel, uint32_t frequency, float duty) {
	uint32_t divider16 = sysClock / frequency / 4096 +
							(sysClock % (frequency * 4096) != 0);
	if (divider16 / 16 == 0)
	divider16 = 16;
	uint32_t wrap = sysClock * 16 / divider16 / frequency - 1;
	pwm_set_clkdiv_int_frac(slice, divider16/16,
										divider16 & 0xF);
	pwm_set_wrap(slice, wrap);
	pwm_set_chan_level(slice, channel, wrap * duty / 100);
	return wrap;
}

