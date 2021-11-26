/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <vector>
#include "pico/util/queue.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "GamepadEnums.h"
#include "Animation.hpp"
#include "pleds.h"
#include "xinput_driver.h"

const int PLED_PINS[] = {PLED1_PIN, PLED2_PIN, PLED3_PIN, PLED4_PIN};
InputMode inputMode;
uint32_t rgbPLEDValues[4];

void setRGBPLEDs(uint32_t *frame)
{
	for (int i = 0; i < PLED_COUNT; i++)
		if (PLED_PINS[i] > -1)
			frame[PLED_PINS[i]] = rgbPLEDValues[i];
}

PLEDAnimationState getXInputAnimation(uint8_t *data)
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

void RGBPlayerLEDs::setup()
{

}

void RGBPlayerLEDs::display()
{
	switch (inputMode)
	{
		case INPUT_MODE_XINPUT:
			for (int i = 0; i < PLED_COUNT; i++) {
				float level = (static_cast<float>(PLED_MAX_LEVEL - ledLevels[i]) / static_cast<float>(PLED_MAX_LEVEL));
				float brightness = as.GetBrightnessX() * level;
				rgbPLEDValues[i] = ((RGB)ColorGreen).value(neopico->GetFormat(), brightness);
			}
			break;
	}
}

void PLEDModule::setup()
{
	queue_init(&featureQueue, PLED_REPORT_SIZE, 20);

	enabled = PLED_TYPE != PLED_TYPE_NONE;
	if (enabled)
	{
		switch (type)
		{
			case PLED_TYPE_PWM:
				pleds = new PWMPlayerLEDs();
				break;
			case PLED_TYPE_RGB:
				pleds = new RGBPlayerLEDs();
				break;
		}

		if (pleds != nullptr)
			pleds->setup();
	}
}

void PLEDModule::loop()
{
	if (pleds != nullptr)
		pleds->display();
}

void PLEDModule::process(Gamepad *gamepad)
{
	static uint8_t featureData[PLED_REPORT_SIZE];

	if (queue_try_remove(&featureQueue, featureData))
	{
		inputMode = gamepad->options.inputMode;
		switch (inputMode)
		{
			case INPUT_MODE_XINPUT:
				animationState = getXInputAnimation(featureData);
				break;
		}

		if (pleds != nullptr && animationState.animation != PLED_ANIM_NONE)
			pleds->animate(animationState);
	}
}
