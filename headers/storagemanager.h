 /*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include "NeoPico.hpp"
#include "FlashPROM.h"

#include "enums.h"
#include "helper.h"
#include "gamepad.h"

#include "config.pb.h"
#include <atomic>
#include "pico/critical_section.h"

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
	PinMappings& getPinMappings() { return config.pinMappings; }
	KeyboardMapping& getKeyboardMapping() { return config.keyboardMapping; }
	DisplayOptions& getDisplayOptions() { return config.displayOptions; }
	DisplayOptions& getPreviewDisplayOptions() { return previewDisplayOptions; }
	LEDOptions& getLedOptions() { return config.ledOptions; }
	AddonOptions& getAddonOptions() { return config.addonOptions; }
	AnimationOptions_Proto& getAnimationOptions() { return config.animationOptions; }
	ProfileOptions& getProfileOptions() { return config.profileOptions; }

	bool save();

	PinMappings& getProfilePinMappings();

	// Perform saves that were enqueued from core1
	void performEnqueuedSaves();

	void enqueueAnimationOptionsSave(const AnimationOptions& animationOptions);

	void SetConfigMode(bool); 			// Config Mode (on-boot)
	bool GetConfigMode();

	void SetGamepad(Gamepad *); 		// MPGS Gamepad Get/Set
	Gamepad * GetGamepad();

	void SetProcessedGamepad(Gamepad *); // MPGS Processed Gamepad Get/Set
	Gamepad * GetProcessedGamepad();

	void SetFeatureData(uint8_t *); 	// USB Feature Data Get/Set
	void ClearFeatureData();
	uint8_t * GetFeatureData();

	void setProfile(const uint32_t);		// profile support for multiple mappings
	void setFunctionalPinMappings(const uint32_t);

	void ResetSettings(); 				// EEPROM Reset Feature

private:
	Storage();
	bool CONFIG_MODE = false; 			// Config mode (boot)
	Gamepad * gamepad = nullptr;    		// Gamepad data
	Gamepad * processedGamepad = nullptr; // Gamepad with ONLY processed data
	uint8_t featureData[32]; // USB X-Input Feature Data
	DisplayOptions previewDisplayOptions;
	Config config;
	std::atomic<bool> animationOptionsSavePending;
	critical_section_t animationOptionsCs;
	uint32_t animationOptionsCrc = 0;
	AnimationOptions animationOptionsToSave = {};
	PinMappings* functionalPinMappings = nullptr;
};

#endif
