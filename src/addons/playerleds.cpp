/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// Pico Includes
#include <vector>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "GamepadEnums.h"

// GP2040 Includes
#include "addons/pleds.h"
#include "helper.h"
#include "storagemanager.h"
#include "usbdriver.h"

// Move to Proto Enums
typedef enum
{
	XINPUT_PLED_OFF       = 0x00, // All off
	XINPUT_PLED_BLINKALL  = 0x01, // All blinking
	XINPUT_PLED_FLASH1    = 0x02, // 1 flashes, then on
	XINPUT_PLED_FLASH2    = 0x03, // 2 flashes, then on
	XINPUT_PLED_FLASH3    = 0x04, // 3 flashes, then on
	XINPUT_PLED_FLASH4    = 0x05, // 4 flashes, then on
	XINPUT_PLED_ON1       = 0x06, // 1 on
	XINPUT_PLED_ON2       = 0x07, // 2 on
	XINPUT_PLED_ON3       = 0x08, // 3 on
	XINPUT_PLED_ON4       = 0x09, // 4 on
	XINPUT_PLED_ROTATE    = 0x0A, // Rotating (e.g. 1-2-4-3)
	XINPUT_PLED_BLINK     = 0x0B, // Blinking*
	XINPUT_PLED_SLOWBLINK = 0x0C, // Slow blinking*
	XINPUT_PLED_ALTERNATE = 0x0D, // Alternating (e.g. 1+4-2+3), then back to previous*
} XInputPLEDPattern;

// TODO: make this a helper function
// Animation Helper for Player LEDs
PLEDAnimationState getXInputAnimationPWM(uint16_t ledState)
{
	PLEDAnimationState animationState =
	{
		.state = 0,
		.animation = PLED_ANIM_NONE,
		.speed = PLED_SPEED_OFF,
	};

	switch (ledState)
	{
		case XINPUT_PLED_BLINKALL:
		case XINPUT_PLED_ROTATE:
		case XINPUT_PLED_BLINK:
		case XINPUT_PLED_SLOWBLINK:
		case XINPUT_PLED_ALTERNATE:
			animationState.state = (PLED_STATE_LED1 | PLED_STATE_LED2 | PLED_STATE_LED3 | PLED_STATE_LED4);
			animationState.animation = PLED_ANIM_BLINK;
			animationState.speed = PLED_SPEED_FAST;
			break;

		case XINPUT_PLED_FLASH1:
		case XINPUT_PLED_ON1:
			animationState.state = PLED_STATE_LED1;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH2:
		case XINPUT_PLED_ON2:
			animationState.state = PLED_STATE_LED2;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH3:
		case XINPUT_PLED_ON3:
			animationState.state = PLED_STATE_LED3;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH4:
		case XINPUT_PLED_ON4:
			animationState.state = PLED_STATE_LED4;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		default:
			break;
	}

	return animationState;
}

bool PlayerLEDAddon::available() {
	return Storage::getInstance().getLedOptions().pledType != PLED_TYPE_NONE;
}

void PlayerLEDAddon::setup() {
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	turnOffWhenSuspended = ledOptions.turnOffWhenSuspended;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	gamepad->auxState.playerID.enabled = true;
    gamepad->auxState.sensors.statusLight.enabled = true;

	switch (ledOptions.pledType)
	{
		case PLED_TYPE_PWM:
			pwmLEDs = new PWMPlayerLEDs();
			break;
		case PLED_TYPE_RGB:
			// Do not assign pwmLEDs (support later on?)
			break;
		default:
			break;
	}

	if (pwmLEDs != nullptr)
		pwmLEDs->setup();
}

void PlayerLEDAddon::process()
{
	if (turnOffWhenSuspended && get_usb_suspended()) return;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();

	// Player LEDs can be PWM or driven by NeoPixel
	if (ledOptions.pledType == PLED_TYPE_PWM) { // only process the feature queue if we're on PWM
		if (pwmLEDs != nullptr)
			pwmLEDs->display();

		if (gamepad->auxState.playerID.enabled && gamepad->auxState.playerID.active) {
			if (gamepad->getOptions().inputMode == INPUT_MODE_XINPUT) {
				animationState = getXInputAnimationPWM(gamepad->auxState.playerID.ledValue);
			}
		}

		if (pwmLEDs != nullptr && animationState.animation != PLED_ANIM_NONE)
			pwmLEDs->animate(animationState);
	}
}

void PWMPlayerLEDs::setup()
{
	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv(&config, 4.f);

	std::vector<uint> sliceNums;

	LEDOptions & ledOptions = Storage::getInstance().getLedOptions();
	int32_t pledPins[] = { ledOptions.pledPin1, ledOptions.pledPin2, ledOptions.pledPin3, ledOptions.pledPin4 };

	for (int i = 0; i < PLED_COUNT; i++)
	{
		if (pledPins[i] > -1)
		{
			gpio_set_function(pledPins[i], GPIO_FUNC_PWM);
			uint sliceNum = pwm_gpio_to_slice_num(pledPins[i]);
			uint channelNum = pwm_gpio_to_channel(pledPins[i]);
			sliceNums.push_back(sliceNum);
			pwm_set_chan_level(sliceNum, channelNum, PLED_MAX_LEVEL);
		}
	}

	for (auto sliceNum : sliceNums)
		pwm_set_enabled(sliceNum, true);
}

void PWMPlayerLEDs::display()
{
	LEDOptions & ledOptions = Storage::getInstance().getLedOptions();
	int32_t pledPins[] = { ledOptions.pledPin1, ledOptions.pledPin2, ledOptions.pledPin3, ledOptions.pledPin4 };

	for (int i = 0; i < PLED_COUNT; i++)
		if (pledPins[i] > -1)
			pwm_set_gpio_level(pledPins[i], ledLevels[i]);
}

