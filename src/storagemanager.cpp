/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "storagemanager.h"

#include "BoardConfig.h"
#include "AnimationStorage.hpp"
#include "Effects/StaticColor.hpp"
#include "FlashPROM.h"
#include "hardware/watchdog.h"
#include "Animation.hpp"
#include "CRC32.h"

#include "addons/analog.h"
#include "addons/board_led.h"
#include "addons/bootsel_button.h"
#include "addons/buzzerspeaker.h"
#include "addons/dualdirectional.h"
#include "addons/extra_button.h"
#include "addons/i2canalog1219.h"
#include "addons/i2cdisplay.h"
#include "addons/jslider.h"
#include "addons/neopicoleds.h"
#include "addons/playernum.h"
#include "addons/ps4mode.h"
#include "addons/pleds.h"
#include "addons/reverse.h"
#include "addons/turbo.h"
#include "addons/slider_socd.h"
#include "addons/wiiext.h"

#include "config_utils.h"

#include "bitmaps.h"

#include "helper.h"

Storage::Storage()
{
	EEPROM.start();

	initBoardOptions();
	initLEDOptions();

	critical_section_init(&animationOptionsCs);

	ConfigUtils::load(config);
}

bool Storage::save()
{
	return ConfigUtils::save(config);
}

static void updateAnimationOptionsProto(const AnimationOptions& options)
{
	AnimationOptions_Proto& optionsProto = Storage::getInstance().getAnimationOptions();

	optionsProto.baseAnimationIndex			= options.baseAnimationIndex;
	optionsProto.brightness					= options.brightness;
	optionsProto.staticColorIndex			= options.staticColorIndex;
	optionsProto.buttonColorIndex			= options.buttonColorIndex;
	optionsProto.chaseCycleTime				= options.chaseCycleTime;
	optionsProto.rainbowCycleTime			= options.rainbowCycleTime;
	optionsProto.themeIndex					= options.themeIndex;
	optionsProto.hasCustomTheme				= options.hasCustomTheme;
	optionsProto.customThemeUp				= options.customThemeUp;
	optionsProto.customThemeDown			= options.customThemeDown;
	optionsProto.customThemeLeft			= options.customThemeLeft;
	optionsProto.customThemeRight			= options.customThemeRight;
	optionsProto.customThemeB1				= options.customThemeB1;
	optionsProto.customThemeB2				= options.customThemeB2;
	optionsProto.customThemeB3				= options.customThemeB3;
	optionsProto.customThemeB4				= options.customThemeB4;
	optionsProto.customThemeL1				= options.customThemeL1;
	optionsProto.customThemeR1				= options.customThemeR1;
	optionsProto.customThemeL2				= options.customThemeL2;
	optionsProto.customThemeR2				= options.customThemeR2;
	optionsProto.customThemeS1				= options.customThemeS1;
	optionsProto.customThemeS2				= options.customThemeS2;
	optionsProto.customThemeA1				= options.customThemeA1;
	optionsProto.customThemeA2				= options.customThemeA2;
	optionsProto.customThemeL3				= options.customThemeL3;
	optionsProto.customThemeR3				= options.customThemeR3;
	optionsProto.customThemeUpPressed		= options.customThemeUpPressed;
	optionsProto.customThemeDownPressed		= options.customThemeDownPressed;
	optionsProto.customThemeLeftPressed		= options.customThemeLeftPressed;
	optionsProto.customThemeRightPressed	= options.customThemeRightPressed;
	optionsProto.customThemeB1Pressed		= options.customThemeB1Pressed;
	optionsProto.customThemeB2Pressed		= options.customThemeB2Pressed;
	optionsProto.customThemeB3Pressed		= options.customThemeB3Pressed;
	optionsProto.customThemeB4Pressed		= options.customThemeB4Pressed;
	optionsProto.customThemeL1Pressed		= options.customThemeL1Pressed;
	optionsProto.customThemeR1Pressed		= options.customThemeR1Pressed;
	optionsProto.customThemeL2Pressed		= options.customThemeL2Pressed;
	optionsProto.customThemeR2Pressed		= options.customThemeR2Pressed;
	optionsProto.customThemeS1Pressed		= options.customThemeS1Pressed;
	optionsProto.customThemeS2Pressed		= options.customThemeS2Pressed;
	optionsProto.customThemeA1Pressed		= options.customThemeA1Pressed;
	optionsProto.customThemeA2Pressed		= options.customThemeA2Pressed;
	optionsProto.customThemeL3Pressed		= options.customThemeL3Pressed;
	optionsProto.customThemeR3Pressed		= options.customThemeR3Pressed;
}

