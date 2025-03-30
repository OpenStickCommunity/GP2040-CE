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
#include "NeoPico.h"

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

#ifndef LEDS_PRESS_COLOR_COOLDOWN_TIME
#define LEDS_PRESS_COLOR_COOLDOWN_TIME 0
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

#ifndef CASE_RGB_TYPE
#define CASE_RGB_TYPE CASE_RGB_TYPE_NONE
#endif

#ifndef CASE_RGB_INDEX
#define CASE_RGB_INDEX -1
#endif

#ifndef CASE_RGB_COUNT
#define CASE_RGB_COUNT 0
#endif

#ifndef AMBIENT_LIGHT_EFFECT
#define AMBIENT_LIGHT_EFFECT AL_CUSTOM_EFFECT_STATIC_COLOR
#endif

#ifndef AMBIENT_STATIC_COLOR_BRIGHTNESS
#define AMBIENT_STATIC_COLOR_BRIGHTNESS 1.00f
#endif

#ifndef AMBIENT_GRADIENT_COLOR_BRIGHTNESS
#define AMBIENT_GRADIENT_COLOR_BRIGHTNESS 1.00f
#endif

#ifndef AMBIENT_CHASE_COLOR_BRIGHTNESS
#define AMBIENT_CHASE_COLOR_BRIGHTNESS 1.00f
#endif

#ifndef AMBIENT_CUSTOM_THEME_BRIGHTNESS
#define AMBIENT_CUSTOM_THEME_BRIGHTNESS 1.00f
#endif

#ifndef AMBIENT_GRADIENT_SPEED
#define AMBIENT_GRADIENT_SPEED 2
#endif

#ifndef AMBIENT_CHASE_SPEED
#define AMBIENT_CHASE_SPEED 100
#endif

#ifndef AMBIENT_BREATH_SPEED
#define AMBIENT_BREATH_SPEED 0.01f
#endif

#ifndef AMBIENT_CUSTOM_THEME
#define AMBIENT_CUSTOM_THEME 0
#endif

#ifndef AMBIENT_STATIC_COLOR
#define AMBIENT_STATIC_COLOR ANIMATION_COLOR_PURPLE
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
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return NeoPicoLEDName; }    
	void ambientLightLinkage(); 
    
private:
    std::vector<uint8_t> * getLEDPositions(std::string button, std::vector<std::vector<uint8_t>> *positions);
    std::vector<std::vector<Pixel>> generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions);
    std::vector<std::vector<Pixel>> generatedLEDStickless(std::vector<std::vector<uint8_t>> *positions);
    std::vector<std::vector<Pixel>> generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions);
    std::vector<std::vector<Pixel>> generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions);
    std::vector<std::vector<Pixel>> createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount);
    uint8_t setupButtonPositions();
    GamepadHotkey animationHotkeys(Gamepad *gamepad);
    void ambientHotkeys(Gamepad *gamepad);
    void ambientLightCustom();
    const uint32_t intervalMS = 10;
    absolute_time_t nextRunTime;
    int ledCount;
    int buttonLedCount;
    PixelMatrix matrix;
    NeoPico neopico;
    PLEDAnimationState animationState; // NeoPico can control the player LEDs
    NeoPicoPlayerLEDs * neoPLEDs = nullptr;
    AnimationStation as;
    std::map<std::string, int> buttonPositions;
    PLEDType ledType;
    GamepadHotkey lastAmbientAction;
    uint32_t frame[100];

    // Ambient neopico leds
	float alBrightnessBreathX;
	uint8_t breathLedEffectCycle;
	bool alReverse;
	int alCurrentFrame;
	int alFrameToRGB;
	int alFrameSpeed;
    RGB ambientLight;
	absolute_time_t nextRunTimeAmbientLight;
    uint8_t chaseLightIndex;
    uint8_t chaseLightMaxIndexPos;

    uint8_t multipleOfButtonLedsCount;
    uint8_t remainderOfButtonLedsCount;

    uint8_t alLinkageStartIndex;
};

#endif
