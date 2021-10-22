/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <GamepadStorage.h>
#include "FlashPROM.h"
#include "BoardConfig.h"
#include "Animation.hpp"
#include "NeoPico.hpp"

/* Gamepad stuffs */

void GamepadStorage::start()
{
	EEPROM.start();
}

void GamepadStorage::save()
{
	EEPROM.commit();
}

GamepadOptions GamepadStorage::getGamepadOptions()
{
	GamepadOptions options =
	{
		.inputMode = InputMode::INPUT_MODE_XINPUT,
		.dpadMode = DpadMode::DPAD_MODE_DIGITAL,
		.socdMode = DEFAULT_SOCD_MODE,
	};

	EEPROM.get(0, options);
	return options;
}

void GamepadStorage::setGamepadOptions(GamepadOptions options)
{
	EEPROM.set(0, options);
}

/* Animation stuffs */

#ifdef BOARD_LEDS_PIN

#include "leds.h"
#include "AnimationStorage.hpp"
#include "AnimationStation/src/Effects/StaticColor.hpp"

#define STORAGE_LEDS_INDEX (STORAGE_FIRST_AVAILBLE_INDEX)         // 1 byte

AnimationOptions AnimationStorage::getAnimationOptions()
{
	AnimationOptions options;
	EEPROM.get(STORAGE_LEDS_INDEX, options);
	return options;
}

void AnimationStorage::setAnimationOptions(AnimationOptions options)
{
	EEPROM.set(STORAGE_LEDS_INDEX, options);
}

void AnimationStorage::setup(AnimationStation *as)
{
	this->as = as;
#ifdef LED_FORMAT
	Animation::format = LED_FORMAT;
#else
	Animation::format = LED_FORMAT_GRB;
#endif
	AnimationStation::SetOptions(getAnimationOptions());
	AnimationStation::ConfigureBrightness(LED_BRIGHTNESS_MAXIMUM, LED_BRIGHTNESS_STEPS);
	as->SetMode(AnimationStation::options.baseAnimationIndex);
}

void AnimationStorage::save()
{
	bool dirty = false;
	AnimationOptions savedOptions = getAnimationOptions();

	if (memcmp(&savedOptions, &AnimationStation::options, sizeof(AnimationOptions)))
	{
		this->setAnimationOptions(AnimationStation::options);
		dirty = true;
	}

	if (dirty)
		EEPROM.commit();
}
#endif
