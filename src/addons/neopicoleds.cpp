/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "AnimationStation.hpp"
#include "AnimationStorage.hpp"
#include "NeoPico.hpp"
#include "Pixel.hpp"
#include "PlayerLEDs.h"
#include "gp2040.h"
#include "addons/neopicoleds.h"
#include "addons/pleds.h"
#include "themes.h"
#include "usbdriver.h"
#include "enums.h"
#include "helper.h"

const std::string BUTTON_LABEL_UP = "Up";
const std::string BUTTON_LABEL_DOWN = "Down";
const std::string BUTTON_LABEL_LEFT = "Left";
const std::string BUTTON_LABEL_RIGHT = "Right";
const std::string BUTTON_LABEL_B1 = "B1";
const std::string BUTTON_LABEL_B2 = "B2";
const std::string BUTTON_LABEL_B3 = "B3";
const std::string BUTTON_LABEL_B4 = "B4";
const std::string BUTTON_LABEL_L1 = "L1";
const std::string BUTTON_LABEL_R1 = "R1";
const std::string BUTTON_LABEL_L2 = "L2";
const std::string BUTTON_LABEL_R2 = "R2";
const std::string BUTTON_LABEL_S1 = "S1";
const std::string BUTTON_LABEL_S2 = "S2";
const std::string BUTTON_LABEL_L3 = "L3";
const std::string BUTTON_LABEL_R3 = "R3";
const std::string BUTTON_LABEL_A1 = "A1";
const std::string BUTTON_LABEL_A2 = "A2";

static std::vector<uint8_t> EMPTY_VECTOR;

uint32_t rgbPLEDValues[4];

// Move to Proto Enums
typedef enum
{
	XINPUT_PLED_OFF       = 0x00, // All off
	XINPUT_PLED_BLINKALL  = 0x01, // All blinking
	XINPUT_PLED_FLASH1    = 0x02, // 1 flashes, then on
	XINPUT_PLED_FLASH2    = 0x03, // 2 flashes, then on
	XINPUT_PLED_FLASH3    = 0x04, // 3 flashes, then on
	XINPUT_PLED_FLASH4    = 0x05, // 4 flashes, then on
	XINPUT_PLED_ON1       = 0x06, // 1 on
	XINPUT_PLED_ON2       = 0x07, // 2 on
	XINPUT_PLED_ON3       = 0x08, // 3 on
	XINPUT_PLED_ON4       = 0x09, // 4 on
	XINPUT_PLED_ROTATE    = 0x0A, // Rotating (e.g. 1-2-4-3)
	XINPUT_PLED_BLINK     = 0x0B, // Blinking*
	XINPUT_PLED_SLOWBLINK = 0x0C, // Slow blinking*
	XINPUT_PLED_ALTERNATE = 0x0D, // Alternating (e.g. 1+4-2+3), then back to previous*
} XInputPLEDPattern;

// TODO: Make this a helper function
// Animation Helper for Player LEDs
PLEDAnimationState getXInputAnimationNEOPICO(uint16_t ledState)
{
	PLEDAnimationState animationState =
	{
		.state = 0,
		.animation = PLED_ANIM_NONE,
		.speed = PLED_SPEED_OFF,
	};

	switch (ledState)
	{
		case XINPUT_PLED_BLINKALL:
		case XINPUT_PLED_ROTATE:
		case XINPUT_PLED_BLINK:
		case XINPUT_PLED_SLOWBLINK:
		case XINPUT_PLED_ALTERNATE:
			animationState.state = (PLED_STATE_LED1 | PLED_STATE_LED2 | PLED_STATE_LED3 | PLED_STATE_LED4);
			animationState.animation = PLED_ANIM_BLINK;
			animationState.speed = PLED_SPEED_FAST;
			break;

		case XINPUT_PLED_FLASH1:
		case XINPUT_PLED_ON1:
			animationState.state = PLED_STATE_LED1;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH2:
		case XINPUT_PLED_ON2:
			animationState.state = PLED_STATE_LED2;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH3:
		case XINPUT_PLED_ON3:
			animationState.state = PLED_STATE_LED3;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		case XINPUT_PLED_FLASH4:
		case XINPUT_PLED_ON4:
			animationState.state = PLED_STATE_LED4;
			animationState.animation = PLED_ANIM_SOLID;
			animationState.speed = PLED_SPEED_OFF;
			break;

		default:
			break;
	}

	return animationState;
}

