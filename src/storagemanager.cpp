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
#include <sstream>

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
#include "addons/pleds.h"
#include "addons/reverse.h"
#include "addons/turbo.h"

#include "bitmaps.h"

#include "helper.h"

#include "config.pb.h"

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

BoardOptions Storage::getBoardOptions()
{
	return boardOptions;
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

AddonOptions Storage::getAddonOptions()
{
	return addonOptions;
}

void Storage::setDefaultAddonOptions()
{
	addonOptions.pinButtonTurbo    		= PIN_BUTTON_TURBO;
	addonOptions.pinButtonReverse  		= PIN_BUTTON_REVERSE;
	addonOptions.pinSliderLS       		= PIN_SLIDER_LS;
	addonOptions.pinSliderRS       		= PIN_SLIDER_RS;
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
	addonOptions.AnalogInputEnabled     = ANALOG_INPUT_ENABLED;
	addonOptions.BoardLedAddonEnabled   = BOARD_LED_ENABLED;
	addonOptions.BootselButtonAddonEnabled = BOOTSEL_BUTTON_ENABLED;
	addonOptions.BuzzerSpeakerAddonEnabled = BUZZER_ENABLED;
	addonOptions.DualDirectionalInputEnabled = DUAL_DIRECTIONAL_ENABLED;
	addonOptions.ExtraButtonAddonEnabled = EXTRA_BUTTON_ENABLED;
	addonOptions.I2CAnalog1219InputEnabled = I2C_ANALOG1219_ENABLED;
	addonOptions.JSliderInputEnabled    = JSLIDER_ENABLED;
	addonOptions.PlayerNumAddonEnabled  = PLAYERNUM_ADDON_ENABLED;
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

SplashImage Storage::getSplashImage()
{
	return splashImage;
}

void Storage::setDefaultSplashImage()
{
	memcpy(&splashImage.data, &splashImageMain, sizeof(splashImageMain));
	setSplashImage(splashImage);
}

void Storage::setSplashImage(SplashImage image)
{
	if (memcmp(&splashImage, &image, sizeof(SplashImage)) != 0)
	{
		image.checksum = CHECKSUM_MAGIC; // set checksum to magic number
		image.checksum = CRC32::calculate(&image);
		EEPROM.set(SPLASH_IMAGE_STORAGE_INDEX, image);
		EEPROM.commit();
		memcpy(&splashImage, &image, sizeof(SplashImage));
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

LEDOptions Storage::getLEDOptions()
{
	return ledOptions;
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

BoardOptions Storage::getPreviewBoardOptions()
{
	return previewBoardOptions;
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

static void writeIndentation(std::string& str, int level)
{
	str.append(static_cast<std::string::size_type>(level), '\t');
}

#define PREPROCESSOR_JOIN2(x, y) x ## y
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN2(x, y)

#define TO_JSON_UENUM(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_INT32(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_UINT32(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_BOOL(fieldname, submessageType) str.append((s.fieldname) ? "true" : "false");
#define TO_JSON_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname); str.push_back('"');
#define TO_JSON_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname, indentLevel + 1);

#define TO_JSON_REPEATED_UENUM(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_INT32(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_UINT32(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_BOOL(fieldname, submessageType) str.append((s.fieldname[i]) ? "true" : "false");
#define TO_JSON_REPEATED_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname[i]); str.push_back('"');
#define TO_JSON_REPEATED_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname[i], indentLevel + 1);

#define TO_JSON_REPEATED(ltype, fieldname, submessageType) \
	str.append("["); \
	for (int i = 0; i < s.PREPROCESSOR_JOIN(fieldname, _count); ++i) \
	{ \
		if (i != 0) str.append(",");\
		str.append("\n"); \
		writeIndentation(str, indentLevel + 1); \
		PREPROCESSOR_JOIN(TO_JSON_REPEATED_, ltype)(fieldname, submessageType) \
	} \
	str.append("\n"); \
	writeIndentation(str, indentLevel); \
	str.append("]"); \

#define TO_JSON_REQUIRED(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_OPTIONAL(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_SINGULAR(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_FIXARRAY(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_ONEOF(ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_STATIC(htype, ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, htype)(ltype, fieldname, submessageType)
#define TO_JSON_POINTER(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_CALLBACK(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_FIELD(parenttype, atype, htype, ltype, fieldname, tag) \
	if (!firstField) str.append(",\n"); \
	firstField = false; \
	writeIndentation(str, indentLevel); \
	str.append("\"" #fieldname "\": "); \
	PREPROCESSOR_JOIN(TO_JSON_, atype)(htype, ltype, fieldname, parenttype ## _ ## fieldname ## _MSGTYPE)

#define GEN_TO_JSON_FUNCTION_DECL(structtype) static void toJSON ## structtype(std::ostringstream& oss, const structtype& s, int indentLevel);

#define GEN_TO_JSON_FUNCTION(structtype) \
	static void toJSON ## structtype(std::string& str, const structtype& s, int indentLevel) \
	{ \
		bool firstField = true; \
		str.append("{\n"); \
		structtype ## _FIELDLIST(TO_JSON_FIELD, structtype) \
		str.push_back('\n'); \
		writeIndentation(str, indentLevel - 1); \
		str.push_back('}'); \
	}

CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION_DECL)
CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION)

std::string Storage::toJSON() const
{
	Config config = Config_init_default;
	
	initMissingPropertiesWithDefaults(config);

	std::string str;
	str.reserve(1024 * 4);
	toJSONConfig(str, config, 1);
	str.push_back('\n');

	return str;
}

#define INIT_MISSING_PROPERTY(parent, property, value) \
	if (!parent.PREPROCESSOR_JOIN(has_, property)) \
	{ \
		parent.property = value; \
		parent.PREPROCESSOR_JOIN(has_, property) = true; \
	} \

#define INIT_MISSING_PROPERTY_STR(parent, property, str) \
	if (!parent.PREPROCESSOR_JOIN(has_, property)) \
	{ \
		strncpy(parent.property, str, sizeof(parent.property)); \
		parent.property[sizeof(parent.property) - 1] = '\0'; \
		parent.PREPROCESSOR_JOIN(has_, property) = true; \
	} \

void Storage::initMissingPropertiesWithDefaults(Config& config)
{
	INIT_MISSING_PROPERTY_STR(config, boardVersion, GP2040VERSION);

	// gamepadOptions
	INIT_MISSING_PROPERTY(config.gamepadOptions, inputMode, InputMode_Proto_INPUT_MODE_XINPUT);
	INIT_MISSING_PROPERTY(config.gamepadOptions, dpadMode, DpadMode_Proto_DPAD_MODE_DIGITAL);
#ifdef DEFAULT_SOCD_MODE
	INIT_MISSING_PROPERTY(config.gamepadOptions, socdMode, static_cast<SOCDMode_Proto>(DEFAULT_SOCD_MODE));
#else
	INIT_MISSING_PROPERTY(config.gamepadOptions, socdMode, SOCDMode_Proto_SOCD_MODE_NEUTRAL);
#endif
	INIT_MISSING_PROPERTY(config.gamepadOptions, invertXAxis, false);
	INIT_MISSING_PROPERTY(config.gamepadOptions, invertYAxis, false);

	// pinMappings
	INIT_MISSING_PROPERTY(config.pinMappings, pinDpadUp, PIN_DPAD_UP);
	INIT_MISSING_PROPERTY(config.pinMappings, pinDpadDown, PIN_DPAD_DOWN);
	INIT_MISSING_PROPERTY(config.pinMappings, pinDpadLeft, PIN_DPAD_LEFT);
	INIT_MISSING_PROPERTY(config.pinMappings, pinDpadRight, PIN_DPAD_RIGHT);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonB1, PIN_BUTTON_B1);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonB2, PIN_BUTTON_B2);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonB3, PIN_BUTTON_B3);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonB4, PIN_BUTTON_B4);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonL1, PIN_BUTTON_L1);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonR1, PIN_BUTTON_R1);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonL2, PIN_BUTTON_L2);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonR2, PIN_BUTTON_R2);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonS1, PIN_BUTTON_S1);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonS2, PIN_BUTTON_S2);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonL3, PIN_BUTTON_L3);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonR3, PIN_BUTTON_R3);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonA1, PIN_BUTTON_A1);
	INIT_MISSING_PROPERTY(config.pinMappings, pinButtonA2, PIN_BUTTON_A2);

	// displayOptions
	INIT_MISSING_PROPERTY(config.displayOptions, enabled, !!HAS_I2C_DISPLAY);
	INIT_MISSING_PROPERTY(config.displayOptions, i2cBlock, (I2C_BLOCK == i2c0) ? 0 : 1);
	INIT_MISSING_PROPERTY(config.displayOptions, i2cSDAPin, I2C_SDA_PIN);
	INIT_MISSING_PROPERTY(config.displayOptions, i2cSCLPin, I2C_SCL_PIN);
	INIT_MISSING_PROPERTY(config.displayOptions, i2cAddress, DISPLAY_I2C_ADDR);
	INIT_MISSING_PROPERTY(config.displayOptions, i2cSpeed, I2C_SPEED);
	INIT_MISSING_PROPERTY(config.displayOptions, buttonLayout, BUTTON_LAYOUT);
	INIT_MISSING_PROPERTY(config.displayOptions, buttonLayoutRight, BUTTON_LAYOUT_RIGHT);

	ButtonLayoutParamsLeft& paramsLeft = config.displayOptions.buttonLayoutCustomOptions.paramsLeft;
	INIT_MISSING_PROPERTY(paramsLeft, layout, BUTTON_LAYOUT);
	INIT_MISSING_PROPERTY(paramsLeft.common, startX, 8);
	INIT_MISSING_PROPERTY(paramsLeft.common, startY, 28);
	INIT_MISSING_PROPERTY(paramsLeft.common, buttonRadius, 8);
	INIT_MISSING_PROPERTY(paramsLeft.common, buttonPadding, 2);

	ButtonLayoutParamsRight& paramsRight = config.displayOptions.buttonLayoutCustomOptions.paramsRight;
	INIT_MISSING_PROPERTY(paramsRight, layout, BUTTON_LAYOUT_RIGHT);
	INIT_MISSING_PROPERTY(paramsRight.common, startX, 8);
	INIT_MISSING_PROPERTY(paramsRight.common, startY, 28);
	INIT_MISSING_PROPERTY(paramsRight.common, buttonRadius, 8);
	INIT_MISSING_PROPERTY(paramsRight.common, buttonPadding, 2);

	INIT_MISSING_PROPERTY(config.displayOptions, splashMode, SPLASH_MODE);
	INIT_MISSING_PROPERTY(config.displayOptions, splashChoice, SPLASH_CHOICE);
	INIT_MISSING_PROPERTY(config.displayOptions, splashDuration, SPLASH_DURATION);
	INIT_MISSING_PROPERTY_STR(config.displayOptions, splashImage, "");
	INIT_MISSING_PROPERTY(config.displayOptions, size, DISPLAY_SIZE);
	INIT_MISSING_PROPERTY(config.displayOptions, flip, DISPLAY_FLIP);
	INIT_MISSING_PROPERTY(config.displayOptions, invert, !!DISPLAY_INVERT);
	INIT_MISSING_PROPERTY(config.displayOptions, displaySaverTimeout, DISPLAY_SAVER_TIMEOUT);

	// ledOptions
	INIT_MISSING_PROPERTY(config.ledOptions, dataPin, BOARD_LEDS_PIN);
	INIT_MISSING_PROPERTY(config.ledOptions, ledFormat, static_cast<LEDFormat_Proto>(LED_FORMAT));
	INIT_MISSING_PROPERTY(config.ledOptions, ledLayout, BUTTON_LAYOUT);
	INIT_MISSING_PROPERTY(config.ledOptions, ledsPerButton, LEDS_PER_PIXEL);
	INIT_MISSING_PROPERTY(config.ledOptions, brightnessMaximum, LED_BRIGHTNESS_MAXIMUM);
	INIT_MISSING_PROPERTY(config.ledOptions, brightnessSteps, LED_BRIGHTNESS_STEPS);

	INIT_MISSING_PROPERTY(config.ledOptions, indexUp, LEDS_DPAD_UP);
	INIT_MISSING_PROPERTY(config.ledOptions, indexDown, LEDS_DPAD_DOWN);
	INIT_MISSING_PROPERTY(config.ledOptions, indexLeft, LEDS_DPAD_LEFT);
	INIT_MISSING_PROPERTY(config.ledOptions, indexRight, LEDS_DPAD_RIGHT);
	INIT_MISSING_PROPERTY(config.ledOptions, indexB1, LEDS_BUTTON_B1);
	INIT_MISSING_PROPERTY(config.ledOptions, indexB2, LEDS_BUTTON_B2);
	INIT_MISSING_PROPERTY(config.ledOptions, indexB3, LEDS_BUTTON_B3);
	INIT_MISSING_PROPERTY(config.ledOptions, indexB4, LEDS_BUTTON_B4);
	INIT_MISSING_PROPERTY(config.ledOptions, indexL1, LEDS_BUTTON_L1);
	INIT_MISSING_PROPERTY(config.ledOptions, indexR1, LEDS_BUTTON_R1);
	INIT_MISSING_PROPERTY(config.ledOptions, indexL2, LEDS_BUTTON_L2);
	INIT_MISSING_PROPERTY(config.ledOptions, indexR2, LEDS_BUTTON_R2);
	INIT_MISSING_PROPERTY(config.ledOptions, indexS1, LEDS_BUTTON_S1);
	INIT_MISSING_PROPERTY(config.ledOptions, indexS2, LEDS_BUTTON_S2);
	INIT_MISSING_PROPERTY(config.ledOptions, indexL3, LEDS_BUTTON_L3);
	INIT_MISSING_PROPERTY(config.ledOptions, indexR3, LEDS_BUTTON_R3);
	INIT_MISSING_PROPERTY(config.ledOptions, indexA1, LEDS_BUTTON_A1);
	INIT_MISSING_PROPERTY(config.ledOptions, indexA2, LEDS_BUTTON_A2);

	// animationOptions
	INIT_MISSING_PROPERTY(config.animationOptions, baseAnimationIndex, LEDS_BASE_ANIMATION_INDEX);
	INIT_MISSING_PROPERTY(config.animationOptions, brightness, LEDS_BRIGHTNESS);
	INIT_MISSING_PROPERTY(config.animationOptions, staticColorIndex, LEDS_STATIC_COLOR_INDEX);
	INIT_MISSING_PROPERTY(config.animationOptions, buttonColorIndex, LEDS_BUTTON_COLOR_INDEX);
	INIT_MISSING_PROPERTY(config.animationOptions, chaseCycleTime, LEDS_CHASE_CYCLE_TIME);
	INIT_MISSING_PROPERTY(config.animationOptions, rainbowCycleTime, LEDS_RAINBOW_CYCLE_TIME);
	INIT_MISSING_PROPERTY(config.animationOptions, themeIndex, LEDS_THEME_INDEX);

	// addonOptions.bootselButtonOptions
	INIT_MISSING_PROPERTY(config.addonOptions.bootselButtonOptions, enabled, !!BOOTSEL_BUTTON_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.bootselButtonOptions, buttonMap, BOOTSEL_BUTTON_MASK);

	// addonOptions.onBoardLedOptions
	INIT_MISSING_PROPERTY(config.addonOptions.onBoardLedOptions, mode, BOARD_LED_TYPE);

	// addonOptions.analogOptions
	INIT_MISSING_PROPERTY(config.addonOptions.analogOptions, enabled, !!ANALOG_INPUT_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.analogOptions, analogAdcPinX, ANALOG_ADC_VRX);
	INIT_MISSING_PROPERTY(config.addonOptions.analogOptions, analogAdcPinY, ANALOG_ADC_VRY);

	// addonOptions.turboOptions
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, enabled, !!TURBO_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, buttonPin, PIN_BUTTON_TURBO);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, ledPin, TURBO_LED_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shotCount, DEFAULT_SHOT_PER_SEC);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupDialPin, PIN_SHMUP_DIAL);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupModeEnabled, !!TURBO_SHMUP_MODE);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn1, SHMUP_ALWAYS_ON1);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn2, SHMUP_ALWAYS_ON2);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn3, SHMUP_ALWAYS_ON3);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn4, SHMUP_ALWAYS_ON4);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtn1Pin, PIN_SHMUP_BUTTON1);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtn2Pin, PIN_SHMUP_BUTTON2);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtn3Pin, PIN_SHMUP_BUTTON3);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtn4Pin, PIN_SHMUP_BUTTON4);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask1, SHMUP_BUTTON1);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask2, SHMUP_BUTTON2);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask3, SHMUP_BUTTON3);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask4, SHMUP_BUTTON4);
	INIT_MISSING_PROPERTY(config.addonOptions.turboOptions, shmupMixMode, SHMUP_MIX_MODE);

	// addonOptions.sliderOptions
	INIT_MISSING_PROPERTY(config.addonOptions.sliderOptions, enabled, !!JSLIDER_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.sliderOptions, pinLS, PIN_SLIDER_LS);
	INIT_MISSING_PROPERTY(config.addonOptions.sliderOptions, pinRS, PIN_SLIDER_RS);

	// addonOptions.reverseOptions
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, enabled, !!REVERSE_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, buttonPin, PIN_REVERSE);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, ledPin, REVERSE_LED_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, actionUp, REVERSE_UP_DEFAULT);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, actionDown, REVERSE_DOWN_DEFAULT);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, actionLeft, REVERSE_LEFT_DEFAULT);
	INIT_MISSING_PROPERTY(config.addonOptions.reverseOptions, actionRight, REVERSE_RIGHT_DEFAULT);

	// addonOptions.analogADS1219Options
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, enabled, !!I2C_ANALOG1219_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, i2cBlock, (I2C_ANALOG1219_BLOCK == i2c0) ? 0 : 1)
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, i2cSDAPin, I2C_ANALOG1219_SDA_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, i2cSCLPin, I2C_ANALOG1219_SCL_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, i2cAddress, I2C_ANALOG1219_ADDRESS);
	INIT_MISSING_PROPERTY(config.addonOptions.analogADS1219Options, i2cSpeed, I2C_ANALOG1219_SPEED);

	// addonOptions.dualDirectionalOptions
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, enabled, !!DUAL_DIRECTIONAL_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, upPin, PIN_DUAL_DIRECTIONAL_UP);
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, downPin, PIN_DUAL_DIRECTIONAL_DOWN)
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, leftPin, PIN_DUAL_DIRECTIONAL_LEFT);
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, rightPin, PIN_DUAL_DIRECTIONAL_RIGHT);
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, dpadMode, static_cast<DpadMode_Proto>(DUAL_DIRECTIONAL_STICK_MODE));
	INIT_MISSING_PROPERTY(config.addonOptions.dualDirectionalOptions, combineMode, DUAL_DIRECTIONAL_COMBINE_MODE);

	// addonOptions.buzzerOptions
	INIT_MISSING_PROPERTY(config.addonOptions.buzzerOptions, enabled, !!BUZZER_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.buzzerOptions, pin, BUZZER_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.buzzerOptions, volume, BUZZER_VOLUME);

	// addonOptions.extraOptions
	INIT_MISSING_PROPERTY(config.addonOptions.extraOptions, enabled, !!EXTRA_BUTTON_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.extraOptions, pin, EXTRA_BUTTON_PIN);
	INIT_MISSING_PROPERTY(config.addonOptions.extraOptions, buttonMap, EXTRA_BUTTON_MASK);

	// addonOptions.playerNumberOptions
	INIT_MISSING_PROPERTY(config.addonOptions.playerNumberOptions, enabled, !!PLAYERNUM_ADDON_ENABLED);
	INIT_MISSING_PROPERTY(config.addonOptions.playerNumberOptions, number, PLAYER_NUMBER);
}
