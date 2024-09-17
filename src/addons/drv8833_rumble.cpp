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
	const DRV8833RumbleOptions& options = Storage::getInstance().getAddonOptions().drv8833RumbleOptions;
	return options.enabled && (isValidPin(options.leftMotorPin) && isValidPin(options.rightMotorPin));
}

void DRV8833RumbleAddon::setup() {
	const DRV8833RumbleOptions& options = Storage::getInstance().getAddonOptions().drv8833RumbleOptions;
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	leftMotorPin = options.leftMotorPin;
	rightMotorPin = options.rightMotorPin;
	motorSleepPin = options.motorSleepPin;
	pwmFrequency = options.pwmFrequency;
	dutyMin = options.dutyMin;
	dutyMax = options.dutyMax;

	// TODO: More robust clock check. Currently just assumes 120 MHz if USB Enabled, 125 MHz otherwise.
	if ( PeripheralManager::getInstance().isUSBEnabled(0) )
		sysClock = 120000000;
	else
		sysClock = 125000000;


	// enable haptics in Aux sensors depending on pin assignments
	if(isValidPin(leftMotorPin)) {
		gpio_set_function(leftMotorPin, GPIO_FUNC_PWM);
		leftMotorPinSlice = pwm_gpio_to_slice_num (leftMotorPin);
		leftMotorPinChannel = pwm_gpio_to_channel (leftMotorPin);
		pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, 0);
		pwm_set_enabled(leftMotorPinSlice, true);
		gamepad->auxState.haptics.leftActuator.enabled = true;
	}

	if(isValidPin(rightMotorPin)) {
		gpio_set_function(rightMotorPin, GPIO_FUNC_PWM);
		rightMotorPinSlice = pwm_gpio_to_slice_num (rightMotorPin);
		rightMotorPinChannel = pwm_gpio_to_channel (rightMotorPin);
		pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, 0);
		pwm_set_enabled(rightMotorPinSlice, true);
		gamepad->auxState.haptics.rightActuator.enabled = true;
	}

	if(isValidPin(motorSleepPin)) {
		gpio_init(motorSleepPin);
		gpio_set_dir(motorSleepPin, GPIO_OUT);
		// turn on sleep mode
		gpio_put(motorSleepPin, false);
	}
}

bool DRV8833RumbleAddon::compareRumbleState(Gamepad * gamepad) {
	if (currentRumbleState.leftActuator.active == gamepad->auxState.haptics.leftActuator.active && 
		currentRumbleState.leftActuator.intensity == gamepad->auxState.haptics.leftActuator.intensity && 
		currentRumbleState.rightActuator.active == gamepad->auxState.haptics.rightActuator.active && 
		currentRumbleState.rightActuator.intensity == gamepad->auxState.haptics.rightActuator.intensity)
		return true;

	return false;

}

void DRV8833RumbleAddon::setRumbleState(Gamepad * gamepad) {
	currentRumbleState.leftActuator.active = gamepad->auxState.haptics.leftActuator.active;
	currentRumbleState.leftActuator.intensity = gamepad->auxState.haptics.leftActuator.intensity;
	currentRumbleState.rightActuator.active = gamepad->auxState.haptics.rightActuator.active;
	currentRumbleState.rightActuator.intensity = gamepad->auxState.haptics.rightActuator.intensity;
}

void DRV8833RumbleAddon::disableMotors() {
	// if motorSleepPin set and all motors are off, enable motor driver sleep mode
	if (isValidPin(motorSleepPin))
		gpio_put(motorSleepPin, false);

	pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, 0);
	pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, 0);
}

void DRV8833RumbleAddon::enableMotors(Gamepad * gamepad) {
	pwmSetFreqDuty(leftMotorPinSlice, leftMotorPinChannel, pwmFrequency, (gamepad->auxState.haptics.leftActuator.intensity == 0) ? 0 : scaleDuty(motorToDuty(gamepad->auxState.haptics.leftActuator.intensity), dutyMin, dutyMax));
	pwmSetFreqDuty(rightMotorPinSlice, rightMotorPinChannel, pwmFrequency, (gamepad->auxState.haptics.rightActuator.intensity == 0) ? 0 : scaleDuty(motorToDuty(gamepad->auxState.haptics.rightActuator.intensity), dutyMin, dutyMax));

	// if motorSleepPin set and any motors are on, disable motor driver sleep mode
	if (isValidPin(motorSleepPin))
		gpio_put(motorSleepPin, true);
}

void DRV8833RumbleAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	if (!compareRumbleState(gamepad)) {
		setRumbleState(gamepad);
		if (!(gamepad->auxState.haptics.leftActuator.active || gamepad->auxState.haptics.rightActuator.active)) {
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