PLEDAnimationState getXBoneAnimationNEOPICO(Gamepad * gamepad)
{
	PLEDAnimationState animationState =
	{
		.state = (PLED_STATE_LED1 | PLED_STATE_LED2 | PLED_STATE_LED3 | PLED_STATE_LED4),
		.animation = PLED_ANIM_OFF
	};

	if ( gamepad->auxState.playerID.ledValue == 1 ) { 
		animationState.animation = PLED_ANIM_SOLID;
	}

	return animationState;
}

PLEDAnimationState getPS3AnimationNEOPICO(uint16_t ledState)
{
	const uint8_t ps3LEDs[10][4] = {
		{ 0x01, 0x00, 0x00, 0x00 },
		{ 0x00, 0x01, 0x00, 0x00 },
		{ 0x00, 0x00, 0x01, 0x00 },
		{ 0x00, 0x00, 0x00, 0x01 },
		{ 0x01, 0x00, 0x00, 0x01 },
		{ 0x00, 0x01, 0x00, 0x01 },
		{ 0x00, 0x00, 0x01, 0x01 },
		{ 0x01, 0x00, 0x01, 0x01 },
		{ 0x00, 0x01, 0x01, 0x01 },
		{ 0x01, 0x01, 0x01, 0x01 }
	};

	PLEDAnimationState animationState =
	{
		.state = 0,
		.animation = PLED_ANIM_NONE,
		.speed = PLED_SPEED_OFF,
	};

	if (ledState != 0) {
		uint8_t ledNumber = ledState & 0x0F;
		if (ps3LEDs[ledNumber-1][0] == 0x01) animationState.state |= PLED_STATE_LED1;
		if (ps3LEDs[ledNumber-1][1] == 0x01) animationState.state |= PLED_STATE_LED2;
		if (ps3LEDs[ledNumber-1][2] == 0x01) animationState.state |= PLED_STATE_LED3;
		if (ps3LEDs[ledNumber-1][3] == 0x01) animationState.state |= PLED_STATE_LED4;
	}

	if (animationState.state != 0) {
		animationState.animation = PLED_ANIM_SOLID;
		animationState.speed = PLED_SPEED_OFF;
	} else {
		animationState.state = 0;
		animationState.animation = PLED_ANIM_OFF;
		animationState.speed = PLED_SPEED_OFF;
	}

	return animationState;
}

PLEDAnimationState getPS4AnimationNEOPICO(uint32_t flashOn, uint32_t flashOff)
{
	PLEDAnimationState animationState =
	{
		.state = (PLED_STATE_LED1 | PLED_STATE_LED2 | PLED_STATE_LED3 | PLED_STATE_LED4),
		.animation = PLED_ANIM_SOLID,
		.speed = PLED_SPEED_OFF,
	};

	if (flashOn > 0 || flashOff > 0) {
		animationState.animation = PLED_ANIM_BLINK_CUSTOM;
		animationState.speedOn = flashOn;
		animationState.speedOff = flashOff;
	}

	return animationState;
}

bool NeoPicoLEDAddon::available() {
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	return isValidPin(ledOptions.dataPin);
}

void NeoPicoLEDAddon::setup()
{
	// Set Default LED Options
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	turnOffWhenSuspended = ledOptions.turnOffWhenSuspended;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	gamepad->auxState.playerID.enabled = true;
	gamepad->auxState.sensors.statusLight.enabled = true;

	if ( ledOptions.pledType == PLED_TYPE_RGB ) {
		neoPLEDs = new NeoPicoPlayerLEDs();
	}

	neopico = nullptr; // set neopico to null

	// Create a dummy Neo Pico for the initial configuration
	neopico = new NeoPico(-1, 0);
	configureLEDs();

	nextRunTime = make_timeout_time_ms(0); // Reset timeout
}