void Storage::performEnqueuedSaves()
{
	if (animationOptionsSavePending.load())
	{
		critical_section_enter_blocking(&animationOptionsCs);
		updateAnimationOptionsProto(animationOptionsToSave);
		save();
		animationOptionsSavePending.store(false);
		critical_section_exit(&animationOptionsCs);
	}
}

void Storage::enqueueAnimationOptionsSave(const AnimationOptions& animationOptions)
{
	const uint32_t crc = CRC32::calculate(&animationOptions);
	critical_section_enter_blocking(&animationOptionsCs);
	if (crc != animationOptionsCrc)
	{
		animationOptionsToSave = animationOptions;
		animationOptionsCrc = crc;
		animationOptionsSavePending.store(true);
	}
	critical_section_exit(&animationOptionsCs);
}

/* Board stuffs */
void Storage::initBoardOptions() {
	setDefaultBoardOptions();
}

void Storage::setDefaultBoardOptions()
{
	// Set GP2040 version string and 0 mem after
	boardOptions.hasBoardOptions   = false;
	boardOptions.pinDpadUp         = PIN_DPAD_UP;
	boardOptions.pinDpadDown       = PIN_DPAD_DOWN;
	boardOptions.pinDpadLeft       = PIN_DPAD_LEFT;
	boardOptions.pinDpadRight      = PIN_DPAD_RIGHT;
	boardOptions.pinButtonB1       = PIN_BUTTON_B1;
	boardOptions.pinButtonB2       = PIN_BUTTON_B2;
	boardOptions.pinButtonB3       = PIN_BUTTON_B3;
	boardOptions.pinButtonB4       = PIN_BUTTON_B4;
	boardOptions.pinButtonL1       = PIN_BUTTON_L1;
	boardOptions.pinButtonR1       = PIN_BUTTON_R1;
	boardOptions.pinButtonL2       = PIN_BUTTON_L2;
	boardOptions.pinButtonR2       = PIN_BUTTON_R2;
	boardOptions.pinButtonS1       = PIN_BUTTON_S1;
	boardOptions.pinButtonS2       = PIN_BUTTON_S2;
	boardOptions.pinButtonL3       = PIN_BUTTON_L3;
	boardOptions.pinButtonR3       = PIN_BUTTON_R3;
	boardOptions.pinButtonA1       = PIN_BUTTON_A1;
	boardOptions.pinButtonA2       = PIN_BUTTON_A2;
	boardOptions.buttonLayout      = static_cast<ConfigLegacy::ButtonLayout>(BUTTON_LAYOUT);
	boardOptions.buttonLayoutRight = static_cast<ConfigLegacy::ButtonLayoutRight>(BUTTON_LAYOUT_RIGHT);
	boardOptions.splashMode        = static_cast<ConfigLegacy::SplashMode>(SPLASH_MODE);
	boardOptions.splashChoice      = static_cast<ConfigLegacy::SplashChoice>(SPLASH_CHOICE);
	boardOptions.splashDuration    = SPLASH_DURATION;
	boardOptions.i2cSDAPin         = I2C_SDA_PIN;
	boardOptions.i2cSCLPin         = I2C_SCL_PIN;
	boardOptions.i2cBlock          = (I2C_BLOCK == i2c0) ? 0 : 1;
	boardOptions.i2cSpeed          = I2C_SPEED;
	boardOptions.hasI2CDisplay     = HAS_I2C_DISPLAY;
	boardOptions.displayI2CAddress = DISPLAY_I2C_ADDR;
	boardOptions.displaySize       = DISPLAY_SIZE;
	boardOptions.displayFlip       = DISPLAY_FLIP;
	boardOptions.displayInvert     = DISPLAY_INVERT;
	boardOptions.displaySaverTimeout     = DISPLAY_SAVER_TIMEOUT;

	ConfigLegacy::ButtonLayoutParams params = {
		.layout = static_cast<ConfigLegacy::ButtonLayout>(BUTTON_LAYOUT),
		.startX = 8,
		.startY = 28,
		.buttonRadius = 8,
		.buttonPadding = 2
	};
	boardOptions.buttonLayoutCustomOptions.params = params;

	ConfigLegacy::ButtonLayoutParams paramsRight = {
		.layoutRight = static_cast<ConfigLegacy::ButtonLayoutRight>(BUTTON_LAYOUT_RIGHT),
		.startX = 8,
		.startY = 28,
		.buttonRadius = 8,
		.buttonPadding = 2
	};
	boardOptions.buttonLayoutCustomOptions.paramsRight = paramsRight;

	strncpy(boardOptions.boardVersion, GP2040VERSION, strlen(GP2040VERSION));
	setBoardOptions(boardOptions);
}

