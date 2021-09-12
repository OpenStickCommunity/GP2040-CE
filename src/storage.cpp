/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <GamepadStorage.h>
#include "FlashPROM.h"
#include "AnimationStorage.hpp"
#include "LEDConfig.h"

#define STORAGE_LEDS_BRIGHTNESS_INDEX (STORAGE_FIRST_AVAILBLE_INDEX)              // 1 byte
#define STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX (STORAGE_FIRST_AVAILBLE_INDEX + 1) // 4 bytes

static void getStorageValue(int index, void *data, uint16_t size)
{
	uint8_t buffer[size] = { };
	for (int i = 0; i < size; i++)
		EEPROM.get(index + i, buffer[i]);

	memcpy(data, buffer, size);
}

static void setStorageValue(int index, void *data, uint16_t size)
{
	uint8_t buffer[size] = { };
	memcpy(buffer, data, size);
	for (int i = 0; i < size; i++)
		EEPROM.set(index + i, buffer[i]);
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

void GamepadStorage::get(int index, void *data, uint16_t size)
{
	getStorageValue(index, data, size);
}

void GamepadStorage::set(int index, void *data, uint16_t size)
{
	setStorageValue(index, data, size);
}

/* Animation stuffs */

AnimationMode AnimationStorage::getBaseAnimation()
{
	AnimationMode mode = RAINBOW;
	getStorageValue(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, &mode, sizeof(AnimationMode));
	return mode;
}

void AnimationStorage::setBaseAnimation(AnimationMode mode)
{
	setStorageValue(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, &mode, sizeof(AnimationMode));
}

uint8_t AnimationStorage::getBrightness()
{
	uint8_t brightness = LEDS_BRIGHTNESS;
	getStorageValue(STORAGE_LEDS_BRIGHTNESS_INDEX, &brightness, sizeof(uint8_t));
	return brightness;
}

void AnimationStorage::setBrightness(uint8_t brightness)
{
	setStorageValue(STORAGE_LEDS_BRIGHTNESS_INDEX, &brightness, sizeof(uint8_t));
}

void AnimationStorage::setup()
{
	AnimationStation::SetBrightness(this->getBrightness());
	Animation::SetDefaultPixels(LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL);
	StaticColor::SetDefaultColor(LEDS_STATIC_COLOR_COLOR);
}

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