void NeoPicoLEDAddon::process()
{
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	if (!isValidPin(ledOptions.dataPin) || !time_reached(this->nextRunTime))
		return;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	AnimationHotkey action = animationHotkeys(gamepad);
	if (ledOptions.pledType == PLED_TYPE_RGB) {
		inputMode = gamepad->getOptions().inputMode; // HACK
		if (gamepad->auxState.playerID.enabled && gamepad->auxState.playerID.active) {
			switch (inputMode) {
				case INPUT_MODE_XINPUT:
					animationState = getXInputAnimationNEOPICO(gamepad->auxState.playerID.ledValue);
					break;
				case INPUT_MODE_PS3:
					animationState = getPS3AnimationNEOPICO(gamepad->auxState.playerID.ledValue);
					break;
				case INPUT_MODE_PS4:
				case INPUT_MODE_PS5:
					animationState = getPS4AnimationNEOPICO(gamepad->auxState.playerID.ledBlinkOn, gamepad->auxState.playerID.ledBlinkOff);
					break;
				case INPUT_MODE_XBONE:
					animationState = getXBoneAnimationNEOPICO(gamepad);
					break;
				default:
					break;
			}
		}

		if (neoPLEDs != nullptr && animationState.animation != PLED_ANIM_NONE) {
			neoPLEDs->animate(animationState);
		}
	}

	if ( action != HOTKEY_LEDS_NONE ) {
		as.HandleEvent(action);
	}

	uint32_t buttonState = gamepad->state.dpad << 16 | gamepad->state.buttons;
	vector<Pixel> pressed;
	for (auto row : matrix.pixels)
	{
		for (auto pixel : row)
		{
			if (buttonState & pixel.mask)
				pressed.push_back(pixel);
		}
	}
	if (pressed.size() > 0)
		as.HandlePressed(pressed);
	else
		as.ClearPressed();

	as.Animate();

	if (turnOffWhenSuspended && get_usb_suspended()) {
		as.DimBrightnessTo0();
	} else {
		as.SetBrightness(AnimationStation::GetBrightness());
	}

	as.ApplyBrightness(frame);

	// Apply the player LEDs to our first 4 leds if we're in NEOPIXEL mode
	if (ledOptions.pledType == PLED_TYPE_RGB) {
		LEDOptions & ledOptions = Storage::getInstance().getLedOptions();
		int32_t pledIndexes[] = { ledOptions.pledIndex1, ledOptions.pledIndex2, ledOptions.pledIndex3, ledOptions.pledIndex4 };
		for (int i = 0; i < PLED_COUNT; i++) {
			if (pledIndexes[i] < 0)
				continue;

			float level = (static_cast<float>(PLED_MAX_LEVEL - neoPLEDs->getLedLevels()[i]) / static_cast<float>(PLED_MAX_LEVEL));
			float brightness = as.GetBrightnessX() * level;
			if (gamepad->auxState.sensors.statusLight.enabled && gamepad->auxState.sensors.statusLight.active) {
				rgbPLEDValues[i] = (RGB(gamepad->auxState.sensors.statusLight.color.red, gamepad->auxState.sensors.statusLight.color.green, gamepad->auxState.sensors.statusLight.color.blue)).value(neopico->GetFormat(), brightness);
			} else {
				rgbPLEDValues[i] = ((RGB)ledOptions.pledColor).value(neopico->GetFormat(), brightness);
			}
			frame[pledIndexes[i]] = rgbPLEDValues[i];
		}
	}

	neopico->SetFrame(frame);
	neopico->Show();
	AnimationStore.save();

	this->nextRunTime = make_timeout_time_ms(NeoPicoLEDAddon::intervalMS);
}

std::vector<uint8_t> * NeoPicoLEDAddon::getLEDPositions(string button, std::vector<std::vector<uint8_t>> *positions)
{
	int buttonPosition = buttonPositions[button];
	if (buttonPosition < 0)
		return &EMPTY_VECTOR;
	else
		return &positions->at(buttonPosition);
}

// Macro for Pixel() declarations
#define PIXEL(BUTTON, MASK) \
	Pixel(buttonPositions[BUTTON], MASK, *getLEDPositions(BUTTON, positions))

