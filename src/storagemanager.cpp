/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "storagemanager.h"

#include "BoardConfig.h"
#include <GamepadStorage.h>
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

#include "bitmaps.h"

#include "helper.h"

/* Board stuffs */
void Storage::initBoardOptions() {
	EEPROM.get(BOARD_STORAGE_INDEX, boardOptions);
	uint32_t lastCRC = boardOptions.checksum;
	boardOptions.checksum = CHECKSUM_MAGIC;
	if (lastCRC != CRC32::calculate(&boardOptions)) {
		setDefaultBoardOptions();
	}
}

void Storage::initAddonOptions() {
	EEPROM.get(ADDON_STORAGE_INDEX, addonOptions);
	uint32_t lastCRC = addonOptions.checksum;
	addonOptions.checksum = CHECKSUM_MAGIC;
	if (lastCRC != CRC32::calculate(&addonOptions)) {
		setDefaultAddonOptions();
	}
}

void Storage::initSplashImage() {
	EEPROM.get(SPLASH_IMAGE_STORAGE_INDEX, splashImage);
	uint32_t lastCRC = splashImage.checksum;
	splashImage.checksum = CHECKSUM_MAGIC;
	if (lastCRC != CRC32::calculate(&splashImage)) {
		setDefaultSplashImage();
	}
}

void Storage::initPS4Options() {
	EEPROM.get(PS4_STORAGE_INDEX, ps4Options);
	if (ps4Options.checksum != NOCHECKSUM_MAGIC) {
		setDefaultPS4Options();
	}
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
	boardOptions.buttonLayout      = BUTTON_LAYOUT;
	boardOptions.buttonLayoutRight = BUTTON_LAYOUT_RIGHT;
	boardOptions.splashMode        = SPLASH_MODE;
	boardOptions.splashChoice      = SPLASH_CHOICE;
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

	ButtonLayoutParams params = {
		.layout = BUTTON_LAYOUT,
		.startX = 8,
		.startY = 28,
		.buttonRadius = 8,
		.buttonPadding = 2
	};
	boardOptions.buttonLayoutCustomOptions.params = params;

	ButtonLayoutParams paramsRight = {
		.layoutRight = BUTTON_LAYOUT_RIGHT,
		.startX = 8,
		.startY = 28,
		.buttonRadius = 8,
		.buttonPadding = 2
	};
	boardOptions.buttonLayoutCustomOptions.paramsRight = paramsRight;

	strncpy(boardOptions.boardVersion, GP2040VERSION, strlen(GP2040VERSION));
	setBoardOptions(boardOptions);
}

void Storage::setBoardOptions(BoardOptions options)
{
	if (memcmp(&options, &boardOptions, sizeof(BoardOptions)) != 0)
	{
		options.checksum = CHECKSUM_MAGIC; // set checksum to magic number
		options.checksum = CRC32::calculate(&options);
		EEPROM.set(BOARD_STORAGE_INDEX, options);
		EEPROM.commit();
		memcpy(&boardOptions, &options, sizeof(BoardOptions));
	}
}

