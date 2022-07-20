 /*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include "NeoPico.hpp"
#include "pico/util/queue.h"

#include "enums.h"
#include "helper.h"
#include "gamepad.h"
#include "gpmodule.h"

#define GAMEPAD_STORAGE_INDEX      0 // 1024 bytes for gamepad options
#define BOARD_STORAGE_INDEX     1024 //  512 bytes for hardware options
#define LED_STORAGE_INDEX       1536 //  512 bytes for LED configuration
#define ANIMATION_STORAGE_INDEX 2048 // ???? bytes for LED animations

struct BoardOptions
{
	bool hasBoardOptions;
	uint8_t pinDpadUp;
	uint8_t pinDpadDown;
	uint8_t pinDpadLeft;
	uint8_t pinDpadRight;
	uint8_t pinButtonB1;
	uint8_t pinButtonB2;
	uint8_t pinButtonB3;
	uint8_t pinButtonB4;
	uint8_t pinButtonL1;
	uint8_t pinButtonR1;
	uint8_t pinButtonL2;
	uint8_t pinButtonR2;
	uint8_t pinButtonS1;
	uint8_t pinButtonS2;
	uint8_t pinButtonL3;
	uint8_t pinButtonR3;
	uint8_t pinButtonA1;
	uint8_t pinButtonA2;
	ButtonLayout buttonLayout;

	int i2cSDAPin;
	int i2cSCLPin;
	int i2cBlock;
	uint32_t i2cSpeed;

	bool hasI2CDisplay;
	int displayI2CAddress;
	uint8_t displaySize;
	bool displayFlip;
	bool displayInvert;

	uint32_t checksum;
};

struct LEDOptions
{
	bool useUserDefinedLEDs;
	int dataPin;
	LEDFormat ledFormat;
	ButtonLayout ledLayout;
	uint8_t ledsPerButton;
	uint8_t brightnessMaximum;
	uint8_t brightnessSteps;
	int indexUp;
	int indexDown;
	int indexLeft;
	int indexRight;
	int indexB1;
	int indexB2;
	int indexB3;
	int indexB4;
	int indexL1;
	int indexR1;
	int indexL2;
	int indexR2;
	int indexS1;
	int indexS2;
	int indexL3;
	int indexR3;
	int indexA1;
	int indexA2;
};

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

	BoardOptions getBoardOptions();
	void setBoardOptions(BoardOptions options);
	LEDOptions getLEDOptions();
	void setLEDOptions(LEDOptions options);
	void setDefaultLEDOptions(); // set defaults

	void SetConfigMode(bool mode) { // hack for config mode
		CONFIG_MODE = mode;
	}
	bool GetConfigMode() { return CONFIG_MODE; }
	void SetGamepadQueue(queue_t queuein) {
		gamepadQueue = queuein;
	}
	queue_t * GetGamepadQueue() { return &gamepadQueue; }
	void SetFeatureQueue(queue_t queuein) {
		featureQueue = queuein;
	}
	queue_t * GetFeatureQueue() { return &featureQueue; }
	void SetGamepad(Gamepad * newpad) {
		gamepad = newpad;
	}
	Gamepad * GetGamepad() { return gamepad; }

	void ResetSettings(); // EEPROM Reset Feature

	std::vector<GPModule*> Modules;

private:
	Storage() {}

	bool CONFIG_MODE; // stack storage for cross-thread chatter
	queue_t gamepadQueue; // Passing Gamepad Data
	queue_t featureQueue; // Passing Feature Data (X-Input Player #)
	Gamepad * gamepad;
};

#endif