void Storage::setBoardOptions(ConfigLegacy::BoardOptions options)
{
}

/* LED stuffs */
void Storage::initLEDOptions()
{
	setDefaultLEDOptions();
	setPLEDPins(ledOptions.pledPin1, ledOptions.pledPin2, ledOptions.pledPin3, ledOptions.pledPin4);
}

void Storage::setDefaultLEDOptions()
{
	ledOptions.dataPin = BOARD_LEDS_PIN;
	ledOptions.ledFormat = LED_FORMAT;
	ledOptions.ledLayout = static_cast<ConfigLegacy::ButtonLayout>(BUTTON_LAYOUT);
	ledOptions.ledsPerButton = LEDS_PER_PIXEL;
	ledOptions.brightnessMaximum = LED_BRIGHTNESS_MAXIMUM;
	ledOptions.brightnessSteps = LED_BRIGHTNESS_STEPS;
	ledOptions.indexUp = LEDS_DPAD_UP;
	ledOptions.indexDown = LEDS_DPAD_DOWN;
	ledOptions.indexLeft = LEDS_DPAD_LEFT;
	ledOptions.indexRight = LEDS_DPAD_RIGHT;
	ledOptions.indexB1 = LEDS_BUTTON_B1;
	ledOptions.indexB2 = LEDS_BUTTON_B2;
	ledOptions.indexB3 = LEDS_BUTTON_B3;
	ledOptions.indexB4 = LEDS_BUTTON_B4;
	ledOptions.indexL1 = LEDS_BUTTON_L1;
	ledOptions.indexR1 = LEDS_BUTTON_R1;
	ledOptions.indexL2 = LEDS_BUTTON_L2;
	ledOptions.indexR2 = LEDS_BUTTON_R2;
	ledOptions.indexS1 = LEDS_BUTTON_S1;
	ledOptions.indexS2 = LEDS_BUTTON_S2;
	ledOptions.indexL3 = LEDS_BUTTON_L3;
	ledOptions.indexR3 = LEDS_BUTTON_R3;
	ledOptions.indexA1 = LEDS_BUTTON_A1;
	ledOptions.indexA2 = LEDS_BUTTON_A2;
	ledOptions.pledType = PLED_TYPE;
	ledOptions.pledPin1 = PLED1_PIN;
	ledOptions.pledPin2 = PLED2_PIN;
	ledOptions.pledPin3 = PLED3_PIN;
	ledOptions.pledPin4 = PLED4_PIN;
	ledOptions.pledColor = ColorWhite;
	setLEDOptions(ledOptions);
}

void Storage::setLEDOptions(ConfigLegacy::LEDOptions options)
{
}

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

void Storage::initPreviewBoardOptions()
{
	memcpy(&previewBoardOptions, &boardOptions, sizeof(ConfigLegacy::BoardOptions));
}

void Storage::setPreviewBoardOptions(const ConfigLegacy::BoardOptions& boardOptions)
{
	memcpy(&previewBoardOptions, &boardOptions, sizeof(ConfigLegacy::BoardOptions));
}

void Storage::SetConfigMode(bool mode) { // hack for config mode
	CONFIG_MODE = mode;
	initPreviewBoardOptions();
}

bool Storage::GetConfigMode()
{
	return CONFIG_MODE;
}

void Storage::SetGamepad(Gamepad * newpad)
{
	gamepad = newpad;
}

Gamepad * Storage::GetGamepad()
{
	return gamepad;
}

void Storage::SetProcessedGamepad(Gamepad * newpad)
{
	processedGamepad = newpad;
}