void Storage::setDefaultAddonOptions()
{
	addonOptions.pinButtonTurbo    		= PIN_BUTTON_TURBO;
	addonOptions.pinButtonReverse  		= PIN_BUTTON_REVERSE;
	addonOptions.pinSliderLS       		= PIN_SLIDER_LS;
	addonOptions.pinSliderRS       		= PIN_SLIDER_RS;
	addonOptions.pinSliderSOCDOne     	= PIN_SLIDER_SOCD_ONE;
	addonOptions.pinSliderSOCDTwo    	= PIN_SLIDER_SOCD_TWO;
	addonOptions.pinDualDirDown    		= PIN_DUAL_DIRECTIONAL_DOWN;
	addonOptions.pinDualDirUp      		= PIN_DUAL_DIRECTIONAL_UP;
	addonOptions.pinDualDirLeft    		= PIN_DUAL_DIRECTIONAL_LEFT;
	addonOptions.pinDualDirRight   		= PIN_DUAL_DIRECTIONAL_RIGHT;
	addonOptions.turboShotCount    		= DEFAULT_SHOT_PER_SEC;
	addonOptions.pinTurboLED       		= TURBO_LED_PIN;
	addonOptions.pinReverseLED     		= REVERSE_LED_PIN;
	addonOptions.reverseActionUp        = REVERSE_UP_DEFAULT;
	addonOptions.reverseActionDown      = REVERSE_DOWN_DEFAULT;
	addonOptions.reverseActionLeft      = REVERSE_LEFT_DEFAULT;
	addonOptions.reverseActionRight     = REVERSE_RIGHT_DEFAULT;
	addonOptions.i2cAnalog1219SDAPin    = I2C_ANALOG1219_SDA_PIN;
	addonOptions.i2cAnalog1219SCLPin    = I2C_ANALOG1219_SCL_PIN;
	addonOptions.i2cAnalog1219Block     = (I2C_ANALOG1219_BLOCK == i2c0) ? 0 : 1;
	addonOptions.i2cAnalog1219Speed     = I2C_ANALOG1219_SPEED;
	addonOptions.i2cAnalog1219Address   = I2C_ANALOG1219_ADDRESS;
	addonOptions.onBoardLedMode			= BOARD_LED_TYPE;
	addonOptions.dualDirDpadMode        = DUAL_DIRECTIONAL_STICK_MODE;
	addonOptions.dualDirCombineMode     = DUAL_DIRECTIONAL_COMBINE_MODE;
	addonOptions.analogAdcPinX      	= ANALOG_ADC_VRX;
	addonOptions.analogAdcPinY      	= ANALOG_ADC_VRY;
	addonOptions.bootselButtonMap		= BOOTSEL_BUTTON_MASK;
	addonOptions.buzzerPin              = BUZZER_PIN;
	addonOptions.buzzerVolume           = BUZZER_VOLUME;
	addonOptions.extraButtonMap		    = EXTRA_BUTTON_MASK;
	addonOptions.extraButtonPin		    = EXTRA_BUTTON_PIN;
	addonOptions.playerNumber           = PLAYER_NUMBER;
	addonOptions.shmupMode = TURBO_SHMUP_MODE; // Turbo SHMUP Mode
	addonOptions.shmupMixMode = SHMUP_MIX_MODE; // How we mix turbo and non-turbo buttons
	addonOptions.shmupAlwaysOn1 = SHMUP_ALWAYS_ON1;
	addonOptions.shmupAlwaysOn2 = SHMUP_ALWAYS_ON2;
	addonOptions.shmupAlwaysOn3 = SHMUP_ALWAYS_ON3;
	addonOptions.shmupAlwaysOn4 = SHMUP_ALWAYS_ON4;
	addonOptions.pinShmupBtn1 = PIN_SHMUP_BUTTON1;
	addonOptions.pinShmupBtn2 = PIN_SHMUP_BUTTON2;
	addonOptions.pinShmupBtn3 = PIN_SHMUP_BUTTON3;
	addonOptions.pinShmupBtn4 = PIN_SHMUP_BUTTON4;
	addonOptions.shmupBtnMask1 = SHMUP_BUTTON1;
	addonOptions.shmupBtnMask2 = SHMUP_BUTTON2;
	addonOptions.shmupBtnMask3 = SHMUP_BUTTON3;
	addonOptions.shmupBtnMask4 = SHMUP_BUTTON4;
	addonOptions.pinShmupDial = PIN_SHMUP_DIAL;
    addonOptions.sliderSOCDModeOne = SLIDER_SOCD_SLOT_ONE;
    addonOptions.sliderSOCDModeTwo  = SLIDER_SOCD_SLOT_TWO;
    addonOptions.sliderSOCDModeDefault = SLIDER_SOCD_SLOT_DEFAULT;
	addonOptions.AnalogInputEnabled     = ANALOG_INPUT_ENABLED;
	addonOptions.BoardLedAddonEnabled   = BOARD_LED_ENABLED;
	addonOptions.BootselButtonAddonEnabled = BOOTSEL_BUTTON_ENABLED;
	addonOptions.BuzzerSpeakerAddonEnabled = BUZZER_ENABLED;
	addonOptions.DualDirectionalInputEnabled = DUAL_DIRECTIONAL_ENABLED;
	addonOptions.ExtraButtonAddonEnabled = EXTRA_BUTTON_ENABLED;
	addonOptions.I2CAnalog1219InputEnabled = I2C_ANALOG1219_ENABLED;
	addonOptions.JSliderInputEnabled    = JSLIDER_ENABLED;
	addonOptions.SliderSOCDInputEnabled    = SLIDER_SOCD_ENABLED;
	addonOptions.PlayerNumAddonEnabled  = PLAYERNUM_ADDON_ENABLED;
	addonOptions.PS4ModeAddonEnabled    = PS4MODE_ADDON_ENABLED;
	addonOptions.ReverseInputEnabled    = REVERSE_ENABLED;
	addonOptions.TurboInputEnabled      = TURBO_ENABLED;
	setAddonOptions(addonOptions);
}

void Storage::setAddonOptions(AddonOptions options)
{
	if (memcmp(&options, &addonOptions, sizeof(AddonOptions)) != 0)
	{
		options.checksum = CHECKSUM_MAGIC; // set checksum to magic number
		options.checksum = CRC32::calculate(&options);
		EEPROM.set(ADDON_STORAGE_INDEX, options);
		EEPROM.commit();
		addonOptions = options;
	}
}

void Storage::setDefaultSplashImage()
{
	memcpy(&splashImage.data, &splashImageMain, sizeof(splashImageMain));
	setSplashImage(splashImage);
}

void Storage::setSplashImage(const SplashImage& image)
{
	if (memcmp(&splashImage, &image, sizeof(SplashImage)) != 0)
	{
		memcpy(&splashImage, &image, sizeof(SplashImage));
		splashImage.checksum = CHECKSUM_MAGIC; // set checksum to magic number
		splashImage.checksum = CRC32::calculate(&splashImage);

		EEPROM.set(SPLASH_IMAGE_STORAGE_INDEX, splashImage);
		EEPROM.commit();

		// Reset, so that the memcmp gives the correct result on the next call to this function
		splashImage.checksum = CHECKSUM_MAGIC;
	}
}

