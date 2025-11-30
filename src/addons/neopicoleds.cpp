/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "animationstation.h"
#include "storagemanager.h"
#include "NeoPico.h"
#include "pixel.h"
#include "playerleds.h"
#include "specialmovesystem.h"
#include "gp2040.h"
#include "addons/neopicoleds.h"
#include "addons/pleds.h"
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

bool NeoPicoLEDAddon::bRestartLeds = false;

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

PLEDAnimationState getSwitchProAnimationNEOPICO(uint16_t ledState)
{
    PLEDAnimationState animationState =
    {
        .state = 0,
        .animation = PLED_ANIM_NONE,
        .speed = PLED_SPEED_OFF,
    };

    if (ledState != 0) {
        uint8_t ledNumber = ledState & 0x0F;
        if (ledNumber & 0x01) animationState.state |= PLED_STATE_LED1;
        if (ledNumber & 0x02) animationState.state |= PLED_STATE_LED2;
        if (ledNumber & 0x04) animationState.state |= PLED_STATE_LED3;
        if (ledNumber & 0x08) animationState.state |= PLED_STATE_LED4;
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

void NeoPicoLEDAddon::setup() {
    // Set Default LED Options
    const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	turnOffWhenSuspended = ledOptions.turnOffWhenSuspended;

	// Setup our aux state player ID sensors
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    gamepad->auxState.playerID.enabled = true;
    gamepad->auxState.sensors.statusLight.enabled = true;

    if ( ledOptions.pledType == PLED_TYPE_RGB ) {
        neoPLEDs = new NeoPicoPlayerLEDs();
    }

	decompressSettings();

	configureLEDs();

	// Next Run
    nextRunTime = make_timeout_time_ms(0); // Reset timeout
}

void NeoPicoLEDAddon::process()
{
	if(bRestartLeds)
	{
		bRestartLeds = false;
		AnimStation.Clear();
		neopico.Clear();
		neopico.Show();
		decompressSettings();
		configureLEDs();
	}

	//Check we have LEDs enabled and is it time to update
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	if (!isValidPin(ledOptions.dataPin) || !time_reached(this->nextRunTime))
		return;

	//Handle player leds (player id lights)
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
   if (ledOptions.pledType == PLED_TYPE_RGB) {
        if (gamepad->auxState.playerID.enabled && gamepad->auxState.playerID.active) {
            switch (gamepad->getOptions().inputMode) {
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
                case INPUT_MODE_SWITCH_PRO:
                    animationState = getSwitchProAnimationNEOPICO(gamepad->auxState.playerID.ledValue);
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
	GamepadHotkey action = ProcessAnimationHotkeys(gamepad);
	AnimStation.HandleEvent(action);

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

	//Update idle, button and special move animations etc
	AnimStation.Animate();

	//check if need to turn off due to usb suspension
	if (turnOffWhenSuspended && get_usb_suspended()) {
		AnimStation.DimBrightnessTo0();
	} else {
		AnimStation.SetBrightnessStepValue(AnimationStation::GetBrightnessStepValue());
	}

	//Grab led values this frame
	AnimStation.ApplyBrightness(frame);

    // Apply the player LEDs to our first 4 leds if we're in NEOPIXEL mode
    if (ledOptions.pledType == PLED_TYPE_RGB) {
        int32_t pledIndexes[] = { ledOptions.pledIndex1, ledOptions.pledIndex2, ledOptions.pledIndex3, ledOptions.pledIndex4 };
        for (int i = 0; i < PLED_COUNT; i++) {
            if (pledIndexes[i] < 0 || pledIndexes[i] > 99)
                continue;

            float level = (static_cast<float>(PLED_MAX_LEVEL - neoPLEDs->getLedLevels()[i]) / static_cast<float>(PLED_MAX_LEVEL));
            float brightness = as.GetNormalisedBrightness() * level;
            if (gamepad->auxState.sensors.statusLight.enabled && gamepad->auxState.sensors.statusLight.active) {
                rgbPLEDValues[i] = (RGB(gamepad->auxState.sensors.statusLight.color.red, gamepad->auxState.sensors.statusLight.color.green, gamepad->auxState.sensors.statusLight.color.blue)).value(neopico.GetFormat(), brightness);
            } else {
                rgbPLEDValues[i] = ((RGB)ledOptions.pledColor).value(neopico.GetFormat(), brightness);
            }
            frame[pledIndexes[i]] = rgbPLEDValues[i];
        }
    }

	// Get turbo options (turbo RGB led)
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
    // Turbo LED is a separate RGB that is on if turbo is on, and off if its off
    if ( turboOptions.turboLedType == PLED_TYPE_RGB ) { // RGB or PWM?
        if ( gamepad->auxState.turbo.activity == 1) { // Turbo is on (active sensor)
            if (turboOptions.turboLedIndex >= 0 && turboOptions.turboLedIndex < FRAME_MAX) { // Double check index value
                float brightness = as.GetNormalisedBrightness();
                frame[turboOptions.turboLedIndex] = ((RGB)turboOptions.turboLedColor).value(neopico.GetFormat(), brightness);
            }
        }
    }

	//Set led values out to the actual leds
	neopico.SetFrame(frame);
	neopico.Show();

	//queue up next frame time
	this->lastRunTime = get_absolute_time();
	this->nextRunTime = make_timeout_time_ms(NeoPicoLEDAddon::intervalMS);
}

///////////////////////////////////
// Old Pixel Setup functions
// Left here for legacy setup until all configs are converted
///////////////////////////////////

void NeoPicoLEDAddon::generateLegacyIndividualLight(int firstLedIndex, int xCoord, int yCoord, uint8_t ledsPerPixel, GpioAction actionButton)
{
	//If button doesnt have a light then return
	if(firstLedIndex < 0)
		return;

	firstLedIndex = firstLedIndex * ledsPerPixel;

    LEDOptions& options = Storage::getInstance().getLedOptions();
	if(options.lightClusterData_count >= FRAME_MAX) //Max data array size (defined in config proto)
		return;

	const GpioMappings& pinMappings = Storage::getInstance().getGpioMappings();

	//NOTE : I dont like this but I'm not sure theres a better way. Since sticks often have multiple buttons bound to the same action I'm hoping the first one found is the "master"
	int gpioPin = -1;
	for(int configIndex = 0; configIndex < pinMappings.pins_count; ++configIndex)
	{
		if(actionButton == pinMappings.pins[configIndex].action)
		{
			gpioPin = configIndex;
			break;
		}
	}

	int thisEntryIndex = options.lightClusterData_count;
	options.lightClusterData[thisEntryIndex].lightLocationData = firstLedIndex;
	options.lightClusterData[thisEntryIndex].lightLocationData += ledsPerPixel << 8;
	options.lightClusterData[thisEntryIndex].lightLocationData += xCoord << 16;
	options.lightClusterData[thisEntryIndex].lightLocationData += yCoord << 24;
	options.lightClusterData[thisEntryIndex].lightTypeData = gpioPin;
	options.lightClusterData[thisEntryIndex].lightTypeData += LightType::LightType_ActionButton << 8;
	options.lightClusterData_count++;
}

/**
 * @brief Create an LED layout using a 2x4 matrix.
 */
void NeoPicoLEDAddon::generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel)
{
	//8 action buttons in 2x4 array
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B3], 4, 0, ledsPerPixel, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B1], 4, 1, ledsPerPixel, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B4], 5, 0, ledsPerPixel, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B2], 5, 1, ledsPerPixel, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R1], 6, 0, ledsPerPixel, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R2], 6, 1, ledsPerPixel, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L1], 7, 0, ledsPerPixel, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L2], 7, 1, ledsPerPixel, BUTTON_PRESS_L2);

	//extras
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_LEFT], 0, 1, ledsPerPixel, BUTTON_PRESS_LEFT);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_DOWN], 1, 2, ledsPerPixel, BUTTON_PRESS_DOWN);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_RIGHT], 2, 1, ledsPerPixel, BUTTON_PRESS_RIGHT);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_UP], 1, 0, ledsPerPixel, BUTTON_PRESS_UP);
}

