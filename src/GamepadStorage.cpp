/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "GamepadStorage.h"
#include "FlashPROM.h"

template <typename T>
static T &get(int index, T &value)
{
	return EEPROM.get(index, value);
}

// Wrapper for "set" call to storage API
template <typename T>
static void set(int index, const T &value)
{
	EEPROM.set(index, value);
}

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
	get(STORAGE_DPAD_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setDpadMode(DpadMode mode)
{
	set(STORAGE_DPAD_MODE_INDEX, mode);
}

InputMode GamepadStorage::getInputMode()
{
	InputMode mode = INPUT_MODE_XINPUT;
	get(STORAGE_INPUT_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setInputMode(InputMode mode)
{
	set(STORAGE_INPUT_MODE_INDEX, mode);
}

SOCDMode GamepadStorage::getSOCDMode()
{
	SOCDMode mode = SOCD_MODE_UP_PRIORITY;
	get(STORAGE_SOCD_MODE_INDEX, mode);
	return mode;
}

void GamepadStorage::setSOCDMode(SOCDMode mode)
{
	set(STORAGE_SOCD_MODE_INDEX, mode);
}