/* LED stuffs */
void Storage::initLEDOptions()
{
	EEPROM.get(LED_STORAGE_INDEX, ledOptions);
	uint32_t lastCRC = ledOptions.checksum;
	ledOptions.checksum = CHECKSUM_MAGIC;
	if (lastCRC != CRC32::calculate(&ledOptions)) {
		setDefaultLEDOptions();
	}
}

void Storage::setDefaultLEDOptions()
{
	ledOptions.dataPin = BOARD_LEDS_PIN;
	ledOptions.ledFormat = LED_FORMAT;
	ledOptions.ledLayout = BUTTON_LAYOUT;
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
	setLEDOptions(ledOptions);
}

void Storage::setLEDOptions(LEDOptions options)
{
	if (memcmp(&options, &ledOptions, sizeof(LEDOptions)) != 0)
	{
		options.checksum = CHECKSUM_MAGIC; // set checksum to magic number
		options.checksum = CRC32::calculate(&options);
		EEPROM.set(LED_STORAGE_INDEX, options);
		EEPROM.commit();
		memcpy(&ledOptions, &options, sizeof(LEDOptions));
	}
}

void Storage::savePS4Options()     // PS4 Options
{
	ps4Options.checksum = NOCHECKSUM_MAGIC;
	EEPROM.set(PS4_STORAGE_INDEX, ps4Options);
	EEPROM.commit();
}

void Storage::setDefaultPS4Options()
{
	// Zero everything out
	memset(&ps4Options, 0, sizeof(PS4Options));
}

PS4Options * Storage::getPS4Options()
{
	return &ps4Options;
}

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

void Storage::initPreviewBoardOptions()
{
	memcpy(&previewBoardOptions, &boardOptions, sizeof(BoardOptions));
}

void Storage::setPreviewBoardOptions(const BoardOptions& boardOptions)
{
	memcpy(&previewBoardOptions, &boardOptions, sizeof(BoardOptions));
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
	EEPROM.get(ANIMATION_STORAGE_INDEX, options);

	uint32_t lastCRC = options.checksum;
	options.checksum = CHECKSUM_MAGIC;
	if (CRC32::calculate(&options) != lastCRC)
	{
		options.baseAnimationIndex = LEDS_BASE_ANIMATION_INDEX;
		options.brightness         = LEDS_BRIGHTNESS;
		options.staticColorIndex   = LEDS_STATIC_COLOR_INDEX;
		options.buttonColorIndex   = LEDS_BUTTON_COLOR_INDEX;
		options.chaseCycleTime     = LEDS_CHASE_CYCLE_TIME;
		options.rainbowCycleTime   = LEDS_RAINBOW_CYCLE_TIME;
		options.themeIndex         = LEDS_THEME_INDEX;
		options.hasCustomTheme = false;
		options.customThemeUp = 0;
		options.customThemeDown = 0;
		options.customThemeLeft = 0;
		options.customThemeRight = 0;
		options.customThemeB1 = 0;
		options.customThemeB2 = 0;
		options.customThemeB3 = 0;
		options.customThemeB4 = 0;
		options.customThemeL1 = 0;
		options.customThemeR1 = 0;
		options.customThemeL2 = 0;
		options.customThemeR2 = 0;
		options.customThemeS1 = 0;
		options.customThemeS2 = 0;
		options.customThemeA1 = 0;
		options.customThemeA2 = 0;
		options.customThemeL3 = 0;
		options.customThemeR3 = 0;
		options.customThemeUpPressed = 0;
		options.customThemeDownPressed = 0;
		options.customThemeLeftPressed = 0;
		options.customThemeRightPressed = 0;
		options.customThemeB1Pressed = 0;
		options.customThemeB2Pressed = 0;
		options.customThemeB3Pressed = 0;
		options.customThemeB4Pressed = 0;
		options.customThemeL1Pressed = 0;
		options.customThemeR1Pressed = 0;
		options.customThemeL2Pressed = 0;
		options.customThemeR2Pressed = 0;
		options.customThemeS1Pressed = 0;
		options.customThemeS2Pressed = 0;
		options.customThemeA1Pressed = 0;
		options.customThemeA2Pressed = 0;
		options.customThemeL3Pressed = 0;
		options.customThemeR3Pressed = 0;

		setAnimationOptions(options);
	}

	return options;
}

void AnimationStorage::setAnimationOptions(AnimationOptions options)
{
	options.checksum = CHECKSUM_MAGIC;
	options.checksum = CRC32::calculate(&options);
	EEPROM.set(ANIMATION_STORAGE_INDEX, options);
}

void AnimationStorage::save()
{
	bool dirty = false;
	AnimationOptions savedOptions = getAnimationOptions();

	if (memcmp(&savedOptions, &AnimationStation::options, sizeof(AnimationOptions)) != 0)
	{
		this->setAnimationOptions(AnimationStation::options);
		dirty = true;
	}

	if (dirty)
		EEPROM.commit();
}