Gamepad * Storage::GetProcessedGamepad()
{
	return processedGamepad;
}

void Storage::SetFeatureData(uint8_t * newData)
{
	memcpy(newData, featureData, sizeof(uint8_t)*sizeof(featureData));
}

void Storage::ClearFeatureData()
{
	memset(featureData, 0, sizeof(uint8_t)*sizeof(featureData));
}

uint8_t * Storage::GetFeatureData()
{
	return featureData;
}

/* Animation stuffs */
AnimationOptions AnimationStorage::getAnimationOptions()
{
	AnimationOptions options;
	const AnimationOptions_Proto& optionsProto = Storage::getInstance().getAnimationOptions();

	options.checksum				= 0;
	options.baseAnimationIndex		= std::min<uint32_t>(optionsProto.baseAnimationIndex, 255);
	options.brightness				= std::min<uint32_t>(optionsProto.brightness, 255);
	options.staticColorIndex		= std::min<uint32_t>(optionsProto.staticColorIndex, 255);
	options.buttonColorIndex		= std::min<uint32_t>(optionsProto.buttonColorIndex, 255);
	options.chaseCycleTime			= std::min<int32_t>(optionsProto.chaseCycleTime, 65535);
	options.rainbowCycleTime		= std::min<int32_t>(optionsProto.rainbowCycleTime, 65535);
	options.themeIndex				= std::min<uint8_t>(optionsProto.themeIndex, 255);
	options.hasCustomTheme			= optionsProto.hasCustomTheme;
	options.customThemeUp			= optionsProto.customThemeUp;
	options.customThemeDown			= optionsProto.customThemeDown;
	options.customThemeLeft			= optionsProto.customThemeLeft;
	options.customThemeRight		= optionsProto.customThemeRight;
	options.customThemeB1			= optionsProto.customThemeB1;
	options.customThemeB2			= optionsProto.customThemeB2;
	options.customThemeB3			= optionsProto.customThemeB3;
	options.customThemeB4			= optionsProto.customThemeB4;
	options.customThemeL1			= optionsProto.customThemeL1;
	options.customThemeR1			= optionsProto.customThemeR1;
	options.customThemeL2			= optionsProto.customThemeL2;
	options.customThemeR2			= optionsProto.customThemeR2;
	options.customThemeS1			= optionsProto.customThemeS1;
	options.customThemeS2			= optionsProto.customThemeS2;
	options.customThemeA1			= optionsProto.customThemeA1;
	options.customThemeA2			= optionsProto.customThemeA2;
	options.customThemeL3			= optionsProto.customThemeL3;
	options.customThemeR3			= optionsProto.customThemeR3;
	options.customThemeUpPressed	= optionsProto.customThemeUpPressed;
	options.customThemeDownPressed	= optionsProto.customThemeDownPressed;
	options.customThemeLeftPressed	= optionsProto.customThemeLeftPressed;
	options.customThemeRightPressed	= optionsProto.customThemeRightPressed;
	options.customThemeB1Pressed	= optionsProto.customThemeB1Pressed;
	options.customThemeB2Pressed	= optionsProto.customThemeB2Pressed;
	options.customThemeB3Pressed	= optionsProto.customThemeB3Pressed;
	options.customThemeB4Pressed	= optionsProto.customThemeB4Pressed;
	options.customThemeL1Pressed	= optionsProto.customThemeL1Pressed;
	options.customThemeR1Pressed	= optionsProto.customThemeR1Pressed;
	options.customThemeL2Pressed	= optionsProto.customThemeL2Pressed;
	options.customThemeR2Pressed	= optionsProto.customThemeR2Pressed;
	options.customThemeS1Pressed	= optionsProto.customThemeS1Pressed;
	options.customThemeS2Pressed	= optionsProto.customThemeS2Pressed;
	options.customThemeA1Pressed	= optionsProto.customThemeA1Pressed;
	options.customThemeA2Pressed	= optionsProto.customThemeA2Pressed;
	options.customThemeL3Pressed	= optionsProto.customThemeL3Pressed;
	options.customThemeR3Pressed	= optionsProto.customThemeR3Pressed;

	return options;
}

void AnimationStorage::save()
{
	Storage::getInstance().enqueueAnimationOptionsSave(AnimationStation::options);
}