/**
 * @brief Create an LED layout using a 3x8 matrix.
 */
void NeoPicoLEDAddon::generatedLEDStickless(vector<vector<uint8_t>> *positions, uint8_t ledsPerPixel)
{
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_LEFT], 0, 2, ledsPerPixel, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_DOWN], 2, 2, ledsPerPixel, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_RIGHT], 4, 3, ledsPerPixel, BUTTON_PRESS_RIGHT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_UP], 5, 6, ledsPerPixel, BUTTON_PRESS_UP);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B3], 6, 2, ledsPerPixel, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B1], 6, 4, ledsPerPixel, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B4], 8, 1, ledsPerPixel, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B2], 8, 3, ledsPerPixel, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R1], 10, 1, ledsPerPixel, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R2], 10, 3, ledsPerPixel, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L1], 12, 1, ledsPerPixel, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L2], 12, 3, ledsPerPixel, BUTTON_PRESS_L2);

	//extras
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S1], 13, 0, ledsPerPixel, BUTTON_PRESS_S1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S2], 14, 0, ledsPerPixel, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L3], 7, 0, ledsPerPixel, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R3], 7, 5, ledsPerPixel, BUTTON_PRESS_R3);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A1], 12, 0, ledsPerPixel, BUTTON_PRESS_A1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A2], 11, 0, ledsPerPixel, BUTTON_PRESS_A2);
}