/**
 * @brief Create an LED layout using a 2x4 matrix.
 */
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			PIXEL(BUTTON_LABEL_B3, GAMEPAD_MASK_B3),
			PIXEL(BUTTON_LABEL_B1, GAMEPAD_MASK_B1),
		},
		{
			PIXEL(BUTTON_LABEL_B4, GAMEPAD_MASK_B4),
			PIXEL(BUTTON_LABEL_B2, GAMEPAD_MASK_B2),
		},
		{
			PIXEL(BUTTON_LABEL_R1, GAMEPAD_MASK_R1),
			PIXEL(BUTTON_LABEL_R2, GAMEPAD_MASK_R2),
		},
		{
			PIXEL(BUTTON_LABEL_L1, GAMEPAD_MASK_L1),
			PIXEL(BUTTON_LABEL_L2, GAMEPAD_MASK_L2),
		},
		{
			PIXEL(BUTTON_LABEL_LEFT, GAMEPAD_MASK_DL),
			PIXEL(BUTTON_LABEL_DOWN, GAMEPAD_MASK_DD),
			PIXEL(BUTTON_LABEL_RIGHT, GAMEPAD_MASK_DR),
			PIXEL(BUTTON_LABEL_UP, GAMEPAD_MASK_DU),
			PIXEL(BUTTON_LABEL_S1, GAMEPAD_MASK_S1),
			PIXEL(BUTTON_LABEL_S2, GAMEPAD_MASK_S2),
			PIXEL(BUTTON_LABEL_L3, GAMEPAD_MASK_L3),
			PIXEL(BUTTON_LABEL_R3, GAMEPAD_MASK_R3),
			PIXEL(BUTTON_LABEL_A1, GAMEPAD_MASK_A1),
			PIXEL(BUTTON_LABEL_A2, GAMEPAD_MASK_A2),
		},
	};

	return pixels;
}

/**
 * @brief Create an LED layout using a 3x8 matrix.
 */
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDStickless(vector<vector<uint8_t>> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			PIXEL(BUTTON_LABEL_LEFT, GAMEPAD_MASK_DL),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_DOWN, GAMEPAD_MASK_DD),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_RIGHT, GAMEPAD_MASK_DR),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_UP, GAMEPAD_MASK_DU),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_B3, GAMEPAD_MASK_B3),
			PIXEL(BUTTON_LABEL_B1, GAMEPAD_MASK_B1),
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_B4, GAMEPAD_MASK_B4),
			PIXEL(BUTTON_LABEL_B2, GAMEPAD_MASK_B2),
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_R1, GAMEPAD_MASK_R1),
			PIXEL(BUTTON_LABEL_R2, GAMEPAD_MASK_R2),
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_L1, GAMEPAD_MASK_L1),
			PIXEL(BUTTON_LABEL_L2, GAMEPAD_MASK_L2),
			NO_PIXEL,
		},
		{
			PIXEL(BUTTON_LABEL_S1, GAMEPAD_MASK_S1),
			PIXEL(BUTTON_LABEL_S2, GAMEPAD_MASK_S2),
			PIXEL(BUTTON_LABEL_L3, GAMEPAD_MASK_L3),
			PIXEL(BUTTON_LABEL_R3, GAMEPAD_MASK_R3),
			PIXEL(BUTTON_LABEL_A1, GAMEPAD_MASK_A1),
			PIXEL(BUTTON_LABEL_A2, GAMEPAD_MASK_A2),
		},
	};

	return pixels;
}

/**
 * @brief Create an LED layout using a 2x7 matrix.
 */
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			NO_PIXEL,
			PIXEL(BUTTON_LABEL_LEFT, GAMEPAD_MASK_DL),
		},
		{
			PIXEL(BUTTON_LABEL_UP, GAMEPAD_MASK_DU),
			PIXEL(BUTTON_LABEL_DOWN, GAMEPAD_MASK_DD),
		},
		{
			NO_PIXEL,
			PIXEL(BUTTON_LABEL_RIGHT, GAMEPAD_MASK_DR),
		},
		{
			PIXEL(BUTTON_LABEL_B3, GAMEPAD_MASK_B3),
			PIXEL(BUTTON_LABEL_B1, GAMEPAD_MASK_B1),
		},
		{
			PIXEL(BUTTON_LABEL_B4, GAMEPAD_MASK_B4),
			PIXEL(BUTTON_LABEL_B2, GAMEPAD_MASK_B2),
		},
		{
			PIXEL(BUTTON_LABEL_R1, GAMEPAD_MASK_R1),
			PIXEL(BUTTON_LABEL_R2, GAMEPAD_MASK_R2),
		},
		{
			PIXEL(BUTTON_LABEL_L1, GAMEPAD_MASK_L1),
			PIXEL(BUTTON_LABEL_L2, GAMEPAD_MASK_L2),
		},
		{
			PIXEL(BUTTON_LABEL_S1, GAMEPAD_MASK_S1),
			PIXEL(BUTTON_LABEL_S2, GAMEPAD_MASK_S2),
			PIXEL(BUTTON_LABEL_L3, GAMEPAD_MASK_L3),
			PIXEL(BUTTON_LABEL_R3, GAMEPAD_MASK_R3),
			PIXEL(BUTTON_LABEL_A1, GAMEPAD_MASK_A1),
			PIXEL(BUTTON_LABEL_A2, GAMEPAD_MASK_A2),
		},
	};

	return pixels;
}

