 /*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include "NeoPico.h"
#include "FlashPROM.h"

#include "enums.h"
#include "helper.h"
#include "gamepad.h"

#include "config.pb.h"
#include <atomic>
#include "pico/critical_section.h"
#include "eventmanager.h"
#include "GPStorageSaveEvent.h"

#define SI Storage::getInstance()

// Storage manager for board, LED options, and thread-safe settings
class Storage {
public:
	Storage(Storage const&) = delete;
	void operator=(Storage const&)  = delete;
	static Storage& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static Storage instance;
		return instance;
	}

	Config& getConfig() { return config; }
	GamepadOptions& getGamepadOptions() { return config.gamepadOptions; }
	HotkeyOptions& getHotkeyOptions() { return config.hotkeyOptions; }
	ForcedSetupOptions& getForcedSetupOptions() { return config.forcedSetupOptions; }
	PinMappings& getDeprecatedPinMappings() { return config.deprecatedPinMappings; }
	GpioMappings& getGpioMappings() { return config.gpioMappings; }
	KeyboardMapping& getKeyboardMapping() { return config.keyboardMapping; }
	DisplayOptions& getDisplayOptions() { return config.displayOptions; }
	LEDOptions& getLedOptions() { return config.ledOptions; }
	AddonOptions& getAddonOptions() { return config.addonOptions; }
	AnimationOptions& getAnimationOptions() { return config.animationOptions; }
	ProfileOptions& getProfileOptions() { return config.profileOptions; }
	GpioMappingInfo* getProfilePinMappings() { return functionalPinMappings; }
	PeripheralOptions& getPeripheralOptions() { return config.peripheralOptions; }

	void init();
	bool save();
	bool save(const bool force);

	void SetGamepad(Gamepad *); 		// MPGS Gamepad Get/Set
	Gamepad * GetGamepad();

	void SetProcessedGamepad(Gamepad *); // MPGS Processed Gamepad Get/Set
	Gamepad * GetProcessedGamepad();

	bool setProfile(const uint32_t);		// profile support for multiple mappings
	void nextProfile();
	void previousProfile();
	void setFunctionalPinMappings();
	char* currentProfileLabel();

	void ResetSettings(); 				// EEPROM Reset Feature

	uint32_t GetFlashSize() { return systemFlashSize; }

private:
	Storage() {}
	bool CONFIG_MODE = false; 			// Config mode (boot)
	Gamepad * gamepad = nullptr;    		// Gamepad data
	Gamepad * processedGamepad = nullptr; // Gamepad with ONLY processed data
	uint8_t featureData[32]; // USB X-Input Feature Data
	Config config;
	GpioMappingInfo functionalPinMappings[NUM_BANK0_GPIOS];
	uint32_t systemFlashSize;
};

#endif