/**
 * @brief Create an LED layout using a 2x7 matrix.
 */
void NeoPicoLEDAddon::generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel)
{
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_LEFT], 0, 2, ledsPerPixel, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_DOWN], 2, 2, ledsPerPixel, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_RIGHT], 4, 3, ledsPerPixel, BUTTON_PRESS_RIGHT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_UP], 2, 1, ledsPerPixel, BUTTON_PRESS_UP);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B3], 6, 2, ledsPerPixel, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B1], 6, 4, ledsPerPixel, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B4], 8, 1, ledsPerPixel, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B2], 8, 3, ledsPerPixel, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R1], 10, 1, ledsPerPixel, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R2], 10, 3, ledsPerPixel, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L1], 12, 1, ledsPerPixel, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L2], 12, 3, ledsPerPixel, BUTTON_PRESS_L2);

	//extras
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S1], 13, 0, ledsPerPixel, BUTTON_PRESS_S1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S2], 14, 0, ledsPerPixel, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L3], 7, 0, ledsPerPixel, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R3], 7, 5, ledsPerPixel, BUTTON_PRESS_R3);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A1], 12, 0, ledsPerPixel, BUTTON_PRESS_A1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A2], 11, 0, ledsPerPixel, BUTTON_PRESS_A2);
}

/**
 * @brief Create an LED layout using a 2x7 matrix for the mirrored Fightboard.
 */
void NeoPicoLEDAddon::generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions, uint8_t ledsPerPixel)
{
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_LEFT], 8, 2, ledsPerPixel, BUTTON_PRESS_LEFT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_DOWN], 10, 2, ledsPerPixel, BUTTON_PRESS_DOWN);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_RIGHT], 12, 3, ledsPerPixel, BUTTON_PRESS_RIGHT);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_UP], 10, 1, ledsPerPixel, BUTTON_PRESS_UP);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B3], 0, 2, ledsPerPixel, BUTTON_PRESS_B3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B1], 0, 4, ledsPerPixel, BUTTON_PRESS_B1);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B4], 2, 1, ledsPerPixel, BUTTON_PRESS_B4);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_B2], 2, 3, ledsPerPixel, BUTTON_PRESS_B2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R1], 4, 1, ledsPerPixel, BUTTON_PRESS_R1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R2], 4, 3, ledsPerPixel, BUTTON_PRESS_R2);

	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L1], 6, 1, ledsPerPixel, BUTTON_PRESS_L1);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L2], 6, 3, ledsPerPixel, BUTTON_PRESS_L2);

	//extras
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S1], 13, 0, ledsPerPixel, BUTTON_PRESS_S1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_S2], 14, 0, ledsPerPixel, BUTTON_PRESS_S2);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_L3], 1, 0, ledsPerPixel, BUTTON_PRESS_L3);
	generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_R3], 1, 5, ledsPerPixel, BUTTON_PRESS_R3);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A1], 12, 0, ledsPerPixel, BUTTON_PRESS_A1);
	//generateLegacyIndividualLight(buttonPositions[BUTTON_LABEL_A2], 11, 0, ledsPerPixel, BUTTON_PRESS_A2);
}

