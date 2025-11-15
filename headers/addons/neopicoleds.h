/*
* SPDX-License-Identifier: MIT
* SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
*/

#ifndef _NEOPICOLEDS_H_
#define _NEOPICOLEDS_H_

// Pico Includes
#include <string>
#include <vector>
#include <map>

// GP2040 Includes
#include "helper.h"
#include "gamepad.h"
#include "gpaddon.h"
#include "storagemanager.h"

// MPGS
#include "BoardConfig.h"
#include "animationstation.h"
#include "specialmovesystem.h"
#include "NeoPico.h"

#include "playerleds.h"

#include "enums.pb.h"

#ifndef BOARD_LEDS_PIN
#define BOARD_LEDS_PIN -1
#endif

#ifndef BUTTON_LAYOUT
#define BUTTON_LAYOUT BUTTON_LAYOUT_ARCADE
#endif

#ifndef LED_FORMAT
#define LED_FORMAT LED_FORMAT_GRB
#endif

#ifndef LIGHT_DATA_SIZE_DEFAULT
#define LIGHT_DATA_SIZE_DEFAULT 0
#endif
#ifndef LIGHT_DATA_DEFAULT
#define LIGHT_DATA_DEFAULT 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_DEFAULT
#define LIGHT_DATA_NAME_DEFAULT ""
#endif
#ifndef LIGHT_DATA_SIZE_1
#define LIGHT_DATA_SIZE_1 0
#endif
#ifndef LIGHT_DATA_1
#define LIGHT_DATA_1 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_1
#define LIGHT_DATA_NAME_1 ""
#endif
#ifndef LIGHT_DATA_SIZE_2
#define LIGHT_DATA_SIZE_2 0
#endif
#ifndef LIGHT_DATA_2
#define LIGHT_DATA_2 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_2
#define LIGHT_DATA_NAME_2 ""
#endif
#ifndef LIGHT_DATA_SIZE_3
#define LIGHT_DATA_SIZE_3 0
#endif
#ifndef LIGHT_DATA_3
#define LIGHT_DATA_3 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_3
#define LIGHT_DATA_NAME_3 ""
#endif
#ifndef LIGHT_DATA_SIZE_4
#define LIGHT_DATA_SIZE_4 0
#endif
#ifndef LIGHT_DATA_4
#define LIGHT_DATA_4 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_4
#define LIGHT_DATA_NAME_4 ""
#endif
#ifndef LIGHT_DATA_SIZE_5
#define LIGHT_DATA_SIZE_5 0
#endif
#ifndef LIGHT_DATA_5
#define LIGHT_DATA_5 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_5
#define LIGHT_DATA_NAME_5 ""
#endif
#ifndef LIGHT_DATA_SIZE_6
#define LIGHT_DATA_SIZE_6 0
#endif
#ifndef LIGHT_DATA_6
#define LIGHT_DATA_6 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_6
#define LIGHT_DATA_NAME_6 ""
#endif
#ifndef LIGHT_DATA_SIZE_7
#define LIGHT_DATA_SIZE_7 0
#endif
#ifndef LIGHT_DATA_7
#define LIGHT_DATA_7 0,0,0,0,0,LightType::LightType_ActionButton
#endif
#ifndef LIGHT_DATA_NAME_7
#define LIGHT_DATA_NAME_7 ""
#endif

#ifndef LEDS_PER_PIXEL
#define LEDS_PER_PIXEL 1
#endif

#ifndef LEDS_BRIGHTNESS
#define LEDS_BRIGHTNESS -1
#endif

#ifndef LEDS_BASE_ANIMATION_INDEX
#define LEDS_BASE_ANIMATION_INDEX 2//1
#endif

#ifndef LEDS_STATIC_COLOR_INDEX
#define LEDS_STATIC_COLOR_INDEX 2
#endif

#ifndef LEDS_BUTTON_COLOR_INDEX
#define LEDS_BUTTON_COLOR_INDEX 1
#endif

#ifndef LEDS_THEME_INDEX
#define LEDS_THEME_INDEX 0
#endif

#ifndef LEDS_RAINBOW_CYCLE_TIME
#define LEDS_RAINBOW_CYCLE_TIME 40
#endif

#ifndef LEDS_CHASE_CYCLE_TIME
#define LEDS_CHASE_CYCLE_TIME 85
#endif

#ifndef LEDS_PRESS_COLOR_COOLDOWN_TIME
#define LEDS_PRESS_COLOR_COOLDOWN_TIME 0
#endif

