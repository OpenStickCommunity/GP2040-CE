/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
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
#include "types.h"

#include "config_utils.h"

#include "bitmaps.h"

#include "helper.h"

void Storage::init() {
	EEPROM.start();
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

	for(int index = 0; index < 4; ++index) //MAX_ANIMATION_PROFILES from AnimationStation.hpp
	{
		optionsProto.profiles[index].baseCycleTime = options.profiles[index].baseCycleTime;
	}
	optionsProto.brightness					= options.brightness;
	optionsProto.baseProfileIndex			= options.baseProfileIndex;
}

static void updateSpecialMoveOptionsProto(const SpecialMoveOptions& options)
{
	//SpecialMoveOptions_Proto& optionsProto = Storage::getInstance().getSpecialMoveOptions();
	//optionsProto.CurrentProfileIndex			= options.CurrentProfileIndex;
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

	if (specialMoveOptionsSavePending.load())
	{
		critical_section_enter_blocking(&specialMoveOptionsCs);
		updateSpecialMoveOptionsProto(specialMoveOptionsToSave);
		save();
		specialMoveOptionsSavePending.store(false);
		critical_section_exit(&specialMoveOptionsCs);
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

void Storage::enqueueSpecialMoveOptionsSave(const SpecialMoveOptions& specialMoveOptions)
{
	const uint32_t crc = CRC32::calculate(&specialMoveOptions);
	critical_section_enter_blocking(&specialMoveOptionsCs);
	if (crc != specialMoveOptionsCrc)
	{
		specialMoveOptionsToSave = specialMoveOptions;
		specialMoveOptionsCrc = crc;
		specialMoveOptionsSavePending.store(true);
	}
	critical_section_exit(&specialMoveOptionsCs);
}

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

bool Storage::setProfile(const uint32_t profileNum)
{
	// is this profile defined?
	if (profileNum >= 1 && profileNum <= config.profileOptions.gpioMappingsSets_count + 1) {
		// is this profile enabled?
		// profile 1 (core) is always enabled, others we must check
		if (profileNum == 1 || config.profileOptions.gpioMappingsSets[profileNum-2].enabled) {
			this->config.gamepadOptions.profileNumber = profileNum;
			return true;
		}
	}
	// if we get here, the requested profile doesn't exist or isn't enabled, so don't change it
	return false;
}

void Storage::nextProfile()
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	uint32_t requestedProfile = (this->config.gamepadOptions.profileNumber % profileCeiling) + 1;
	while (!setProfile(requestedProfile)) {
		// if the set failed, try again with the next in the sequence
		requestedProfile = (requestedProfile % profileCeiling) + 1;
	}
}
void Storage::previousProfile()
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	uint32_t requestedProfile = this->config.gamepadOptions.profileNumber > 1 ?
			config.gamepadOptions.profileNumber - 1 : profileCeiling;
	while (!setProfile(requestedProfile)) {
		// if the set failed, try again with the next in the sequence
		requestedProfile = requestedProfile > 1 ? requestedProfile - 1 : profileCeiling;
	}
}

/**
 * @brief Return the current profile label.
 */
char* Storage::currentProfileLabel() {
	if (this->config.gamepadOptions.profileNumber == 1)
		return this->config.gpioMappings.profileLabel;
	else
		return this->config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].profileLabel;
}

void Storage::setFunctionalPinMappings()
{
	GpioMappingInfo* alts = nullptr;
	if (config.gamepadOptions.profileNumber >= 2 &&
			config.gamepadOptions.profileNumber <= config.profileOptions.gpioMappingsSets_count + 1) {
		if (config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].enabled) {
			alts = config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].pins;
		}
	}

	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
		// assign the functional pin to the profile pin if:
		// 1: there was a profile to load
		// 2: the new action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		// 3: the old action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		// else use whatever is in the core mapping
		if (alts != nullptr &&
				alts[pin].action != GpioAction::RESERVED &&
				alts[pin].action != GpioAction::ASSIGNED_TO_ADDON &&
				this->config.gpioMappings.pins[pin].action != GpioAction::RESERVED &&
				this->config.gpioMappings.pins[pin].action != GpioAction::ASSIGNED_TO_ADDON) {
			functionalPinMappings[pin] = alts[pin];
		} else {
			functionalPinMappings[pin] = this->config.gpioMappings.pins[pin];
		}
	}
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

