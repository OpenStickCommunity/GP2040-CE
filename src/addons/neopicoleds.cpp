/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "AnimationStation.hpp"
#include "SpecialMoveSystem.hpp"
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Player LEDs ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//END Player LEDs ////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//RBG LEDs ///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
//GP Addon functions
///////////////////////////////////

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
	//Check we have LEDs enabled and is it time to update
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	if (!isValidPin(ledOptions.dataPin) || !time_reached(this->nextRunTime))
		return;

	//Process hotkeys and action any requests
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
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

	//Check for button combos that change animation settings
	AnimationHotkey action = ProcessAnimationHotkeys(gamepad);
	if ( action != HOTKEY_LEDS_NONE ) {
		AnimStation.HandleEvent(action);
	}

	//New check for buttons being pressed. this is a direct check to see if a pin is held
	Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
	vector<int32_t> pressedPins;
	for(auto thisLight : RGBLights.AllLights)
	{
		if(values & (1 << thisLight.GIPOPin))
		{
			pressedPins.push_back(thisLight.GIPOPin);
		}
	}
	AnimStation.HandlePressedPins(pressedPins);

	//Still need to check logical buttons so that we can trigger special moves
	uint32_t buttonState = gamepad->state.dpad << 16 | gamepad->state.buttons;
	AnimStation.HandlePressedButtons(buttonState);

	//Update idle, button and special move animations
	AnimStation.Animate();

	//check if need to turn off due to usb suspension
	if (turnOffWhenSuspended && get_usb_suspended()) {
		AnimStation.DimBrightnessTo0();
	} else {
		AnimStation.SetBrightness(AnimationStation::GetBrightness());
	}

	//Grab led values this frame
	AnimStation.ApplyBrightness(frame);

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

	//Set led values out to the actual leds
	neopico->SetFrame(frame);
	neopico->Show();
	AnimationStore.save();

	//queue up next frame time
	this->nextRunTime = make_timeout_time_ms(NeoPicoLEDAddon::intervalMS);
}

///////////////////////////////////
// Old Pixel Setup functions
// Left here for legacy setup until all configs are converted
///////////////////////////////////

void NeoPicoLEDAddon::generateLegacyIndividualLight(int lightIndex, int firstLedIndex, int xCoord, int yCoord, uint8_t ledsPerPixel, LEDOptions_lightData_t& out_lightData, GpioAction actionButton)
{
	const GpioMappings& pinMappings = Storage::getInstance().getGpioMappings();

	int gpioPin = -1;
	for(int configIndex = 0; configIndex < pinMappings.pins_count; ++configIndex)
	{
		if(actionButton == pinMappings.pins[configIndex].action)
			gpioPin = configIndex;
	}

	int arrayOffset = lightIndex * 6;
	out_lightData.bytes[arrayOffset] = firstLedIndex; //first led index
	out_lightData.bytes[arrayOffset+1] = ledsPerPixel; //leds on this light
	out_lightData.bytes[arrayOffset+2] = xCoord; //xcoord
	out_lightData.bytes[arrayOffset+3] = yCoord; //ycoord
	out_lightData.bytes[arrayOffset+4] = gpioPin; //GPIO pin
	out_lightData.bytes[arrayOffset+5] = LightType::LightType_ActionButton; //Type
}

/**
 * @brief Create an LED layout using a 2x4 matrix.
 */
