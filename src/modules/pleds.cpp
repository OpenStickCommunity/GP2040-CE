/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <vector>
#include "pico/util/queue.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "GamepadEnums.h"
#include "xinput_driver.h"

// Pico Includes
#include "modules/pleds.h"
#include "helper.h"
#include "storage.h"

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

bool PLEDModule::available() {
	return PLED_TYPE != PLED_TYPE_NONE;
}

void PLEDModule::setup() {

	switch (PLED_TYPE)
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

void PLEDModule::loop() {
	// Player LEDs can be PWM or driven by NeoPixel
	if (PLED_TYPE == PLED_TYPE_PWM) {
		if (pwmLEDs != nullptr)
			pwmLEDs->display();
	}
}

void PLEDModule::process(Gamepad *gamepad) {
	static uint8_t featureData[PLED_REPORT_SIZE];
	if (PLED_TYPE == PLED_TYPE_PWM) { // only process the feature queue if we're on PWM
		if (queue_try_remove(Storage::getInstance().GetFeatureQueue(), featureData))
		{
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
}

void PWMPlayerLEDs::setup()
{
	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv(&config, 4.f);

	std::vector<uint> sliceNums;

	for (int i = 0; i < PLED_COUNT; i++)
	{
		if (PLED_PINS[i] > -1)
		{
			gpio_set_function(PLED_PINS[i], GPIO_FUNC_PWM);
			uint sliceNum = pwm_gpio_to_slice_num(PLED_PINS[i]);
			uint channelNum = pwm_gpio_to_channel(PLED_PINS[i]);
			sliceNums.push_back(sliceNum);
			pwm_set_chan_level(sliceNum, channelNum, PLED_MAX_LEVEL);
		}
	}

	for (auto sliceNum : sliceNums)
		pwm_set_enabled(sliceNum, true);
}

void PWMPlayerLEDs::display()
{
	for (int i = 0; i < PLED_COUNT; i++)
		if (PLED_PINS[i] > -1)
			pwm_set_gpio_level(PLED_PINS[i], ledLevels[i]);
}

