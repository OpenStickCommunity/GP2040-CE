/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "BoardConfig.h"
#include "storage.h"
#include <GamepadStorage.h>
#include "FlashPROM.h"
#include "Animation.hpp"
#include "enums.h"
#include "leds.h"

/* Board stuffs */

BoardOptions getBoardOptions()
{
	BoardOptions options;
	EEPROM.get(BOARD_STORAGE_INDEX, options);
	return options;
}

void setBoardOptions(BoardOptions options)
{
	EEPROM.set(BOARD_STORAGE_INDEX, options);
}

/* LED stuffs */

LEDOptions getLEDOptions()
{
	LEDOptions options;
	EEPROM.get(LED_STORAGE_INDEX, options);
	return options;
}

void setLEDOptions(LEDOptions options)
{
	EEPROM.set(LED_STORAGE_INDEX, options);
}

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
#ifdef DEFAULT_SOCD_MODE
		.socdMode = DEFAULT_SOCD_MODE,
#else
		.socdMode = SOCD_MODE_NEUTRAL,
#endif
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