void NeoPicoLEDAddon::generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel, LEDOptions_lightData_t& out_lightData, int32_t& out_lightDataSize)
{
	out_lightDataSize = 18;

	//8 action buttons in 2x4 array
	generateLegacyIndividualLight(0, ledsPerPixel * buttonPositions[BUTTON_LABEL_B3], 0, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(1, ledsPerPixel * buttonPositions[BUTTON_LABEL_B1], 0, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(2, ledsPerPixel * buttonPositions[BUTTON_LABEL_B4], 1, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(3, ledsPerPixel * buttonPositions[BUTTON_LABEL_B2], 1, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(4, ledsPerPixel * buttonPositions[BUTTON_LABEL_R1], 2, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(5, ledsPerPixel * buttonPositions[BUTTON_LABEL_R2], 2, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(6, ledsPerPixel * buttonPositions[BUTTON_LABEL_L1], 3, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(7, ledsPerPixel * buttonPositions[BUTTON_LABEL_L2], 3, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_L2);

	//extras
	generateLegacyIndividualLight(8, ledsPerPixel * buttonPositions[BUTTON_LABEL_LEFT], 4, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_LEFT);
	generateLegacyIndividualLight(9, ledsPerPixel * buttonPositions[BUTTON_LABEL_DOWN], 4, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_DOWN);
	generateLegacyIndividualLight(10, ledsPerPixel * buttonPositions[BUTTON_LABEL_RIGHT], 4, 2, ledsPerPixel, out_lightData, BUTTON_PRESS_RIGHT);
	generateLegacyIndividualLight(11, ledsPerPixel * buttonPositions[BUTTON_LABEL_UP], 4, 3, ledsPerPixel, out_lightData, BUTTON_PRESS_UP);
	generateLegacyIndividualLight(12, ledsPerPixel * buttonPositions[BUTTON_LABEL_S1], 4, 4, ledsPerPixel, out_lightData, BUTTON_PRESS_S1);
	generateLegacyIndividualLight(13, ledsPerPixel * buttonPositions[BUTTON_LABEL_S2], 4, 5, ledsPerPixel, out_lightData, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(14, ledsPerPixel * buttonPositions[BUTTON_LABEL_L3], 4, 6, ledsPerPixel, out_lightData, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(15, ledsPerPixel * buttonPositions[BUTTON_LABEL_R3], 4, 7, ledsPerPixel, out_lightData, BUTTON_PRESS_R3);
	generateLegacyIndividualLight(16, ledsPerPixel * buttonPositions[BUTTON_LABEL_A1], 4, 8, ledsPerPixel, out_lightData, BUTTON_PRESS_A1);
	generateLegacyIndividualLight(17, ledsPerPixel * buttonPositions[BUTTON_LABEL_A2], 4, 9, ledsPerPixel, out_lightData, BUTTON_PRESS_A2);
}

/**
 * @brief Create an LED layout using a 3x8 matrix.
 */
void NeoPicoLEDAddon::generatedLEDStickless(vector<vector<uint8_t>> *positions, uint8_t ledsPerPixel, LEDOptions_lightData_t& out_lightData, int32_t& out_lightDataSize)
{
	out_lightDataSize = 18;

	generateLegacyIndividualLight(0, ledsPerPixel * buttonPositions[BUTTON_LABEL_LEFT], 0, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(1, ledsPerPixel * buttonPositions[BUTTON_LABEL_DOWN], 1, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(2, ledsPerPixel * buttonPositions[BUTTON_LABEL_RIGHT], 2, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_RIGHT);

	generateLegacyIndividualLight(3, ledsPerPixel * buttonPositions[BUTTON_LABEL_UP], 3, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_UP);

	generateLegacyIndividualLight(4, ledsPerPixel * buttonPositions[BUTTON_LABEL_B3], 4, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(5, ledsPerPixel * buttonPositions[BUTTON_LABEL_B1], 4, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(6, ledsPerPixel * buttonPositions[BUTTON_LABEL_B4], 5, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(7, ledsPerPixel * buttonPositions[BUTTON_LABEL_B2], 5, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(8, ledsPerPixel * buttonPositions[BUTTON_LABEL_R1], 6, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(9, ledsPerPixel * buttonPositions[BUTTON_LABEL_R2], 6, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(10, ledsPerPixel * buttonPositions[BUTTON_LABEL_L1], 7, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(11, ledsPerPixel * buttonPositions[BUTTON_LABEL_L2], 7, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_L2);

	//extras
	generateLegacyIndividualLight(12, ledsPerPixel * buttonPositions[BUTTON_LABEL_S1], 8, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_S1);
	generateLegacyIndividualLight(13, ledsPerPixel * buttonPositions[BUTTON_LABEL_S2], 8, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(14, ledsPerPixel * buttonPositions[BUTTON_LABEL_L3], 8, 2, ledsPerPixel, out_lightData, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(15, ledsPerPixel * buttonPositions[BUTTON_LABEL_R3], 8, 3, ledsPerPixel, out_lightData, BUTTON_PRESS_R3);
	generateLegacyIndividualLight(16, ledsPerPixel * buttonPositions[BUTTON_LABEL_A1], 8, 4, ledsPerPixel, out_lightData, BUTTON_PRESS_A1);
	generateLegacyIndividualLight(17, ledsPerPixel * buttonPositions[BUTTON_LABEL_A2], 8, 5, ledsPerPixel, out_lightData, BUTTON_PRESS_A2);
}

/**
 * @brief Create an LED layout using a 2x7 matrix.
 */
void NeoPicoLEDAddon::generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel, LEDOptions_lightData_t& out_lightData, int32_t& out_lightDataSize)
{
	out_lightDataSize = 18;

	generateLegacyIndividualLight(0, ledsPerPixel * buttonPositions[BUTTON_LABEL_LEFT], 0, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(1, ledsPerPixel * buttonPositions[BUTTON_LABEL_UP], 1, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_UP);
	generateLegacyIndividualLight(2, ledsPerPixel * buttonPositions[BUTTON_LABEL_DOWN], 1, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(3, ledsPerPixel * buttonPositions[BUTTON_LABEL_RIGHT], 2, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_RIGHT);

	generateLegacyIndividualLight(4, ledsPerPixel * buttonPositions[BUTTON_LABEL_B3], 3, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(5, ledsPerPixel * buttonPositions[BUTTON_LABEL_B1], 3, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(6, ledsPerPixel * buttonPositions[BUTTON_LABEL_B4], 4, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(7, ledsPerPixel * buttonPositions[BUTTON_LABEL_B2], 4, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(8, ledsPerPixel * buttonPositions[BUTTON_LABEL_R1], 5, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(9, ledsPerPixel * buttonPositions[BUTTON_LABEL_R2], 5, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(10, ledsPerPixel * buttonPositions[BUTTON_LABEL_L1], 6, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(11, ledsPerPixel * buttonPositions[BUTTON_LABEL_L2], 6, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_L2);

	//extras
	generateLegacyIndividualLight(12, ledsPerPixel * buttonPositions[BUTTON_LABEL_S1], 7, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_S1);
	generateLegacyIndividualLight(13, ledsPerPixel * buttonPositions[BUTTON_LABEL_S2], 7, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(14, ledsPerPixel * buttonPositions[BUTTON_LABEL_L3], 7, 2, ledsPerPixel, out_lightData, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(15, ledsPerPixel * buttonPositions[BUTTON_LABEL_R3], 7, 3, ledsPerPixel, out_lightData, BUTTON_PRESS_R3);
	generateLegacyIndividualLight(16, ledsPerPixel * buttonPositions[BUTTON_LABEL_A1], 7, 4, ledsPerPixel, out_lightData, BUTTON_PRESS_A1);
	generateLegacyIndividualLight(17, ledsPerPixel * buttonPositions[BUTTON_LABEL_A2], 7, 5, ledsPerPixel, out_lightData, BUTTON_PRESS_A2);
}

/**
 * @brief Create an LED layout using a 2x7 matrix for the mirrored Fightboard.
 */
void NeoPicoLEDAddon::generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel, LEDOptions_lightData_t& out_lightData, int32_t& out_lightDataSize)
{
	out_lightDataSize = 18;

	generateLegacyIndividualLight(0, ledsPerPixel * buttonPositions[BUTTON_LABEL_L1], 0, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(1, ledsPerPixel * buttonPositions[BUTTON_LABEL_L2], 0, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_L2);

	generateLegacyIndividualLight(2, ledsPerPixel * buttonPositions[BUTTON_LABEL_R1], 1, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(3, ledsPerPixel * buttonPositions[BUTTON_LABEL_R2], 1, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(4, ledsPerPixel * buttonPositions[BUTTON_LABEL_B4], 2, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(5, ledsPerPixel * buttonPositions[BUTTON_LABEL_B2], 2, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(6, ledsPerPixel * buttonPositions[BUTTON_LABEL_B3], 3, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(7, ledsPerPixel * buttonPositions[BUTTON_LABEL_B1], 3, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(8, ledsPerPixel * buttonPositions[BUTTON_LABEL_LEFT], 4, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(9, ledsPerPixel * buttonPositions[BUTTON_LABEL_UP], 5, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_UP);
	generateLegacyIndividualLight(10, ledsPerPixel * buttonPositions[BUTTON_LABEL_DOWN], 5, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(11, ledsPerPixel * buttonPositions[BUTTON_LABEL_RIGHT], 6, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_RIGHT);

	//extras
	generateLegacyIndividualLight(12, ledsPerPixel * buttonPositions[BUTTON_LABEL_S1], 7, 0, ledsPerPixel, out_lightData, BUTTON_PRESS_S1);
	generateLegacyIndividualLight(13, ledsPerPixel * buttonPositions[BUTTON_LABEL_S2], 7, 1, ledsPerPixel, out_lightData, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(14, ledsPerPixel * buttonPositions[BUTTON_LABEL_L3], 7, 2, ledsPerPixel, out_lightData, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(15, ledsPerPixel * buttonPositions[BUTTON_LABEL_R3], 7, 3, ledsPerPixel, out_lightData, BUTTON_PRESS_R3);
	generateLegacyIndividualLight(16, ledsPerPixel * buttonPositions[BUTTON_LABEL_A1], 7, 4, ledsPerPixel, out_lightData, BUTTON_PRESS_A1);
	generateLegacyIndividualLight(17, ledsPerPixel * buttonPositions[BUTTON_LABEL_A2], 7, 5, ledsPerPixel, out_lightData, BUTTON_PRESS_A2);
}

void NeoPicoLEDAddon::createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount, LEDOptions_lightData_t& out_lightData, int32_t& out_lightDataSize)
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
			generatedLEDStickless(&positions, ledsPerPixel, out_lightData, out_lightDataSize);
			break;
		case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
			generatedLEDWasdFBM(&positions, ledsPerPixel, out_lightData, out_lightDataSize);
			break;
		case BUTTON_LAYOUT_BUTTONS_ANGLED:
		case BUTTON_LAYOUT_FIGHTBOARD_STICK:
			generatedLEDWasd(&positions, ledsPerPixel, out_lightData, out_lightDataSize);
			break;
		case BUTTON_LAYOUT_BLANKA:
		case BUTTON_LAYOUT_BUTTONS_BASIC:
		case BUTTON_LAYOUT_KEYBOARD_ANGLED:
		case BUTTON_LAYOUT_KEYBOARDA:
		case BUTTON_LAYOUT_DANCEPADA:
		case BUTTON_LAYOUT_TWINSTICKA:
		case BUTTON_LAYOUT_ARCADE:
		case BUTTON_LAYOUT_VLXA:
		default:
			generatedLEDButtons(&positions, ledsPerPixel, out_lightData, out_lightDataSize);
			break;
	}
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

	//New grid based setup
	LEDOptions_lightData_t lightData = ledOptions.lightData; //todo make this const ref if we remove the legacy version
	int32_t lightDataSize = ledOptions.lightDataSize;
	if(lightDataSize == 0)
	{
		//fall back to old matrix setup which will now approximate a grid and return the same data struct ready for light creation
		uint8_t buttonCount = setupButtonPositions();
		createLEDLayout(static_cast<ButtonLayout>(ledOptions.ledLayout), ledOptions.ledsPerButton, buttonCount, lightData, lightDataSize);
	}

	GenerateLights(lightData, lightDataSize);
	ledCount = RGBLights.GetLedCount();

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0)
		ledCount += PLED_COUNT;

	// Remove the old neopico (config can call this)
	delete neopico;
	neopico = new NeoPico(ledOptions.dataPin, ledCount, static_cast<LEDFormat>(ledOptions.ledFormat));
	neopico->Off();

	Animation::format = static_cast<LEDFormat>(ledOptions.ledFormat);
	AnimStation.ConfigureBrightness(ledOptions.brightnessMaximum, ledOptions.brightnessSteps);
	AnimationStore.getAnimationOptions(AnimStation.options);
	AnimationStore.getSpecialMoveOptions(AnimStation.specialMoveSystem.Options);
	AnimStation.SetBrightness(AnimStation.options.brightness);
	AnimStation.specialMoveSystem.SetParentAnimationStation(&AnimStation);
	AnimStation.specialMoveSystem.SetDirectionMasks(GAMEPAD_MASK_DU, GAMEPAD_MASK_DD, GAMEPAD_MASK_DL, GAMEPAD_MASK_DR);
	AnimStation.specialMoveSystem.SetButtonMasks(GAMEPAD_MASK_B3);
	AnimStation.specialMoveSystem.Init();
	AnimStation.SetLights(RGBLights);
	AnimStation.SetMode(as.options.baseProfileIndex);
}

////////////////////////////////////////////
//New RGBLight setups
////////////////////////////////////////////

void NeoPicoLEDAddon::GenerateLights(const LEDOptions_lightData_t& InLightData, uint32_t InLightDataSize)
{
	std::vector<Light> generatedLights;
	for(int index = 0; index < (int)InLightDataSize; ++index)
	{
		int arrayOffset = index*6;
		//Data format = {first led index, leds on this light, xcoord, ycoord, GPIO pin, Type}
		LightPosition newLightPos (InLightData.bytes[arrayOffset+2], InLightData.bytes[arrayOffset+3]);
		Light newLight (InLightData.bytes[arrayOffset],
						InLightData.bytes[arrayOffset+1],
						newLightPos,
						InLightData.bytes[arrayOffset+4],
						(LightType)InLightData.bytes[arrayOffset+5]);

		generatedLights.push_back(newLight);
	}

	RGBLights.Setup(generatedLights);
}

////////////////////////////////////////////
//Helper functions
////////////////////////////////////////////

AnimationHotkey NeoPicoLEDAddon::ProcessAnimationHotkeys(Gamepad *gamepad)
{
	AnimationHotkey action = HOTKEY_LEDS_NONE;

	if (gamepad->pressedS1() && gamepad->pressedS2())
	{
		if (gamepad->pressedB3())
		{
			action = HOTKEY_LEDS_PROFILE_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedB1())
		{
			action = HOTKEY_LEDS_PROFILE_DOWN;
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
			action = HOTKEY_LEDS_SPECIALMOVE_PROFILE_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedR3())
		{
			action = HOTKEY_LEDS_SPECIALMOVE_PROFILE_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
	}

	return action;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//END RBG LEDs ///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////