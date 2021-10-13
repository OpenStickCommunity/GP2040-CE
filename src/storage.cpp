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

#define STORAGE_LEDS_BRIGHTNESS_INDEX (STORAGE_FIRST_AVAILBLE_INDEX)         // 1 byte
#define STORAGE_LEDS_ANIMATION_MODE_INDEX (STORAGE_FIRST_AVAILBLE_INDEX + 1) // 1 byte

uint8_t AnimationStorage::getMode()
{
	uint8_t mode = 0;
	EEPROM.get(STORAGE_LEDS_ANIMATION_MODE_INDEX, mode);
	return mode;
}

void AnimationStorage::setMode(uint8_t mode)
{
	EEPROM.set(STORAGE_LEDS_ANIMATION_MODE_INDEX, mode);
}

uint8_t AnimationStorage::getBrightness()
{
	uint8_t brightness = LEDS_BRIGHTNESS;
	EEPROM.get(STORAGE_LEDS_BRIGHTNESS_INDEX, brightness);
	return brightness;
}

void AnimationStorage::setBrightness(uint8_t brightness)
{
	EEPROM.set(STORAGE_LEDS_BRIGHTNESS_INDEX, brightness);
}

void AnimationStorage::setup(AnimationStation *as)
{
	this->as = as;
#ifdef LED_FORMAT
	Animation::format = LED_FORMAT;
#else
	Animation::format = LED_FORMAT_GRB;
#endif
	AnimationStation::ConfigureBrightness(LED_BRIGHTNESS_MAXIMUM, LED_BRIGHTNESS_STEPS);
	AnimationStation::SetBrightness(this->getBrightness());
	as->SetMode(getMode());
	configureAnimations(as);
}

void AnimationStorage::save()
{
	bool dirty = false;

	uint8_t brightness = as->GetBrightness();
	if (brightness != getBrightness())
	{
		setBrightness(brightness);
		dirty = true;
	}

	uint8_t mode = as->GetMode();
	if (mode != getMode())
	{
		setMode(mode);
		dirty = true;
	}

	if (dirty)
		EEPROM.commit();
}

#endif
