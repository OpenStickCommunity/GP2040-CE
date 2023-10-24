/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "storagemanager.h"

#include "BoardConfig.h"
#include "AnimationStorage.hpp"
#include "Effects/StaticColor.hpp"
#include "FlashPROM.h"
#include "config.pb.h"
#include "hardware/watchdog.h"
#include "Animation.hpp"
#include "CRC32.h"

#include "addons/analog.h"
#include "addons/board_led.h"
#include "addons/bootsel_button.h"
#include "addons/buzzerspeaker.h"
#include "addons/dualdirectional.h"
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
#include "addons/input_macro.h"
#include "addons/snes_input.h"
#include "addons/tilt.h"

#include "config_utils.h"

#include "bitmaps.h"

#include "helper.h"

Storage::Storage()
{
	EEPROM.start();
	gpioMappingsArray[0] = &config.gpioMappings.pin00;
	gpioMappingsArray[1] = &config.gpioMappings.pin01;
	gpioMappingsArray[2] = &config.gpioMappings.pin02;
	gpioMappingsArray[3] = &config.gpioMappings.pin03;
	gpioMappingsArray[4] = &config.gpioMappings.pin04;
	gpioMappingsArray[5] = &config.gpioMappings.pin05;
	gpioMappingsArray[6] = &config.gpioMappings.pin06;
	gpioMappingsArray[7] = &config.gpioMappings.pin07;
	gpioMappingsArray[8] = &config.gpioMappings.pin08;
	gpioMappingsArray[9] = &config.gpioMappings.pin09;
	gpioMappingsArray[10] = &config.gpioMappings.pin10;
	gpioMappingsArray[11] = &config.gpioMappings.pin11;
	gpioMappingsArray[12] = &config.gpioMappings.pin12;
	gpioMappingsArray[13] = &config.gpioMappings.pin13;
	gpioMappingsArray[14] = &config.gpioMappings.pin14;
	gpioMappingsArray[15] = &config.gpioMappings.pin15;
	gpioMappingsArray[16] = &config.gpioMappings.pin16;
	gpioMappingsArray[17] = &config.gpioMappings.pin17;
	gpioMappingsArray[18] = &config.gpioMappings.pin18;
	gpioMappingsArray[19] = &config.gpioMappings.pin19;
	gpioMappingsArray[20] = &config.gpioMappings.pin20;
	gpioMappingsArray[21] = &config.gpioMappings.pin21;
	gpioMappingsArray[22] = &config.gpioMappings.pin22;
	gpioMappingsArray[23] = &config.gpioMappings.pin23;
	gpioMappingsArray[24] = &config.gpioMappings.pin24;
	gpioMappingsArray[25] = &config.gpioMappings.pin25;
	gpioMappingsArray[26] = &config.gpioMappings.pin26;
	gpioMappingsArray[27] = &config.gpioMappings.pin27;
	gpioMappingsArray[28] = &config.gpioMappings.pin28;
	gpioMappingsArray[29] = &config.gpioMappings.pin29;
	critical_section_init(&animationOptionsCs);
	ConfigUtils::load(config);

	setFunctionalPinMappings(config.gamepadOptions.profileNumber);
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

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

void Storage::setProfile(const uint32_t profileNum)
{
	if (profileNum < 1 || profileNum > 4) return;
	setFunctionalPinMappings(profileNum);
	this->config.gamepadOptions.profileNumber = profileNum;
}

void Storage::setFunctionalPinMappings(const uint32_t profileNum)
{
	for (unsigned int pin = 0; pin < NUM_BANK0_GPIOS; pin++) {
		functionalPinMappings[pin] = *gpioMappingsArray[pin];
	}
	if (profileNum < 2 || profileNum > 4) return;

	AlternativePinMappings alts = this->config.profileOptions.alternativePinMappings[profileNum-2];

	const auto reassignProfilePin = [&](uint32_t targetPin, GpioAction newAction) -> void {
		// reassign the functional pin if:
		// 1: it's a real pin (this only matters until profiles are refactored)
		// 2: the new action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		// 3: the old action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		if (isValidPin(targetPin) && newAction != GpioAction::RESERVED &&
				newAction != GpioAction::ASSIGNED_TO_ADDON &&
				functionalPinMappings[targetPin] != GpioAction::RESERVED &&
				functionalPinMappings[targetPin] != GpioAction::ASSIGNED_TO_ADDON) {
			functionalPinMappings[targetPin] = newAction;
		}
	};
	reassignProfilePin(alts.pinButtonB1,  GpioAction::BUTTON_PRESS_B1);
	reassignProfilePin(alts.pinButtonB2,  GpioAction::BUTTON_PRESS_B2);
	reassignProfilePin(alts.pinButtonB3,  GpioAction::BUTTON_PRESS_B3);
	reassignProfilePin(alts.pinButtonB4,  GpioAction::BUTTON_PRESS_B4);
	reassignProfilePin(alts.pinButtonL1,  GpioAction::BUTTON_PRESS_L1);
	reassignProfilePin(alts.pinButtonR1,  GpioAction::BUTTON_PRESS_R1);
	reassignProfilePin(alts.pinButtonL2,  GpioAction::BUTTON_PRESS_L2);
	reassignProfilePin(alts.pinButtonR2,  GpioAction::BUTTON_PRESS_R2);
	reassignProfilePin(alts.pinDpadUp,    GpioAction::BUTTON_PRESS_UP);
	reassignProfilePin(alts.pinDpadDown,  GpioAction::BUTTON_PRESS_DOWN);
	reassignProfilePin(alts.pinDpadLeft,  GpioAction::BUTTON_PRESS_LEFT);
	reassignProfilePin(alts.pinDpadRight, GpioAction::BUTTON_PRESS_RIGHT);
}

void Storage::SetConfigMode(bool mode) { // hack for config mode
	CONFIG_MODE = mode;
	previewDisplayOptions = config.displayOptions;
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
