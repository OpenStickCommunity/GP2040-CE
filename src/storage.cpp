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
#include "CRC32.h"
#include "display.h"
#include "storage.h"
#include "leds.h"

/* Board stuffs */

BoardOptions getBoardOptions()
{
	BoardOptions options;
	EEPROM.get(BOARD_STORAGE_INDEX, options);

	uint32_t lastCRC = options.checksum;
	options.checksum = 0;
	if (CRC32::calculate(&options) != lastCRC)
	{
		options.hasBoardOptions   = false;
		options.pinDpadUp         = PIN_DPAD_UP;
		options.pinDpadDown       = PIN_DPAD_DOWN;
		options.pinDpadLeft       = PIN_DPAD_LEFT;
		options.pinDpadRight      = PIN_DPAD_RIGHT;
		options.pinButtonB1       = PIN_BUTTON_B1;
		options.pinButtonB2       = PIN_BUTTON_B2;
		options.pinButtonB3       = PIN_BUTTON_B3;
		options.pinButtonB4       = PIN_BUTTON_B4;
		options.pinButtonL1       = PIN_BUTTON_L1;
		options.pinButtonR1       = PIN_BUTTON_R1;
		options.pinButtonL2       = PIN_BUTTON_L2;
		options.pinButtonR2       = PIN_BUTTON_R2;
		options.pinButtonS1       = PIN_BUTTON_S1;
		options.pinButtonS2       = PIN_BUTTON_S2;
		options.pinButtonL3       = PIN_BUTTON_L3;
		options.pinButtonR3       = PIN_BUTTON_R3;
		options.pinButtonA1       = PIN_BUTTON_A1;
		options.pinButtonA2       = PIN_BUTTON_A2;
		options.buttonLayout      = BUTTON_LAYOUT;
		options.i2cSDAPin         = I2C_SDA_PIN;
		options.i2cSCLPin         = I2C_SCL_PIN;
		options.i2cBlock          = (I2C_BLOCK == i2c0) ? 0 : 1;
		options.i2cSpeed          = I2C_SPEED;
		options.hasI2CDisplay     = HAS_I2C_DISPLAY;
		options.displayI2CAddress = DISPLAY_I2C_ADDR;
		options.displaySize       = DISPLAY_SIZE;
		options.displayFlip       = DISPLAY_FLIP;
		options.displayInvert     = DISPLAY_INVERT;
	}

	return options;
}

void setBoardOptions(BoardOptions options)
{
	options.checksum = 0;
	options.checksum = CRC32::calculate(&options);
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

	uint32_t lastCRC = options.checksum;
	options.checksum = 0;
	if (CRC32::calculate(&options) != lastCRC)
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
	options.checksum = 0;
	options.checksum = CRC32::calculate(&options);
	EEPROM.set(GAMEPAD_STORAGE_INDEX, options);
}

/* Animation stuffs */

AnimationOptions AnimationStorage::getAnimationOptions()
{
	AnimationOptions options;
	EEPROM.get(ANIMATION_STORAGE_INDEX, options);

	uint32_t lastCRC = options.checksum;
	options.checksum = 0;
	if (CRC32::calculate(&options) != lastCRC)
	{
		options.baseAnimationIndex = LEDS_BASE_ANIMATION_INDEX;
		options.brightness         = LEDS_BRIGHTNESS;
		options.staticColorIndex   = LEDS_STATIC_COLOR_INDEX;
		options.buttonColorIndex   = LEDS_BUTTON_COLOR_INDEX;
		options.chaseCycleTime     = LEDS_CHASE_CYCLE_TIME;
		options.rainbowCycleTime   = LEDS_RAINBOW_CYCLE_TIME;
		options.themeIndex         = LEDS_THEME_INDEX;

		setAnimationOptions(options);
	}

	return options;
}

void AnimationStorage::setAnimationOptions(AnimationOptions options)
{
	options.checksum = 0;
	options.checksum = CRC32::calculate(&options);
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
