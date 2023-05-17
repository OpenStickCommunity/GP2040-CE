/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// Pico Includes
#include <vector>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "GamepadEnums.h"
#include "xinput_driver.h"

// GP2040 Includes
#include "addons/pleds.h"
#include "helper.h"
#include "storagemanager.h"

// TODO: make this a helper function
// Animation Helper for Player LEDs
PLEDAnimationState getXInputAnimationPWM(uint8_t *data)
{
	PLEDAnimationState animationState =
	{
		.state = 0,
		.animation = PLED_ANIM_NONE,
		.speed = PLED_SPEED_OFF,
	};

	// Check first byte for LED payload
	if (data[0] == 0x01)
	{
		switch (data[2])
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
	}

	return animationState;
}

bool PlayerLEDAddon::available() {
	return Storage::getInstance().getLEDOptions().pledType != PLED_TYPE_NONE;
}

void PlayerLEDAddon::setup() {
	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
	switch (ledOptions.pledType)
	{
		case PLED_TYPE_PWM:
			pwmLEDs = new PWMPlayerLEDs();
			break;
		case PLED_TYPE_RGB:
			// Do not assign pwmLEDs (support later on?)
			break;
	}

	if (pwmLEDs != nullptr)
		pwmLEDs->setup();
}

void PlayerLEDAddon::process()
{
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();

	// Player LEDs can be PWM or driven by NeoPixel
	uint8_t * featureData = Storage::getInstance().GetFeatureData();
	if (ledOptions.pledType == PLED_TYPE_PWM) { // only process the feature queue if we're on PWM
		if (pwmLEDs != nullptr)
			pwmLEDs->display();

		switch (gamepad->options.inputMode)
		{
			case INPUT_MODE_XINPUT:
				animationState = getXInputAnimationPWM(featureData);
				break;
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

	auto pledPins = Storage::getInstance().getPLEDPins();
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
	auto pledPins = Storage::getInstance().getPLEDPins();
	for (int i = 0; i < PLED_COUNT; i++)
		if (pledPins[i] > -1)
			pwm_set_gpio_level(pledPins[i], ledLevels[i]);
}