void NeoPicoLEDAddon::createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount)
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
			generatedLEDStickless(&positions, ledsPerPixel);
			break;
		case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
			generatedLEDWasdFBM(&positions, ledsPerPixel);
			break;
		case BUTTON_LAYOUT_BUTTONS_ANGLED:
		case BUTTON_LAYOUT_FIGHTBOARD_STICK:
			generatedLEDWasd(&positions, ledsPerPixel);
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
			generatedLEDButtons(&positions, ledsPerPixel);
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

void NeoPicoLEDAddon::AssignLedPreset(const unsigned char* data, int32_t dataSize) 
{
	LEDOptions& options = Storage::getInstance().getLedOptions();
	options.lightClusterData_count = 0;
	options.lightClusterDataInitialised = true;
	for (int thisEntryIndex = 0; (thisEntryIndex * 6) + 5 < dataSize; ++thisEntryIndex) //each data entry has 6 elements
	{
		int thisDataIndex = thisEntryIndex * 6;
		options.lightClusterData[thisEntryIndex].lightLocationData = data[thisDataIndex];
		options.lightClusterData[thisEntryIndex].lightLocationData += ((int)data[thisDataIndex+1]) << 8;
		options.lightClusterData[thisEntryIndex].lightLocationData += ((int)data[thisDataIndex+2]) << 16;
		options.lightClusterData[thisEntryIndex].lightLocationData += ((int)data[thisDataIndex+3]) << 24;
		options.lightClusterData[thisEntryIndex].lightTypeData = ((int)data[thisDataIndex+4]);
		options.lightClusterData[thisEntryIndex].lightTypeData += ((int)data[thisDataIndex+5]) << 8;

		options.lightClusterData_count = thisEntryIndex + 1;

		if(options.lightClusterData_count >= FRAME_MAX) //100 entries total
			return;
	}
}

void NeoPicoLEDAddon::configureLEDs()
{
	LEDOptions& ledOptions = Storage::getInstance().getLedOptions();

	//New grid based setup
	if(ledOptions.lightClusterDataInitialised == false)
	{
		ledOptions.lightClusterDataInitialised = true;

		//do we have any presets defined?
		if(strcmp("", LIGHT_DATA_NAME_DEFAULT) != 0)
		{
			const unsigned char lightData[] = { LIGHT_DATA_DEFAULT };
			AssignLedPreset(lightData, sizeof(lightData));
		}
		else
		{
			//fall back to old matrix setup which will now approximate a grid and return the same data struct ready for light creation
			uint8_t buttonCount = setupButtonPositions();
			createLEDLayout(static_cast<ButtonLayout>(ledOptions.ledLayout), ledOptions.ledsPerButton, buttonCount);
		}

		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	}

	GenerateLights();
	ledCount = RGBLights.GetLedCount();

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0)
	{
	    int32_t pledIndexes[] = { ledOptions.pledIndex1, ledOptions.pledIndex2, ledOptions.pledIndex3, ledOptions.pledIndex4 };
        for (int i = 0; i < PLED_COUNT; i++)
		{
			if(pledIndexes[i] >= 0 && pledIndexes[i] <= 99 && pledIndexes[i] > ledCount)
				ledCount = pledIndexes[i];
		}
	}

	// Setup neo pico (once only)
	if(!bHasSetupNeoPico)
	{
		bHasSetupNeoPico = true;
		neopico.Setup(ledOptions.dataPin, ledCount, static_cast<LEDFormat>(ledOptions.ledFormat), pio0,0);
		neopico.Off();
	}
	else
	{
		neopico.ChangeNumPixels(ledCount);
	}

	Animation::format = static_cast<LEDFormat>(ledOptions.ledFormat);
	AnimStation.SetMaxBrightness(ledOptions.brightnessMaximum);
	AnimStation.SetBrightnessStepValue(AnimStation.options.brightness);
	AnimStation.specialMoveSystem.SetParentAnimationStation(&AnimStation);
	AnimStation.specialMoveSystem.SetDirectionMasks(GAMEPAD_MASK_DU, GAMEPAD_MASK_DD, GAMEPAD_MASK_DL, GAMEPAD_MASK_DR);
	AnimStation.specialMoveSystem.SetButtonMasks(GAMEPAD_MASK_B3);
	AnimStation.specialMoveSystem.Init();
	AnimStation.SetLights(RGBLights);
	AnimStation.SetMode(as.options.baseProfileIndex);
}

