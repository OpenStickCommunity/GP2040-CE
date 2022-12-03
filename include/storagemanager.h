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

#define GAMEPAD_STORAGE_INDEX      		0    // 1024 bytes for gamepad options
#define BOARD_STORAGE_INDEX     		1024 //  512 bytes for hardware options
#define LED_STORAGE_INDEX       		1536 //  512 bytes for LED configuration
#define ANIMATION_STORAGE_INDEX 		2048 // 1024 bytes for LED animations
#define SPLASH_IMAGE_STORAGE_INDEX		2560

#define CHECKSUM_MAGIC          0 	// Checksum CRC

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
	uint8_t pinButtonTurbo;
	uint8_t pinButtonReverse;
	uint8_t pinSliderLS;
	uint8_t pinSliderRS;
	ButtonLayout buttonLayout;
	ButtonLayoutRight buttonLayoutRight;
	SplashMode splashMode;
	SplashChoice splashChoice;
	uint8_t i2cSDAPin;
	uint8_t i2cSCLPin;
	int i2cBlock;
	uint32_t i2cSpeed;
	bool hasI2CDisplay;
	int displayI2CAddress;
	uint8_t displaySize;
	bool displayFlip;
	bool displayInvert;
	uint8_t turboShotCount; // Turbo
	uint8_t pinTurboLED;    // Turbo LED
	uint8_t pinReverseLED;    // Reverse LED
	uint8_t reverseActionUp;
	uint8_t reverseActionDown;
	uint8_t reverseActionLeft;
	uint8_t reverseActionRight;
	uint8_t i2cAnalog1219SDAPin;
	uint8_t i2cAnalog1219SCLPin;
	int i2cAnalog1219Block;
	uint32_t i2cAnalog1219Speed;
	uint8_t i2cAnalog1219Address;
	char boardVersion[32]; // 32-char limit to board name
	uint32_t checksum;
	bool hasBuzzerSpeaker;
	uint8_t buzzerPin;
	uint8_t buzzerVolume;
};

struct SplashImage {
	uint8_t data[16*64];
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
	char boardVersion[32]; // 32-char limit to board name
	uint32_t checksum;
};

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
	
	void setBoardOptions(BoardOptions);	// Board Options
	void setDefaultBoardOptions();
	BoardOptions getBoardOptions();
	
	void setSplashImage(SplashImage);
	void setDefaultSplashImage();
	SplashImage getSplashImage();

	void setLEDOptions(LEDOptions);		// LED Options
	void setDefaultLEDOptions();
	LEDOptions getLEDOptions();

	void SetConfigMode(bool); 			// Config Mode (on-boot)
	bool GetConfigMode();

	void SetGamepad(Gamepad *); 		// MPGS Gamepad Get/Set
	Gamepad * GetGamepad();

	void SetProcessedGamepad(Gamepad *); // MPGS Processed Gamepad Get/Set
	Gamepad * GetProcessedGamepad();

	void SetFeatureData(uint8_t *); 	// USB Feature Data Get/Set
	void ClearFeatureData();
	uint8_t * GetFeatureData();

	void ResetSettings(); 				// EEPROM Reset Feature

	int GetButtonLayout();
	int GetButtonLayoutRight();

	int GetSplashMode();
	int GetSplashChoice();

private:
	Storage() : gamepad(0) {
		EEPROM.start(); // init EEPROM
		initBoardOptions();
		initLEDOptions();
		initSplashImage();
	}
	void initBoardOptions();
	void initLEDOptions();
	void initSplashImage();
	bool CONFIG_MODE; 			// Config mode (boot)
	Gamepad * gamepad;    		// Gamepad data
	Gamepad * processedGamepad; // Gamepad with ONLY processed data
	BoardOptions boardOptions;
	LEDOptions ledOptions;
	uint8_t featureData[32]; // USB X-Input Feature Data
	SplashImage splashImage;
};

#endif
