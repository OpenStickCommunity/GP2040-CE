/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "storagemanager.h"

#include "BoardConfig.h"
#include "animationstorage.h"
#include "FlashPROM.h"
#include "drivermanager.h"
#include "eventmanager.h"
#include "peripheralmanager.h"
#include "config.pb.h"
#include "hardware/watchdog.h"
#include "CRC32.h"
#include "types.h"

// Check for saves
#include "ps4/PS4Driver.h"

#include "config_utils.h"

void Storage::init() {
	systemFlashSize = System::getPhysicalFlash(); // System Flash Size must be called once
	EEPROM.start();
	ConfigUtils::load(config);
	// Initialize the cross-core processed-gamepad mutex exactly once, before Core1
	// is launched. Doing this lazily inside SetProcessedGamepad / Publish... races
	// when both cores can hit the "first init" path in parallel, since the SDK does
	// not internally serialize mutex_init on the same mutex.
	if (!processedGamepadMutexInited) {
		mutex_init(&processedGamepadMutex);
		processedGamepadMutexInited = true;
	}
}

/**
 * @brief Save the config, but only if it is safe to (as in USB host is not being used.)
 */
bool Storage::save()
{
	return save(false);
}

/**
 * @brief Save the config; if forcing a save is requested, or if USB host is not enabled, this will write to flash.
 */
bool Storage::save(const bool force) {
	// Conditions for saving:
	//   1. Force = True
	//   2. Input Mode NOT (PS4/PS5 with USB enabled)
	// Save will disconnect USB host, which is okay for gamepad and keyboard hosts
	if (!force &&
		PeripheralManager::getInstance().isUSBEnabled(0) &&
		(DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
			DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
		((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
		return false;
	}

	return ConfigUtils::save(config);
}

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

bool Storage::setProfile(const uint32_t profileNum)
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	
	// is this profile defined?
	if (profileNum >= 1 && profileNum <= profileCeiling) {
		// is this profile enabled?
		// profile 1 (core) is always enabled, others we must check
		if (profileNum == 1 || config.profileOptions.gpioMappingsSets[profileNum-2].enabled) {
			// Update the profile number - reinit will be triggered automatically in gp2040.cpp
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
	uint32_t profileNumber = this->config.gamepadOptions.profileNumber;
	uint32_t profileCeiling = this->config.profileOptions.gpioMappingsSets_count + 1;
	// Profile 1 is the base mapping, profiles 2..profileCeiling live in gpioMappingsSets[0..]
	// Guard against a corrupt config that could otherwise read past gpioMappingsSets[].
	if (profileNumber == 1) {
		return this->config.gpioMappings.profileLabel;
	}
	if (profileNumber >= 2 && profileNumber <= profileCeiling &&
			this->config.profileOptions.gpioMappingsSets[profileNumber-2].enabled) {
		return this->config.profileOptions.gpioMappingsSets[profileNumber-2].profileLabel;
	}
	return this->config.gpioMappings.profileLabel;
}

void Storage::setFunctionalPinMappings()
{
	GpioMappingInfo* alts = nullptr;
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;

	if (config.gamepadOptions.profileNumber >= 2 &&
			config.gamepadOptions.profileNumber <= profileCeiling) {
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

/**
 * @brief constructs a temporary pin-mapping in order to correctly initialize GPIO pins before
 * selecting input mode at boot
 */
void Storage::setBootModeFunctionalPinMappings()
{
	BootModeOptions& bootModeOptions = getBootModeOptions();
	if (!bootModeOptions.enabled) {
		return;
	}
	// Relying on the assumption that all profiles share same set of RESERVED/ASSIGNED_TO_ADDON pins
	GpioMappingInfo* pins = getGpioMappings().pins;

	int32_t mask = bootModeOptions.webConfigPinMask | bootModeOptions.usbModePinMask;
	for (size_t i = 0; i < bootModeOptions.inputModeMappings_count; i++) {
		auto mapping = bootModeOptions.inputModeMappings[i];
		if (mapping.pinMask == -1) {
			continue;
		}
		mask |= mapping.pinMask;
	}

	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
		if (pins[pin].action != GpioAction::RESERVED &&
			pins[pin].action != GpioAction::ASSIGNED_TO_ADDON &&
			(mask & (1 << pin)))
		{
			// Just setting an arbitrary non-zero action
			functionalPinMappings[pin].action = GpioAction::BUTTON_PRESS_A1;
		}
	}
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

void Storage::PublishProcessedGamepadState(const GamepadState& src) {
	if (processedGamepad == nullptr || !processedGamepadMutexInited) return;
	mutex_enter_blocking(&processedGamepadMutex);
	memcpy(&processedGamepad->state, &src, sizeof(GamepadState));
	mutex_exit(&processedGamepadMutex);
}

bool Storage::SnapshotProcessedGamepadState(GamepadState& dest) {
	if (processedGamepad == nullptr || !processedGamepadMutexInited) return false;
	mutex_enter_blocking(&processedGamepadMutex);
	memcpy(&dest, &processedGamepad->state, sizeof(GamepadState));
	mutex_exit(&processedGamepadMutex);
	return true;
}
