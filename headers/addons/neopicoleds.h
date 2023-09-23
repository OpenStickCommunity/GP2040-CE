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
#include "AnimationStation.hpp"
#include "NeoPico.hpp"

#ifndef BOARD_LEDS_PIN
#define BOARD_LEDS_PIN -1
#endif

#ifndef BUTTON_LAYOUT
#define BUTTON_LAYOUT BUTTON_LAYOUT_ARCADE
#endif

#ifndef LED_FORMAT
#define LED_FORMAT LED_FORMAT_GRB
#endif

#ifndef LEDS_PER_PIXEL
#define LEDS_PER_PIXEL 1
#endif

#ifndef LEDS_BRIGHTNESS
#define LEDS_BRIGHTNESS 75
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

#ifndef LED_BRIGHTNESS_MAXIMUM
#define LED_BRIGHTNESS_MAXIMUM 128
#endif

#ifndef LED_BRIGHTNESS_STEPS
#define LED_BRIGHTNESS_STEPS 5
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

void configureAnimations(AnimationStation *as);
AnimationHotkey animationHotkeys(Gamepad *gamepad);
PixelMatrix createLedButtonLayout(ButtonLayout layout, int ledsPerPixel);
PixelMatrix createLedButtonLayout(ButtonLayout layout, std::vector<uint8_t> *positions);

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
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return NeoPicoLEDName; }
	void configureLEDs();
	uint32_t frame[100];
private:
	std::vector<uint8_t> * getLEDPositions(std::string button, std::vector<std::vector<uint8_t>> *positions);
	std::vector<std::vector<Pixel>> generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions);
	std::vector<std::vector<Pixel>> generatedLEDStickless(std::vector<std::vector<uint8_t>> *positions);
	std::vector<std::vector<Pixel>> generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions);
	std::vector<std::vector<Pixel>> generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions);
	std::vector<std::vector<Pixel>> createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount);
	uint8_t setupButtonPositions();
	const uint32_t intervalMS = 10;
	absolute_time_t nextRunTime;
	uint8_t ledCount;
	PixelMatrix matrix;
	NeoPico *neopico;
	InputMode inputMode; // HACK
	PLEDAnimationState animationState; // NeoPico can control the player LEDs
	NeoPicoPlayerLEDs * neoPLEDs = nullptr;
	AnimationStation as;
	std::map<std::string, int> buttonPositions;
	bool isFocusModeEnabled;
	bool focusModePrevState;
	bool turnOffWhenSuspended;
};

#endif
