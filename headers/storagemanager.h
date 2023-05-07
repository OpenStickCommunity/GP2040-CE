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

#include "mbedtls/rsa.h"

#define GAMEPAD_STORAGE_INDEX      		0    // 1024 bytes for gamepad options
#define BOARD_STORAGE_INDEX     		1024 //  512 bytes for hardware options
#define LED_STORAGE_INDEX       		1536 //  512 bytes for LED configuration
#define ANIMATION_STORAGE_INDEX 		2048 // 1024 bytes for LED animations
#define ADDON_STORAGE_INDEX             3072 // 1024 bytes for Add-Ons
#define PS4_STORAGE_INDEX               4096 // 2048 bytes for PS4 options
#define SPLASH_IMAGE_STORAGE_INDEX		6144 // 1032 bytes for Display Config


#define CHECKSUM_MAGIC          0 	// Checksum CRC
#define NOCHECKSUM_MAGIC        0xDEADBEEF // No checksum CRC

struct ButtonLayoutParams
{
	union {
		ButtonLayout layout;
		ButtonLayoutRight layoutRight;
	};
	int startX;
	int startY;
	int buttonRadius;
	int buttonPadding;
};

struct ButtonLayoutCustomOptions
{
	ButtonLayoutParams params;
	ButtonLayoutParams paramsRight;
}; // 76 bytes


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
	ButtonLayoutRight buttonLayoutRight;
	SplashMode splashMode;
	SplashChoice splashChoice;
	int splashDuration; // -1 = Always on
	uint8_t i2cSDAPin;
	uint8_t i2cSCLPin;
	int i2cBlock;
	uint32_t i2cSpeed;
	bool hasI2CDisplay;
	int displayI2CAddress;
	uint8_t displaySize;
	uint8_t displayFlip;
	bool displayInvert;
	int displaySaverTimeout;
	ButtonLayoutCustomOptions buttonLayoutCustomOptions;
	char boardVersion[32]; // 32-char limit to board name
	uint32_t checksum;
};

struct AddonOptions {
	uint8_t pinButtonTurbo;
	uint8_t pinButtonReverse;
	uint8_t pinSliderLS;
	uint8_t pinSliderRS;
	uint8_t pinSliderSOCDOne;
	uint8_t pinSliderSOCDTwo;
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
	uint8_t pinDualDirUp;    // Pins for Dual Directional Input
	uint8_t pinDualDirDown;
	uint8_t pinDualDirLeft;
	uint8_t pinDualDirRight;
	DpadMode dualDirDpadMode;    // LS/DP/RS
	uint8_t dualDirCombineMode; // Mix/Gamepad/Dual/None
	OnBoardLedMode onBoardLedMode;
	uint8_t analogAdcPinX;
	uint8_t analogAdcPinY;
	uint16_t bootselButtonMap;
	uint8_t extraButtonPin;
	uint32_t extraButtonMap;
	uint8_t buzzerPin;
	uint8_t buzzerVolume;
	uint8_t playerNumber;
	uint8_t shmupMode; // Turbo SHMUP Mode
	uint8_t shmupMixMode; // How we mix turbo and non-turbo buttons
	uint16_t shmupAlwaysOn1;
	uint16_t shmupAlwaysOn2;
	uint16_t shmupAlwaysOn3;
	uint16_t shmupAlwaysOn4;
	uint8_t pinShmupBtn1;
	uint8_t pinShmupBtn2;
	uint8_t pinShmupBtn3;
	uint8_t pinShmupBtn4;
	uint16_t shmupBtnMask1;
	uint16_t shmupBtnMask2;
	uint16_t shmupBtnMask3;
	uint16_t shmupBtnMask4;
	uint8_t pinShmupDial;
	SOCDMode sliderSOCDModeOne;
	SOCDMode sliderSOCDModeTwo;
	SOCDMode sliderSOCDModeDefault;
	uint8_t AnalogInputEnabled;
	uint8_t BoardLedAddonEnabled;
	uint8_t BootselButtonAddonEnabled;
	uint8_t BuzzerSpeakerAddonEnabled;
	uint8_t DualDirectionalInputEnabled;
	uint8_t ExtraButtonAddonEnabled;
	uint8_t I2CAnalog1219InputEnabled;
	//bool I2CDisplayAddonEnabled; // I2C is special case
	uint8_t JSliderInputEnabled;
	//bool NeoPicoLEDAddonEnabled; // NeoPico is special case
	//bool PlayerLEDAddonEnabled; // PlayerLED is special case
	uint8_t PlayerNumAddonEnabled;
	uint8_t PS4ModeAddonEnabled;
	uint8_t ReverseInputEnabled;
	uint8_t TurboInputEnabled;
	uint8_t SliderSOCDInputEnabled;
	uint32_t checksum;
};

struct SplashImage {
	uint8_t data[16*64];
	uint32_t checksum;
};

struct PS4Options {
	uint8_t serial[16];
	uint8_t signature[256];
	mbedtls_mpi_uint rsa_n[64];
	mbedtls_mpi_uint rsa_e[1];
	mbedtls_mpi_uint rsa_d[64];
	mbedtls_mpi_uint rsa_p[32];
	mbedtls_mpi_uint rsa_q[32];
	mbedtls_mpi_uint rsa_dp[32];
	mbedtls_mpi_uint rsa_dq[32];
	mbedtls_mpi_uint rsa_qp[32];
	mbedtls_mpi_uint rsa_rn[64];
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
	const BoardOptions& getBoardOptions() { return boardOptions; }

	void setPreviewBoardOptions(const BoardOptions&);	// Preview Board Options
	const BoardOptions& getPreviewBoardOptions() { return previewBoardOptions; }

	void setAddonOptions(AddonOptions); // Add-On Options
	const AddonOptions& getAddonOptions() { return addonOptions; }

	void setSplashImage(const SplashImage&);
	const SplashImage& getSplashImage() { return splashImage; }

	void setLEDOptions(LEDOptions);		// LED Options
	void setDefaultLEDOptions();
	const LEDOptions& getLEDOptions() { return ledOptions; }

	void savePS4Options();     // PS4 Options
	void setDefaultPS4Options();
	PS4Options * getPS4Options();

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

private:
	Storage() : gamepad(0) {
		EEPROM.start(); // init EEPROM
		initBoardOptions();
		initAddonOptions();
		initLEDOptions();
		initSplashImage();
		initPS4Options();

	}
	void initBoardOptions();
	void initPreviewBoardOptions();
	void initAddonOptions();
	void initLEDOptions();
	void initSplashImage();
	void setDefaultBoardOptions();
	void setDefaultAddonOptions();
	void setDefaultSplashImage();
	void initPS4Options();
	bool CONFIG_MODE; 			// Config mode (boot)
	Gamepad * gamepad;    		// Gamepad data
	Gamepad * processedGamepad; // Gamepad with ONLY processed data
	BoardOptions boardOptions;
	BoardOptions previewBoardOptions;
	AddonOptions addonOptions;
	LEDOptions ledOptions;
	PS4Options ps4Options;
	uint8_t featureData[32]; // USB X-Input Feature Data
	SplashImage splashImage;
};

#endif