/**
 * @brief Create an LED layout using a 2x7 matrix for the mirrored Fightboard.
 */
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			PIXEL(BUTTON_LABEL_L1, GAMEPAD_MASK_L1),
			PIXEL(BUTTON_LABEL_L2, GAMEPAD_MASK_L2),
		},
		{
			PIXEL(BUTTON_LABEL_R1, GAMEPAD_MASK_R1),
			PIXEL(BUTTON_LABEL_R2, GAMEPAD_MASK_R2),
		},
		{
			PIXEL(BUTTON_LABEL_B4, GAMEPAD_MASK_B4),
			PIXEL(BUTTON_LABEL_B2, GAMEPAD_MASK_B2),
		},
		{
			PIXEL(BUTTON_LABEL_B3, GAMEPAD_MASK_B3),
			PIXEL(BUTTON_LABEL_B1, GAMEPAD_MASK_B1),
		},
		{
			NO_PIXEL,
			PIXEL(BUTTON_LABEL_LEFT, GAMEPAD_MASK_DL),
		},
		{
			PIXEL(BUTTON_LABEL_UP, GAMEPAD_MASK_DU),
			PIXEL(BUTTON_LABEL_DOWN, GAMEPAD_MASK_DD),
		},
		{
			NO_PIXEL,
			PIXEL(BUTTON_LABEL_RIGHT, GAMEPAD_MASK_DR),
		},
		{
			PIXEL(BUTTON_LABEL_S1, GAMEPAD_MASK_S1),
			PIXEL(BUTTON_LABEL_S2, GAMEPAD_MASK_S2),
			PIXEL(BUTTON_LABEL_L3, GAMEPAD_MASK_L3),
			PIXEL(BUTTON_LABEL_R3, GAMEPAD_MASK_R3),
			PIXEL(BUTTON_LABEL_A1, GAMEPAD_MASK_A1),
			PIXEL(BUTTON_LABEL_A2, GAMEPAD_MASK_A2),
		},
	};

	return pixels;
}

std::vector<std::vector<Pixel>> NeoPicoLEDAddon::createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount)
{
	vector<vector<uint8_t>> positions(ledButtonCount);
	for (int i = 0; i != ledButtonCount; i++)
	{
		positions[i].resize(ledsPerPixel);
		for (int l = 0; l != ledsPerPixel; l++)
			positions[i][l] = (i * ledsPerPixel) + l;
	}

	switch (static_cast<ButtonLayout>(layout))
	{
		case BUTTON_LAYOUT_STICKLESS:
		case BUTTON_LAYOUT_OPENCORE0WASDA:
		case BUTTON_LAYOUT_STICKLESS_13:
		case BUTTON_LAYOUT_STICKLESS_14:
		case BUTTON_LAYOUT_STICKLESS_16:
		case BUTTON_LAYOUT_STICKLESS_R16:
		case BUTTON_LAYOUT_BOARD_DEFINED_A:
			return generatedLEDStickless(&positions);
		case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
			return generatedLEDWasdFBM(&positions);
		case BUTTON_LAYOUT_BUTTONS_ANGLED:
		case BUTTON_LAYOUT_FIGHTBOARD_STICK:
			return generatedLEDWasd(&positions);
		case BUTTON_LAYOUT_BLANKA:
		case BUTTON_LAYOUT_BUTTONS_BASIC:
		case BUTTON_LAYOUT_KEYBOARD_ANGLED:
		case BUTTON_LAYOUT_KEYBOARDA:
		case BUTTON_LAYOUT_DANCEPADA:
		case BUTTON_LAYOUT_TWINSTICKA:
		case BUTTON_LAYOUT_ARCADE:
		case BUTTON_LAYOUT_VLXA:
		default:
			return generatedLEDButtons(&positions);
	}

	assert(false);
	return std::vector<std::vector<Pixel>>();
}

