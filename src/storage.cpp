/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "BoardConfig.h"
#include <GamepadStorage.h>
#include "AnimationStorage.hpp"
#include "AnimationStation/src/Effects/StaticColor.hpp"
#include "FlashPROM.h"
#include "Animation.hpp"
#include "storage.h"
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
	GamepadOptions options;
	EEPROM.get(GAMEPAD_STORAGE_INDEX, options);
	if (!options.isSet)
	{
		options.inputMode = InputMode::INPUT_MODE_XINPUT;
		options.dpadMode = DpadMode::DPAD_MODE_DIGITAL;
#ifdef DEFAULT_SOCD_MODE
		options.socdMode = DEFAULT_SOCD_MODE;
#else
		options.socdMode = SOCD_MODE_NEUTRAL;
#endif
	}

	return options;
}

void GamepadStorage::setGamepadOptions(GamepadOptions options)
{
	options.isSet = true;
	EEPROM.set(GAMEPAD_STORAGE_INDEX, options);
}

/* Animation stuffs */

AnimationOptions AnimationStorage::getAnimationOptions()
{
	AnimationOptions options;
	EEPROM.get(ANIMATION_STORAGE_INDEX, options);
	return options;
}

void AnimationStorage::setAnimationOptions(AnimationOptions options)
{
	EEPROM.set(ANIMATION_STORAGE_INDEX, options);
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