/* Animation stuffs */
void AnimationStorage::getAnimationOptions(AnimationOptions& options)
{
	const AnimationOptions_Proto& optionsProto = Storage::getInstance().getAnimationOptions();
	
	options.checksum				= 0;
	options.NumValidProfiles = optionsProto.profiles_count;
	for(int index = 0; index < options.NumValidProfiles && index < 4; ++index) //MAX_ANIMATION_PROFILES from AnimationStation.hpp
	{
		options.profiles[index].bEnabled = optionsProto.profiles[index].bEnabled;
		options.profiles[index].baseNonPressedEffect = (AnimationNonPressedEffects)((int)optionsProto.profiles[index].baseNonPressedEffect);
		options.profiles[index].basePressedEffect = (AnimationPressedEffects)((int)optionsProto.profiles[index].basePressedEffect);
		options.profiles[index].baseCaseEffect = (AnimationNonPressedEffects)((int)optionsProto.profiles[index].baseCaseEffect);
		options.profiles[index].baseCycleTime = optionsProto.profiles[index].baseCycleTime;
		for(unsigned int packedPinIndex = 0; packedPinIndex < (NUM_BANK0_GPIOS/4)+1; ++packedPinIndex)
		{
			int pinIndex = packedPinIndex * 4;
			options.profiles[index].notPressedStaticColors[pinIndex + 0] = optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] & 0xFF;
			options.profiles[index].notPressedStaticColors[pinIndex + 1] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 8) & 0xFF;
			options.profiles[index].notPressedStaticColors[pinIndex + 2] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 16) & 0xFF;
			options.profiles[index].notPressedStaticColors[pinIndex + 3] = (optionsProto.profiles[index].notPressedStaticColors[packedPinIndex] >> 24) & 0xFF;
			options.profiles[index].pressedStaticColors[pinIndex + 0] = optionsProto.profiles[index].pressedStaticColors[packedPinIndex] & 0xFF;
			options.profiles[index].pressedStaticColors[pinIndex + 1] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 8) & 0xFF;
			options.profiles[index].pressedStaticColors[pinIndex + 2] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 16) & 0xFF;
			options.profiles[index].pressedStaticColors[pinIndex + 3] = (optionsProto.profiles[index].pressedStaticColors[packedPinIndex] >> 24) & 0xFF;
		}
		for(unsigned int packedCaseIndex = 0; packedCaseIndex < MAX_CASE_LIGHTS / 4; ++packedCaseIndex)
		{
			int caseIndex = packedCaseIndex * 4;
			options.profiles[index].caseStaticColors[caseIndex + 0] = optionsProto.profiles[index].caseStaticColors[packedCaseIndex] & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 1] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 8) & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 2] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 16) & 0xFF;
			options.profiles[index].caseStaticColors[caseIndex + 3] = (optionsProto.profiles[index].caseStaticColors[packedCaseIndex] >> 24) & 0xFF;
		}
		options.profiles[index].buttonPressHoldTimeInMs = optionsProto.profiles[index].buttonPressHoldTimeInMs;
		options.profiles[index].buttonPressFadeOutTimeInMs = optionsProto.profiles[index].buttonPressFadeOutTimeInMs;
		options.profiles[index].nonPressedSpecialColour = optionsProto.profiles[index].nonPressedSpecialColour;
		options.profiles[index].pressedSpecialColour = optionsProto.profiles[index].pressedSpecialColour;
		options.profiles[index].bUseCaseLightsInSpecialMoves = optionsProto.profiles[index].bUseCaseLightsInSpecialMoves;
	}
	options.brightness				= std::min<uint32_t>(optionsProto.brightness, 255);
	options.baseProfileIndex		= optionsProto.baseProfileIndex;

	customColors.clear();
	for(unsigned int customColIndex = 0; customColIndex < MAX_CUSTOM_COLORS; ++customColIndex)
	{
		customColors.push_back(optionsProto.customColors[customColIndex]);
	}
}

void AnimationStorage::getSpecialMoveOptions(SpecialMoveOptions& options)
{
	options.NumValidProfiles = 0;
	
	/*const SpecialMoveOptions_Proto& optionsProto = Storage::getInstance().getSpecialMoveOptions();
	
	options.NumValidProfiles = optionsProto.profiles_count;
	for(unsigned int profileIndex = 0; profileIndex < 4 && profileIndex < options.NumValidProfiles; ++profileIndex) //MAX_SPECIALMOVE_PROFILES from SpecialMoveSystem.hpp
	{
		options.profiles[profileIndex].bEnabled = optionsProto.profiles[profileIndex].bEnabled;
		options.profiles[profileIndex].NumValidMoves = optionsProto.profiles[profileIndex].AllSpecialMoves_count;
		for(unsigned int moveIndex = 0; moveIndex < 20 && moveIndex < options.profiles[profileIndex].NumValidMoves; ++moveIndex) //MAX_SPECIALMOVES from SpecialMoveSystem.hpp
		{
			options.profiles[profileIndex].AllSpecialMoves[moveIndex].NumRequiredInputCombos = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredInputCombos_count;
			for(unsigned int inputsIndex = 0; inputsIndex < 4 && inputsIndex < options.profiles[profileIndex].AllSpecialMoves[moveIndex].NumRequiredInputCombos; ++inputsIndex) //MAX_SPECIALMOVE_INPUTTYPES_PER_MOVE from SpecialMoveSystem.hpp
			{
				options.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredInputCombos[inputsIndex] = (SpecialMoveInputTypes)((int)optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredInputCombos[inputsIndex]);
			}

			options.profiles[profileIndex].AllSpecialMoves[moveIndex].NumRequiredTriggerCombos = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredTriggerCombos_count;
			for(unsigned int triggersIndex = 0; triggersIndex < 3 && triggersIndex < options.profiles[profileIndex].AllSpecialMoves[moveIndex].NumRequiredTriggerCombos; ++triggersIndex) //MAX_SPECIALMOVE_TRIGGERS_PER_MOVE from SpecialMoveSystem.hpp
			{
				options.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredTriggerCombos[triggersIndex].OptionalParams = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredTriggerCombos[triggersIndex].OptionalParams;				
				options.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredTriggerCombos[triggersIndex].RequiredTriggers = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].RequiredTriggerCombos[triggersIndex].RequiredTriggers;				
			}

			options.profiles[profileIndex].AllSpecialMoves[moveIndex].Animation = (SpecialMoveEffects)((int)optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].Animation);
 			options.profiles[profileIndex].AllSpecialMoves[moveIndex].bIsChargeMove = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].bIsChargeMove;
			options.profiles[profileIndex].AllSpecialMoves[moveIndex].Priority = optionsProto.profiles[profileIndex].AllSpecialMoves[moveIndex].Priority;
		}
	}

	options.ChargeTimeInMs			= optionsProto.ChargeTimeInMs;
	options.CurrentProfileIndex		= optionsProto.CurrentProfileIndex;*/
}

void AnimationStorage::save()
{
	Storage::getInstance().enqueueAnimationOptionsSave(AnimationStation::options);
	//Storage::getInstance().enqueueSpecialMoveOptionsSave(SpecialMoveSystem::Options);
}
