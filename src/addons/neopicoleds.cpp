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

#include "enums.h"
#include "helper.h"

static std::vector<uint8_t> EMPTY_VECTOR;

uint32_t rgbPLEDValues[4];

// TODO: Make this a helper function
// Animation Helper for Player LEDs
PLEDAnimationState getXInputAnimationNEOPICO(uint8_t *data)
{
	PLEDAnimationState animationState =
	{
		.state = 0,
		.animation = PLED_ANIM_NONE,
		.speed = PLED_SPEED_OFF,
	};

	// Check first byte for LED payload
	if (data[0] == 0x01)
	{
		switch (data[2])
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
	}

	return animationState;
}

bool NeoPicoLEDAddon::available() {
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	return ledOptions.dataPin != -1;
}

void NeoPicoLEDAddon::setup()
{
	// Set Default LED Options
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	if (!ledOptions.useUserDefinedLEDs) {
		Storage::getInstance().setDefaultLEDOptions();
	}

	if ( PLED_TYPE == PLED_TYPE_RGB ) {
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
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	if (ledOptions.dataPin < 0 || !time_reached(this->nextRunTime))
		return;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	uint8_t * featureData = Storage::getInstance().GetFeatureData();
	AnimationHotkey action = animationHotkeys(gamepad);
	if (PLED_TYPE == PLED_TYPE_RGB) {
		inputMode = gamepad->options.inputMode; // HACK
		switch (gamepad->options.inputMode) {
			case INPUT_MODE_XINPUT:
				animationState = getXInputAnimationNEOPICO(featureData);
				if (neoPLEDs != nullptr && animationState.animation != PLED_ANIM_NONE)
					neoPLEDs->animate(animationState);
				break;
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
	as.ApplyBrightness(frame);

	// Apply the player LEDs to our first 4 leds if we're in NEOPIXEL mode
	if (PLED_TYPE == PLED_TYPE_RGB) {
		switch (inputMode) { // HACK
			case INPUT_MODE_XINPUT:
				for (int i = 0; i < PLED_COUNT; i++) {
					float level = (static_cast<float>(PLED_MAX_LEVEL - neoPLEDs->getLedLevels()[i]) / static_cast<float>(PLED_MAX_LEVEL));
					float brightness = as.GetBrightnessX() * level;
					rgbPLEDValues[i] = ((RGB)ColorGreen).value(neopico->GetFormat(), brightness);
					frame[PLED_PINS[i]] = rgbPLEDValues[i];
				}
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

	switch (layout)
	{
		case BUTTON_LAYOUT_BLANKA:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_BUTTONS_BASIC:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_KEYBOARD_ANGLED:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_KEYBOARDA:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_DANCEPADA:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_TWINSTICKA:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_ARCADE:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_STICKLESS:
			return generatedLEDStickless(&positions);

		case BUTTON_LAYOUT_BUTTONS_ANGLED:
			return generatedLEDWasd(&positions);

		case BUTTON_LAYOUT_VLXA:
			return generatedLEDButtons(&positions);

		case BUTTON_LAYOUT_FIGHTBOARD_STICK:
			return generatedLEDWasd(&positions);

		case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
			return generatedLEDWasdFBM(&positions);
	}

	assert(false);
	return std::vector<std::vector<Pixel>>();
}

uint8_t NeoPicoLEDAddon::setupButtonPositions()
{
	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
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
	for (auto const buttonPosition : buttonPositions)
	{
		if (buttonPosition.second != -1)
			buttonCount++;
	}

	return buttonCount;
}

void NeoPicoLEDAddon::configureLEDs()
{
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	uint8_t buttonCount = setupButtonPositions();
	vector<vector<Pixel>> pixels = createLEDLayout(ledOptions.ledLayout, ledOptions.ledsPerButton, buttonCount);
	matrix.setup(pixels, ledOptions.ledsPerButton);
	ledCount = matrix.getLedCount();
	if (PLED_TYPE == PLED_TYPE_RGB && PLED_COUNT > 0)
		ledCount += PLED_COUNT;

	// Remove the old neopico (config can call this)
	delete neopico;
	neopico = new NeoPico(ledOptions.dataPin, ledCount, ledOptions.ledFormat);
	neopico->Off();

	Animation::format = ledOptions.ledFormat;
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

	if (gamepad->pressedF1())
	{
		if (gamepad->pressedB3())
		{
			action = HOTKEY_LEDS_ANIMATION_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B3 | gamepad->f1Mask);
		}
		else if (gamepad->pressedB1())
		{
			action = HOTKEY_LEDS_ANIMATION_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B1 | gamepad->f1Mask);
		}
		else if (gamepad->pressedB4())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B4 | gamepad->f1Mask);
		}
		else if (gamepad->pressedB2())
		{
			action = HOTKEY_LEDS_BRIGHTNESS_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B2 | gamepad->f1Mask);
		}
		else if (gamepad->pressedR1())
		{
			action = HOTKEY_LEDS_PARAMETER_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R1 | gamepad->f1Mask);
		}
		else if (gamepad->pressedR2())
		{
			action = HOTKEY_LEDS_PARAMETER_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R2 | gamepad->f1Mask);
		}
		else if (gamepad->pressedL1())
		{
			action = HOTKEY_LEDS_PRESS_PARAMETER_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L1 | gamepad->f1Mask);
		}
		else if (gamepad->pressedL2())
		{
			action = HOTKEY_LEDS_PRESS_PARAMETER_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L2 | gamepad->f1Mask);
		}
	}

	return action;
}