void NeoPicoLEDAddon::decompressSettings()
{
	AnimStation.DecompressSettings();
}

////////////////////////////////////////////
//New RGBLight setups
////////////////////////////////////////////

void NeoPicoLEDAddon::GenerateLights()
{
	int minX = -1;
	int minY = -1;

	LEDOptions& ledOptions = Storage::getInstance().getLedOptions();

	std::vector<Light> generatedLights;
	for(int index = 0; index < (int)ledOptions.lightClusterData_count; ++index)
	{
		int ledIndex = (ledOptions.lightClusterData[index].lightLocationData) & 0xFF;
		int ledCount = (ledOptions.lightClusterData[index].lightLocationData >> 8) & 0xFF;
		int posX = (ledOptions.lightClusterData[index].lightLocationData >> 16) & 0xFF;
		int posY = (ledOptions.lightClusterData[index].lightLocationData >> 24) & 0xFF;
		int gpioPin = (ledOptions.lightClusterData[index].lightTypeData) & 0xFF;
		int ledType = (ledOptions.lightClusterData[index].lightTypeData >> 8) & 0xFF;
		//Data format = {first led index, leds on this light, xcoord, ycoord, GPIO pin, Type}
		LightPosition newLightPos (posX, posY);
		Light newLight (ledIndex,
						ledCount,
						newLightPos,
						gpioPin,
						(LightType)ledType);

		//Update mins
		if(minX == -1 || newLight.Position.XPosition < minX)
			minX = newLight.Position.XPosition;
		if(minY == -1 || newLight.Position.YPosition < minY)
			minY = newLight.Position.YPosition;

		generatedLights.push_back(newLight);
	}

	//check for critical error
	if(minX < 0 || minY < 0)
		return;

	//Strip Empty rows and coloums on left and top side
	for(int index = 0; index < (int)generatedLights.size(); ++index)
	{
		generatedLights[index].Position.XPosition -= minX;
		generatedLights[index].Position.YPosition -= minY;
	}

	RGBLights.Setup(generatedLights);
}

////////////////////////////////////////////
//Helper functions
////////////////////////////////////////////

GamepadHotkey NeoPicoLEDAddon::ProcessAnimationHotkeys(Gamepad *gamepad)
{
	GamepadHotkey action = HOTKEY_LEDS_NONE;

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
			action = HOTKEY_LEDS_PARAMETER_CYCLE;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedR2())
		{
			action = HOTKEY_LEDS_CASE_PARAMETER_CYCLE;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedL1())
		{
			action = HOTKEY_LEDS_PRESS_PARAMETER_CYCLE;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		else if (gamepad->pressedL2())
		{
			action = HOTKEY_LEDS_PARAMETER_UNUSED;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		}
		// else if (gamepad->pressedL3())
		// {
		// 	action = HOTKEY_LEDS_SPECIALMOVE_PROFILE_UP;
		// 	gamepad->state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		// }
		// else if (gamepad->pressedR3())
		// {
		// 	action = HOTKEY_LEDS_SPECIALMOVE_PROFILE_DOWN;
		// 	gamepad->state.buttons &= ~(GAMEPAD_MASK_R3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
		// }
	}

	return action;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//END RBG LEDs ///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
