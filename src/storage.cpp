/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define STORAGE_INPUT_MODE_INDEX 0               // 1 byte
#define STORAGE_DPAD_MODE_INDEX 1                // 1 byte
#define STORAGE_SOCD_MODE_INDEX 2                // 1 byte
#define STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX 3 // 1 byte
#define STORAGE_LEDS_BRIGHTNESS_INDEX 4          // 1 byte

#include "pico/mutex.h"
#include "pico/time.h"

#include "FlashPROM.h"
#include "GamepadStorage.h"
#include "AnimationStorage.hpp"

#include "LEDConfig.h"

GamepadStorage::GamepadStorage()
{
	EEPROM.start();
}

void GamepadStorage::save()
{
	EEPROM.commit();
}

DpadMode GamepadStorage::getDpadMode()
{
	DpadMode mode = DPAD_MODE_DIGITAL;
	EEPROM.get(STORAGE_DPAD_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setDpadMode(DpadMode mode)
{
	EEPROM.set(STORAGE_DPAD_MODE_INDEX, mode);
}

InputMode GamepadStorage::getInputMode()
{
	InputMode mode = INPUT_MODE_XINPUT;
	EEPROM.get(STORAGE_INPUT_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setInputMode(InputMode mode)
{
	EEPROM.set(STORAGE_INPUT_MODE_INDEX, mode);
}

SOCDMode GamepadStorage::getSOCDMode()
{
	SOCDMode mode = SOCD_MODE_UP_PRIORITY;
	EEPROM.get(STORAGE_SOCD_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setSOCDMode(SOCDMode mode)
{
	EEPROM.set(STORAGE_SOCD_MODE_INDEX, mode);
}

AnimationMode AnimationStorage::getBaseAnimation()
{
	AnimationMode mode = RAINBOW;
	EEPROM.get(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, mode);
	return mode;
}

void AnimationStorage::setBaseAnimation(AnimationMode mode)
{
	EEPROM.set(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, mode);
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

void AnimationStorage::setup()
{
	AnimationStation::SetBrightness(this->getBrightness());
	Animation::SetDefaultPixels(LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL);
	StaticColor::SetDefaultColor(LEDS_STATIC_COLOR_COLOR);
}

/* We don't want to couple our event calls directly to what AS is doing. That means we need to
	let it handle its business, and then afterwards save any changes we find. */
void AnimationStorage::save(AnimationStation as)
{
	bool dirty = false;

	uint8_t brightness = as.GetBrightness();
	if (brightness != getBrightness())
	{
		setBrightness(brightness);
		dirty = true;
	}

	if (as.animations.size() > 0 && as.animations.at(0)->mode != getBaseAnimation())
	{
		setBaseAnimation(as.animations.at(0)->mode);
		dirty = true;
	}

	if (dirty)
		EEPROM.commit();
}