uint8_t NeoPicoLEDAddon::setupButtonPositions()
{
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	buttonPositions.clear();
	buttonPositions.emplace(BUTTON_LABEL_UP, ledOptions.indexUp);
	buttonPositions.emplace(BUTTON_LABEL_DOWN, ledOptions.indexDown);
	buttonPositions.emplace(BUTTON_LABEL_LEFT, ledOptions.indexLeft);
	buttonPositions.emplace(BUTTON_LABEL_RIGHT, ledOptions.indexRight);
	buttonPositions.emplace(BUTTON_LABEL_B1, ledOptions.indexB1);
	buttonPositions.emplace(BUTTON_LABEL_B2, ledOptions.indexB2);
	buttonPositions.emplace(BUTTON_LABEL_B3, ledOptions.indexB3);
	buttonPositions.emplace(BUTTON_LABEL_B4, ledOptions.indexB4);
	buttonPositions.emplace(BUTTON_LABEL_L1, ledOptions.indexL1);
	buttonPositions.emplace(BUTTON_LABEL_R1, ledOptions.indexR1);
	buttonPositions.emplace(BUTTON_LABEL_L2, ledOptions.indexL2);
	buttonPositions.emplace(BUTTON_LABEL_R2, ledOptions.indexR2);
	buttonPositions.emplace(BUTTON_LABEL_S1, ledOptions.indexS1);
	buttonPositions.emplace(BUTTON_LABEL_S2, ledOptions.indexS2);
	buttonPositions.emplace(BUTTON_LABEL_L3, ledOptions.indexL3);
	buttonPositions.emplace(BUTTON_LABEL_R3, ledOptions.indexR3);
	buttonPositions.emplace(BUTTON_LABEL_A1, ledOptions.indexA1);
	buttonPositions.emplace(BUTTON_LABEL_A2, ledOptions.indexA2);
	uint8_t buttonCount = 0;
	for (auto const& buttonPosition : buttonPositions)
	{
		if (buttonPosition.second > -1)
			buttonCount++;
	}

	return buttonCount;
}

void NeoPicoLEDAddon::configureLEDs()
{
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	uint8_t buttonCount = setupButtonPositions();
	vector<vector<Pixel>> pixels = createLEDLayout(static_cast<ButtonLayout>(ledOptions.ledLayout), ledOptions.ledsPerButton, buttonCount);
	matrix.setup(pixels, ledOptions.ledsPerButton);
	ledCount = matrix.getLedCount();
	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0)
		ledCount += PLED_COUNT;

	// Remove the old neopico (config can call this)
	delete neopico;
	neopico = new NeoPico(ledOptions.dataPin, ledCount, static_cast<LEDFormat>(ledOptions.ledFormat));
	neopico->Off();

	Animation::format = static_cast<LEDFormat>(ledOptions.ledFormat);
	as.ConfigureBrightness(ledOptions.brightnessMaximum, ledOptions.brightnessSteps);
	AnimationOptions animationOptions = AnimationStore.getAnimationOptions();
	addStaticThemes(ledOptions, animationOptions);
	as.SetOptions(animationOptions);
	as.SetMatrix(matrix);
	as.SetMode(as.options.baseAnimationIndex);
}

AnimationHotkey animationHotkeys(Gamepad *gamepad)
{
	AnimationHotkey action = HOTKEY_LEDS_NONE;

	if (gamepad->pressedS1() && gamepad->pressedS2())
	{
		if (gamepad->pressedB3())
		{
			action = HOTKEY_LEDS_ANIMATION_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedB1())
		{
			action = HOTKEY_LEDS_ANIMATION_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedB4())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B4 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedB2())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedR1())
		{
			action = HOTKEY_LEDS_PARAMETER_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedR2())
		{
			action = HOTKEY_LEDS_PARAMETER_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedL1())
		{
			action = HOTKEY_LEDS_PRESS_PARAMETER_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedL2())
		{
			action = HOTKEY_LEDS_PRESS_PARAMETER_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedL3())
		{
			action = HOTKEY_LEDS_FADETIME_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedR3())
		{
			action = HOTKEY_LEDS_FADETIME_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
	}

	return action;
}