#ifndef LED_BRIGHTNESS_MAXIMUM
#define LED_BRIGHTNESS_MAXIMUM 128
#endif

#ifndef LEDS_DPAD_LEFT
#define LEDS_DPAD_LEFT  -1
#endif

#ifndef LEDS_DPAD_DOWN
#define LEDS_DPAD_DOWN  -1
#endif

#ifndef LEDS_DPAD_RIGHT
#define LEDS_DPAD_RIGHT -1
#endif

#ifndef LEDS_DPAD_UP
#define LEDS_DPAD_UP    -1
#endif

#ifndef LEDS_BUTTON_B1
#define LEDS_BUTTON_B1  -1
#endif

#ifndef LEDS_BUTTON_B2
#define LEDS_BUTTON_B2  -1
#endif

#ifndef LEDS_BUTTON_B3
#define LEDS_BUTTON_B3  -1
#endif

#ifndef LEDS_BUTTON_B4
#define LEDS_BUTTON_B4  -1
#endif

#ifndef LEDS_BUTTON_R1
#define LEDS_BUTTON_R1  -1
#endif

#ifndef LEDS_BUTTON_L1
#define LEDS_BUTTON_L1  -1
#endif

#ifndef LEDS_BUTTON_L2
#define LEDS_BUTTON_L2  -1
#endif

#ifndef LEDS_BUTTON_R2
#define LEDS_BUTTON_R2  -1
#endif

#ifndef LEDS_BUTTON_S1
#define LEDS_BUTTON_S1  -1
#endif

#ifndef LEDS_BUTTON_S2
#define LEDS_BUTTON_S2  -1
#endif

#ifndef LEDS_BUTTON_L3
#define LEDS_BUTTON_L3  -1
#endif

#ifndef LEDS_BUTTON_R3
#define LEDS_BUTTON_R3  -1
#endif

#ifndef LEDS_BUTTON_A1
#define LEDS_BUTTON_A1  -1
#endif

#ifndef LEDS_BUTTON_A2
#define LEDS_BUTTON_A2  -1
#endif

#ifndef LEDS_TURN_OFF_WHEN_SUSPENDED
#define LEDS_TURN_OFF_WHEN_SUSPENDED 0
#endif

// Neo Pixel needs to tie into PlayerLEDS led Levels
class NeoPicoPlayerLEDs : public PlayerLEDs
{
public:
    virtual void setup(){}
    virtual void display(){}
    uint16_t * getLedLevels() { return ledLevels; }
};

#define NeoPicoLEDName "NeoPicoLED"

// NeoPico LED Addon
class NeoPicoLEDAddon : public GPAddon {
public:

	//GP Addon functions
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
   	virtual void postprocess(bool sent) {}
    virtual void reinit() {}
	virtual std::string name() { return NeoPicoLEDName; }

	static void AssignLedPreset(const unsigned char* data, int32_t dataSize);
	static void RestartLedSystem() { bRestartLeds = true; }

	uint32_t frame[FRAME_MAX];
private:

	void decompressSettings();

	void configureLEDs();

	GamepadHotkey ProcessAnimationHotkeys(Gamepad *gamepad);

	//Legacy setup functions
	void generateLegacyIndividualLight(int firstLedIndex, int xCoord, int yCoord, uint8_t ledsPerPixel, GpioAction actionButton);
	void generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel);
	void generatedLEDStickless(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel);
	void generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel);
	void generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel);
	void createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount);
	uint8_t setupButtonPositions();

	//New co-ordinated setup
	void GenerateLights();

	//Controls the actual lights on the board. Writes out state each frame
	NeoPico neopico;

	//Classes to control the player LEDS
	PLEDAnimationState animationState; // NeoPico can control the player LEDs
	NeoPicoPlayerLEDs * neoPLEDs = nullptr;

	//Data representation of the lights
	Lights RGBLights;

	//Animation class. Handles idle animations, special move animations and pressed button effects
	class AnimationStation AnimStation;

	const uint32_t intervalMS = 25;
	absolute_time_t nextRunTime;
	absolute_time_t lastRunTime;
	uint8_t ledCount;
	InputMode inputMode; // HACK
	std::map<std::string, int> buttonPositions;
	bool turnOffWhenSuspended;

	bool bHasSetupNeoPico = false;

	//Webconfig/testing
	static bool bRestartLeds;
};

#endif
