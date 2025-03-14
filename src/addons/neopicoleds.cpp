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

#define FRAME_MAX 100
#define AL_ROW	5
#define AL_COL	8
#define AL_STATIC_COLOR_COUNT	14
#define AL_EFFECT_MODE_MAX 5
#define POWER_ON_EFFECT 1	            // chase:1	breath:0
#define CHASE_LIGHTS_TURN_ON 4
#define CHASE_LIGHTS_INTERVAL 11
#define POWER_ON_EFFECT_DURATION 5000
#define POWER_ON_CHASE_FRAME_MAX 30     // M Lite:20		M Ultra:30
#define RESERVE_DUMMY_LEDS_NUM 8
#define POWER_ON_CHASE_BRIGHTNESSX 1

typedef enum
{
  AL_CUSTOM_EFFECT_STATIC_COLOR,	// Static
  AL_CUSTOM_EFFECT_GRADIENT,	    // Gradient Color
  AL_CUSTOM_EFFECT_CHASE,		    // Chase
  AL_CUSTOM_EFFECT_BREATH,		    // Breathing
  AL_CUSTOM_EFFECT_STATIC_THEME,	// Custom Static
} AmbientLightCustomEffects;

const RGB alCustomStaticTheme[AL_ROW][AL_COL] = 
									  {{ColorRed, ColorOrange, ColorYellow, ColorGreen, ColorBlue, ColorIndigo, ColorViolet, ColorWhite},
									   {ColorOrange, ColorRed, ColorGreen, ColorYellow, ColorIndigo, ColorBlue, ColorWhite, ColorViolet},
									   {ColorYellow, ColorOrange, ColorRed, ColorIndigo, ColorBlue, ColorGreen, ColorViolet, ColorWhite},
									   {ColorGreen, ColorOrange, ColorYellow, ColorRed, ColorWhite, ColorIndigo, ColorViolet, ColorBlue},
									   {ColorWhite, ColorIndigo, ColorViolet, ColorOrange, ColorBlue, ColorGreen, ColorYellow, ColorRed},};

const RGB alCustomStaticColors[AL_STATIC_COLOR_COUNT] {
    ColorBlack,     ColorWhite,  ColorRed,     ColorOrange, ColorYellow,
    ColorLimeGreen, ColorGreen,  ColorSeafoam, ColorAqua,   ColorSkyBlue,
    ColorBlue,      ColorPurple, ColorPink,    ColorMagenta };

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

    // Get turbo options (turbo RGB led)
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;

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

void NeoPicoLEDAddon::ambientLightCustom(uint32_t alFrame[100]) 
{
	uint8_t chaseLightMaxIndexPos = 0;
	uint8_t alStartIndex = 0;

	static float alBrightnessBreathX = 1.00f;
	static uint8_t breathLedEffectCycle = 0;
	static bool alReverse = false;
	static int alCurrentFrame = 0;
	static int alFrameToRGB = 0;
	static int alFrameSpeed = 2;

	static uint8_t alR = 0x00;
	static uint8_t alG = 0x00;
	static uint8_t alB = 0x00;

	static absolute_time_t nextRunTimeAmbientLight = nil_time;

	AnimationOptions animationOptions = AnimationStore.getAnimationOptions();
	uint8_t ambientLightEffectsCount = animationOptions.ambientLightEffectsCountIndex;

	float staticColorBrightnessCustomX = animationOptions.alStaticColorBrightnessCustomX;
	float gradientBrightnessCustomX = animationOptions.alGradientBrightnessCustomX;
	float chaseBrightnessCustomX = animationOptions.alChaseBrightnessCustomX;
	float staticBrightnessCustomThemeX = animationOptions.alStaticBrightnessCustomThemeX;
	float customBrightnessStep = 1.0f / as.GetBrightnessSteps();
	
	int alFrameGradientSpeed = animationOptions.ambientLightGradientSpeed;
	int alFrameChaseSpeed = animationOptions.ambientLightChaseSpeed;
	float alFrameBreathSpeed = animationOptions.ambientLightBreathSpeed;

	uint8_t buttonCount = setupButtonPositions();
	uint8_t totalOfButtonLedsCount = this->ledCount;
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();

	uint8_t multipleOfCustomStaticThemeCount;
	uint8_t remainderOfCustomStaticThemeCount;
	uint8_t customStaticColorIndex = 0;
	int8_t customStaticThemeCount = animationOptions.alCustomStaticThemeIndex;
	int8_t customStaticColorCount = animationOptions.alCustomStaticColorIndex;

	#if POWER_ON_EFFECT == 1	// Power-on animation chase
	uint8_t ledsCount = totalOfButtonLedsCount;

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
		ledsCount = totalOfButtonLedsCount - PLED_COUNT;
	}

	if(ledOptions.ledsPerButton == 1){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 2){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 3){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT; // max 96
		}
		else{
			alStartIndex = ledsCount;
			chaseLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else{
		alStartIndex = ledsCount;	
		chaseLightMaxIndexPos = FRAME_MAX - 1;	
	}

	static uint8_t chaseLightOfPowerOnIndex = alStartIndex;
	uint32_t now = to_ms_since_boot(get_absolute_time());

	if(now < POWER_ON_EFFECT_DURATION){ 
		alR = 0;
		alG = 0;
		alB = 255;
		for(int j = 0; j < (FRAME_MAX - alStartIndex); j++){
			alFrame[alStartIndex + j] = 0x0;
		}

		if(chaseLightOfPowerOnIndex < (FRAME_MAX - CHASE_LIGHTS_TURN_ON - 1)){
			for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
				alFrame[chaseLightOfPowerOnIndex + i] = ((uint32_t)(alG * POWER_ON_CHASE_BRIGHTNESSX) << 16) 
													  | ((uint32_t)(alR * POWER_ON_CHASE_BRIGHTNESSX) << 8) 
													  | (uint32_t)(alB * POWER_ON_CHASE_BRIGHTNESSX);
			}
				
			if((chaseLightOfPowerOnIndex - totalOfButtonLedsCount) > CHASE_LIGHTS_INTERVAL){
				for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
					alFrame[chaseLightOfPowerOnIndex - CHASE_LIGHTS_INTERVAL + i] = ((uint32_t)(alG * POWER_ON_CHASE_BRIGHTNESSX) << 16) 
																			    | ((uint32_t)(alR * POWER_ON_CHASE_BRIGHTNESSX) << 8) 
																				| (uint32_t)(alB * POWER_ON_CHASE_BRIGHTNESSX);
				}
			}
		}

		chaseLightOfPowerOnIndex++;
		if(chaseLightOfPowerOnIndex > (chaseLightMaxIndexPos - CHASE_LIGHTS_TURN_ON - 1)){
			chaseLightOfPowerOnIndex = alStartIndex;
		}

		return;
	}
	#else	// Power-on animation breathing lights
	static float alPowerOnBrightnessY = 0.0f; 
	static bool alPowerOnReverse = true;

	uint8_t ledsCount = totalOfButtonLedsCount;

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
		ledsCount = totalOfButtonLedsCount - PLED_COUNT;
	}
	alStartIndex = ledsCount;

	uint32_t now = to_ms_since_boot(get_absolute_time());

	if(now < POWER_ON_EFFECT_DURATION){ 
		alR = 0;
		alG = 0;
		alB = 255;

		if((now > POWER_ON_EFFECT_DURATION) && (alPowerOnBrightnessY == 0.0f) && (alPowerOnReverse == true)){
			alPowerOnBrightnessY = 0.0f;
			alPowerOnReverse = true;
			return;
		}

		if(alPowerOnReverse){
			alPowerOnBrightnessY += 0.1f;
			if(alPowerOnBrightnessY > 1.0f){
				alPowerOnBrightnessY = 1.0f;
				alPowerOnReverse = false;
			}
		}
		else{
			alPowerOnBrightnessY -= 0.1f;
			if(alPowerOnBrightnessY < 0.0f){
				alPowerOnBrightnessY = 0.0f;
				alPowerOnReverse = true;
			}
		}

		for(int i = 0; i < (FRAME_MAX - alStartIndex); i++){
			alFrame[alStartIndex + i] = ((uint32_t)(alG * alPowerOnBrightnessY) << 16) 
									  | ((uint32_t)(alR * alPowerOnBrightnessY) << 8) 
									  | (uint32_t)(alB * alPowerOnBrightnessY);
		}
		
		return;
	}
	#endif
	
	if(ledOptions.ledsPerButton == 1){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 2){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 3){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT; // max 96
		}
		else{
			alStartIndex = totalOfButtonLedsCount;
			chaseLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else{
		alStartIndex = totalOfButtonLedsCount;	
		chaseLightMaxIndexPos = FRAME_MAX - 1;	// 99
	}

	static uint8_t chaseLightIndex = alStartIndex;

	if(as.ambientLightEffectsChangeFlag == true){	// ambient effect change set
		if(as.ambientLightOnOffFlag == false){ // non-closed
			ambientLightEffectsCount++;
			if(ambientLightEffectsCount > AL_EFFECT_MODE_MAX - 1){
				ambientLightEffectsCount = 0;
			}
			alR = 0x00;
			alG = 0x00;
			alB = 0x00;
			alCurrentFrame = 0;
			alFrameToRGB = 0;
			alReverse = false;
			chaseLightIndex = alStartIndex;
			alBrightnessBreathX = 1.00f;
			breathLedEffectCycle = 0;
		}
		as.ambientLightEffectsChangeFlag = false;
	}
	AnimationStation::options.ambientLightEffectsCountIndex = ambientLightEffectsCount;	// save 

	if(ambientLightEffectsCount == AL_CUSTOM_EFFECT_STATIC_COLOR){
		if(as.aleLedsBrightnessCustomXupFlag == true){	// brightness up
			if(as.ambientLightOnOffFlag == false){ // non-closed
				staticColorBrightnessCustomX += customBrightnessStep;
				if(staticColorBrightnessCustomX > 1.00f){
					staticColorBrightnessCustomX = 1.00f;
				}
			}
			as.aleLedsBrightnessCustomXupFlag = false;
		}

		if(as.aleLedsBrightnessCustomXDownFlag == true){	// brightness down
			if(as.ambientLightOnOffFlag == false){ // 非关闭情况下
				staticColorBrightnessCustomX -= customBrightnessStep;
				if(staticColorBrightnessCustomX <= 0.00f){
					staticColorBrightnessCustomX = 0.00f;
				}
			}
			as.aleLedsBrightnessCustomXDownFlag = false;
		}
	}
	else if(ambientLightEffectsCount == AL_CUSTOM_EFFECT_GRADIENT){ 
		if(as.aleLedsBrightnessCustomXupFlag == true){	// brightness up
			if(as.ambientLightOnOffFlag == false){ // non-closed
				gradientBrightnessCustomX += customBrightnessStep;
				if(gradientBrightnessCustomX > 1.00f){
					gradientBrightnessCustomX = 1.00f;
				}
			}
			as.aleLedsBrightnessCustomXupFlag = false;
		}

		if(as.aleLedsBrightnessCustomXDownFlag == true){	// brightness down
			if(as.ambientLightOnOffFlag == false){ // 非关闭情况下
				gradientBrightnessCustomX -= customBrightnessStep;
				if(gradientBrightnessCustomX <= 0.00f){
					gradientBrightnessCustomX = 0.00f;
				}
			}
			as.aleLedsBrightnessCustomXDownFlag = false;
		}
	}
	else if(ambientLightEffectsCount == AL_CUSTOM_EFFECT_CHASE){
		if(as.aleLedsBrightnessCustomXupFlag == true){	// brightness up
			if(as.ambientLightOnOffFlag == false){ // non-closed
				chaseBrightnessCustomX += customBrightnessStep;
				if(chaseBrightnessCustomX > 1.00f){
					chaseBrightnessCustomX = 1.00f;
				}
			}
			as.aleLedsBrightnessCustomXupFlag = false;
		}

		if(as.aleLedsBrightnessCustomXDownFlag == true){	// brightness down
			if(as.ambientLightOnOffFlag == false){ // 非关闭情况下
				chaseBrightnessCustomX -= customBrightnessStep;
				if(chaseBrightnessCustomX <= 0.00f){
					chaseBrightnessCustomX = 0.00f;
				}
			}
			as.aleLedsBrightnessCustomXDownFlag = false;
		}
	}
	else if(ambientLightEffectsCount == AL_CUSTOM_EFFECT_BREATH){ // 呼吸灯不支持调亮度
		as.aleLedsBrightnessCustomXupFlag = false;
		as.aleLedsBrightnessCustomXDownFlag = false;
	}
	else if(ambientLightEffectsCount == AL_CUSTOM_EFFECT_STATIC_THEME){
		if(as.aleLedsBrightnessCustomXupFlag == true){	// brightness up
			if(as.ambientLightOnOffFlag == false){ //non-closed
				staticBrightnessCustomThemeX += customBrightnessStep;
				if(staticBrightnessCustomThemeX > 1.00f){
					staticBrightnessCustomThemeX = 1.00f;
				}
			}
			as.aleLedsBrightnessCustomXupFlag = false;
		}

		if(as.aleLedsBrightnessCustomXDownFlag == true){	// brightness down
			if(as.ambientLightOnOffFlag == false){ // non-closed
				staticBrightnessCustomThemeX -= customBrightnessStep;
				if(staticBrightnessCustomThemeX <= 0.00f){
					staticBrightnessCustomThemeX = 0.00f;
				}
			}
			as.aleLedsBrightnessCustomXDownFlag = false;
		}
	}
	else{
		as.aleLedsBrightnessCustomXupFlag = false;
		as.aleLedsBrightnessCustomXDownFlag = false;
	}
	
	AnimationStation::options.alStaticColorBrightnessCustomX = staticColorBrightnessCustomX;	// save
	AnimationStation::options.alGradientBrightnessCustomX = gradientBrightnessCustomX;	// save
	AnimationStation::options.alChaseBrightnessCustomX = chaseBrightnessCustomX;	// save
	AnimationStation::options.alStaticBrightnessCustomThemeX = staticBrightnessCustomThemeX;	// save
	
	if(as.aleLedsParameterCustomUpFlag == true){	// parameters up
		if(as.ambientLightOnOffFlag == false){ // non-closed
			switch(ambientLightEffectsCount){
				case AL_CUSTOM_EFFECT_STATIC_COLOR:
					customStaticColorCount++;
					if(customStaticColorCount > AL_STATIC_COLOR_COUNT - 1){
						customStaticColorCount = 0;
					}
					break;

				case AL_CUSTOM_EFFECT_STATIC_THEME:
					customStaticThemeCount++;
					if(customStaticThemeCount > (AL_ROW - 1)){
						customStaticThemeCount = 0;
					}
					break;

				default:	
					break;
			}
		}
		as.aleLedsParameterCustomUpFlag = false;
	}

	if(as.aleLedsParameterCustomDownFlag == true){	// parameters down
		if(as.ambientLightOnOffFlag == false){ // non-closed
			switch(ambientLightEffectsCount){
				case AL_CUSTOM_EFFECT_STATIC_COLOR:
					customStaticColorCount--;
					if(customStaticColorCount < 0){
						customStaticColorCount = AL_STATIC_COLOR_COUNT - 1;
					}
					break;

				case AL_CUSTOM_EFFECT_STATIC_THEME:
					customStaticThemeCount--;
					if(customStaticThemeCount < 0){
						customStaticThemeCount = AL_ROW - 1;
					}
					break;

				default:	
					break;
			}
		}
		as.aleLedsParameterCustomDownFlag = false;
	}
	
	AnimationStation::options.alCustomStaticThemeIndex = customStaticThemeCount;	// save 
	AnimationStation::options.alCustomStaticColorIndex = customStaticColorCount;	// save 

	if(as.alGradientChaseBreathSpeedUpFlag == true){ // speed up
		if(as.ambientLightOnOffFlag == false){ // non-closed
			switch(ambientLightEffectsCount){
				case AL_CUSTOM_EFFECT_GRADIENT: 
					alFrameGradientSpeed ++;
					if(alFrameGradientSpeed > 6) alFrameGradientSpeed = 6;
					break;

				case AL_CUSTOM_EFFECT_CHASE: 
					alFrameChaseSpeed -= 20;
					if(alFrameChaseSpeed <= 0) alFrameChaseSpeed = 0;
					break;

				case AL_CUSTOM_EFFECT_BREATH: 
					alFrameBreathSpeed += 0.01f;
					if(alFrameBreathSpeed > 0.05f) alFrameBreathSpeed = 0.05f;
					break;
				
				default:
					break;
			}
		}

		as.alGradientChaseBreathSpeedUpFlag = false;
	}

	if(as.alGradientChaseBreathSpeedDownFlag == true){ // speed down
		if(as.ambientLightOnOffFlag == false){ //non-closed
			switch(ambientLightEffectsCount){
				case AL_CUSTOM_EFFECT_GRADIENT: 
					alFrameGradientSpeed --;
					if(alFrameGradientSpeed <= 2) alFrameGradientSpeed = 2;
					break;

				case AL_CUSTOM_EFFECT_CHASE: 
					alFrameChaseSpeed += 20;
					if(alFrameChaseSpeed >= 100) alFrameChaseSpeed = 100;
					break;

				case AL_CUSTOM_EFFECT_BREATH: 
					alFrameBreathSpeed -= 0.01f;
					if(alFrameBreathSpeed <= 0.01f) alFrameBreathSpeed = 0.01f;
					break;
				
				default:
					break;
			}
		}

		as.alGradientChaseBreathSpeedDownFlag = false;
	}
	AnimationStation::options.ambientLightGradientSpeed = alFrameGradientSpeed;
	AnimationStation::options.ambientLightChaseSpeed = alFrameChaseSpeed;
	AnimationStation::options.ambientLightBreathSpeed = alFrameBreathSpeed;

	switch(ambientLightEffectsCount)
	{
		case AL_CUSTOM_EFFECT_STATIC_COLOR: 
			customStaticColorIndex = customStaticColorCount;
			break;

		case AL_CUSTOM_EFFECT_GRADIENT: 
			alFrameToRGB = 255 - alCurrentFrame; // From 0 -> 255 to 255 -> 0
			if(alFrameToRGB < 85){ // Less than 85, transitions from red to yellow. The red component starts at 255 and gradually decreases, the green component always reaches 0, and the blue component starts at 0 and increases gradually.
				alR = 255 - alFrameToRGB * 3;
				alG = 0;
				alB = alFrameToRGB * 3;
			}
			else if(alFrameToRGB < 170){ // Between 85 and 170, there is a transition from yellow to cyan. The red component is always 0, the green component starts at 0 and gradually increases, and the blue component starts at 255 and gradually decreases.
				alFrameToRGB -= 85;
				alR = 0;
				alG = alFrameToRGB * 3;
				alB = 255 - alFrameToRGB * 3;
			} 
			else{ // When greater than or equal to 170, it transitions from cyan to blue. The red component is always 0, the green component starts at 255 and gradually decreases, and the blue component starts at 0 and gradually increases.
				alFrameToRGB -= 170;
				alR = alFrameToRGB * 3;
				alG = 255 - alFrameToRGB * 3; // Power-on animation chase
				alB = 0;
			}

			if(alReverse){
				alCurrentFrame -= alFrameGradientSpeed;
				if(alCurrentFrame < 0){
					alCurrentFrame = 1;
					alReverse = false;
				}
			}
			else{
				alCurrentFrame += alFrameGradientSpeed;
				if(alCurrentFrame > 255){
					alCurrentFrame = 254;
					alReverse = true;
				}
			}
			break;
		case AL_CUSTOM_EFFECT_CHASE: 
			if(!time_reached(nextRunTimeAmbientLight)){
				for(int j = 0; j < (FRAME_MAX - alStartIndex); j++){
					alFrame[alStartIndex + j] = 0x0;
				}
				
				if(chaseLightIndex < FRAME_MAX - CHASE_LIGHTS_TURN_ON - 1) {
					for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
						alFrame[chaseLightIndex + i] = ((uint32_t)(alG * chaseBrightnessCustomX) << 16) 
												     | ((uint32_t)(alR * chaseBrightnessCustomX) << 8) 
													 | (uint32_t)(alB * chaseBrightnessCustomX);
					}
				
					if((chaseLightIndex - totalOfButtonLedsCount) > CHASE_LIGHTS_INTERVAL){
						for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
							alFrame[chaseLightIndex - CHASE_LIGHTS_INTERVAL + i] = ((uint32_t)(alG * chaseBrightnessCustomX) << 16) 
																			   | ((uint32_t)(alR * chaseBrightnessCustomX) << 8) 
																			   | (uint32_t)(alB * chaseBrightnessCustomX);
						}
					}
				}
				
				return;
			}

			alFrameToRGB = 255 - alCurrentFrame; // 从 0 -> 255 变为 255 -> 0
			if(alFrameToRGB < 85){ // Less than 85, transitions from red to yellow. The red component starts at 255 and gradually decreases, the green component always reaches 0, and the blue component starts at 0 and increases gradually.
				alR = 255 - alFrameToRGB * 3;
				alG = 0;
				alB = alFrameToRGB * 3;
			}
			else if(alFrameToRGB < 170){ // Between 85 and 170, there is a transition from yellow to cyan. The red component is always 0, the green component starts at 0 and gradually increases, and the blue component starts at 255 and gradually decreases.
				alFrameToRGB -= 85;
				alR = 0;
				alG = alFrameToRGB * 3;
				alB = 255 - alFrameToRGB * 3;
			} 
			else{ // When greater than or equal to 170, it transitions from cyan to blue. The red component is always 0, the green component starts at 255 and gradually decreases, and the blue component starts at 0 and gradually increases.
				alFrameToRGB -= 170;
				alR = alFrameToRGB * 3;
				alG = 255 - alFrameToRGB * 3;
				alB = 0;
			}

			if(alReverse){
				alCurrentFrame -= alFrameSpeed;
				if(alCurrentFrame < 0){
					alCurrentFrame = 1;
					alReverse = false;
				}
			}
			else{
				alCurrentFrame += alFrameSpeed;
				if(alCurrentFrame > 255){
					alCurrentFrame = 254;
					alReverse = true;
				}
			}

			chaseLightIndex++;
			if(chaseLightIndex > chaseLightMaxIndexPos - CHASE_LIGHTS_TURN_ON - 1){
				chaseLightIndex = alStartIndex;
			}
			break;

		case AL_CUSTOM_EFFECT_BREATH:
			if(alReverse){
				alBrightnessBreathX += alFrameBreathSpeed;
				if(alBrightnessBreathX > 1.00f){
					alBrightnessBreathX = 1.00f;
					alReverse = false;
				}
			}
			else{
				alBrightnessBreathX -= alFrameBreathSpeed;
				if(alBrightnessBreathX < 0.00f){
					alBrightnessBreathX = 0.00f;
					alReverse = true;
					breathLedEffectCycle++;
				}
			}
			
			if(breathLedEffectCycle <= 1){
				alR = 255;
				alG = 0;
				alB = 255;
			}
			else if((breathLedEffectCycle > 1) && (breathLedEffectCycle <= 3)){
				alR = 255;
				alG = 0;
				alB = 0;
			}
			else if((breathLedEffectCycle > 3) && (breathLedEffectCycle <= 5)){
				alR = 0;
				alG = 255;
				alB = 0;
			}
			else if((breathLedEffectCycle > 5) && (breathLedEffectCycle <= 7)){
				alR = 0;
				alG = 0;
				alB = 255;
			}
			else{
				breathLedEffectCycle = 0;	
			}
			break;

		case AL_CUSTOM_EFFECT_STATIC_THEME:
			multipleOfCustomStaticThemeCount = (FRAME_MAX - totalOfButtonLedsCount) / AL_COL;
			remainderOfCustomStaticThemeCount = (FRAME_MAX - totalOfButtonLedsCount) % AL_COL;
			break;

		default:
			break;
	}

	if(as.ambientLightOnOffFlag == true){
		for(int i = 0; i < (FRAME_MAX - alStartIndex); i++){
			alFrame[alStartIndex + i] = 0x0;
		}
	}
	else{
		switch(ambientLightEffectsCount){
			case AL_CUSTOM_EFFECT_STATIC_COLOR:
				for(int i = 0; i < (FRAME_MAX - alStartIndex); i++){
					alFrame[alStartIndex + i] = alCustomStaticColors[customStaticColorIndex].value(Animation::format, staticColorBrightnessCustomX);
				}
				break;

			case AL_CUSTOM_EFFECT_GRADIENT:
				for(int i = 0; i < (FRAME_MAX - alStartIndex); i++){
					alFrame[alStartIndex + i] = ((uint32_t)(alG * gradientBrightnessCustomX) << 16) 
											  | ((uint32_t)(alR * gradientBrightnessCustomX) << 8) 
											  | (uint32_t)(alB * gradientBrightnessCustomX);
				}
				break;

			case AL_CUSTOM_EFFECT_CHASE:
				for(int j = 0; j < (FRAME_MAX - alStartIndex); j++){
					alFrame[alStartIndex + j] = 0x0;
				}

				if(chaseLightIndex < FRAME_MAX - CHASE_LIGHTS_TURN_ON - 1){
					for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
						alFrame[chaseLightIndex + i] = ((uint32_t)(alG * chaseBrightnessCustomX) << 16) 
													 | ((uint32_t)(alR * chaseBrightnessCustomX) << 8) 
													 | (uint32_t)(alB * chaseBrightnessCustomX);
					}
					
					if((chaseLightIndex - totalOfButtonLedsCount) > CHASE_LIGHTS_INTERVAL){
						for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
							alFrame[chaseLightIndex - CHASE_LIGHTS_INTERVAL + i] = ((uint32_t)(alG * chaseBrightnessCustomX) << 16) 
																			   | ((uint32_t)(alR * chaseBrightnessCustomX) << 8) 
																			   | (uint32_t)(alB * chaseBrightnessCustomX);
						}
					}
				}
				
				nextRunTimeAmbientLight = make_timeout_time_ms(alFrameChaseSpeed);
				break;

			case AL_CUSTOM_EFFECT_BREATH:
				for(int i = 0; i < (FRAME_MAX - alStartIndex); i++){
					alFrame[alStartIndex + i] = ((uint32_t)(alG * alBrightnessBreathX) << 16) 
											  | ((uint32_t)(alR * alBrightnessBreathX) << 8) 
											  | (uint32_t)(alB * alBrightnessBreathX);
				}
				break;

			case AL_CUSTOM_EFFECT_STATIC_THEME:
				for(int i = 0; i < multipleOfCustomStaticThemeCount; i++){
					for(int j = 0; j < AL_COL; j++){
						alFrame[alStartIndex + i*AL_COL + j] = alCustomStaticTheme[customStaticThemeCount][j].value(Animation::format, staticBrightnessCustomThemeX);
					}
				}

				if(remainderOfCustomStaticThemeCount != 0){
					for(int k = 0; k < remainderOfCustomStaticThemeCount; k++){
						alFrame[alStartIndex + multipleOfCustomStaticThemeCount * AL_COL + k] = alCustomStaticTheme[customStaticThemeCount][k].value(Animation::format, staticBrightnessCustomThemeX);
					}
				}
				break;

			default:
				break;
		}
	}
}

void NeoPicoLEDAddon::ambientLightLinkage(uint32_t alFrame[100]){
	uint8_t totalOfBaseAnimationLedsCount = this->ledCount;
	uint8_t alLinkageStartIndex = totalOfBaseAnimationLedsCount;
	uint8_t buttonLedsCount = totalOfBaseAnimationLedsCount; 
	uint8_t multipleOfButtonLedsCount;
	uint8_t remainderOfButtonLedsCount;

	uint8_t powerOn_R = 0;
	uint8_t powerOn_G = 0;
	uint8_t powerOn_B = 255;
	uint8_t powerOnStartIndex;

	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	float preLinkageBrightnessX = AnimationStation::GetLinkageModeOfBrightnessX();
	AnimationEffects preEffect = static_cast<AnimationEffects>(AnimationStation::options.baseAnimationIndex);

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
		buttonLedsCount = totalOfBaseAnimationLedsCount - PLED_COUNT;
	}

	#if POWER_ON_EFFECT == 1	// Power-On Animation Chase
	uint8_t chasePowerOnLightMaxIndexPos = 0;
	uint8_t buttonCount = setupButtonPositions();

	if(ledOptions.ledsPerButton == 1){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*1 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 2){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT;
		}
		else{
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*2 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else if(ledOptions.ledsPerButton == 3){
		if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0){
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM + PLED_COUNT; // max 96
		}
		else{
			powerOnStartIndex = buttonLedsCount;
			chasePowerOnLightMaxIndexPos = buttonCount*3 + POWER_ON_CHASE_FRAME_MAX + RESERVE_DUMMY_LEDS_NUM;
		}
	}
	else{
		powerOnStartIndex = buttonLedsCount;	
		chasePowerOnLightMaxIndexPos = FRAME_MAX - 1;	
	}
	
	static uint8_t chasePowerOnIndex = powerOnStartIndex;
	uint32_t now = to_ms_since_boot(get_absolute_time());
	
	if(now < POWER_ON_EFFECT_DURATION){
		for(int j = 0; j < (FRAME_MAX - powerOnStartIndex); j++){
			alFrame[powerOnStartIndex + j] = 0x0;
		}

		if(chasePowerOnIndex < (FRAME_MAX - CHASE_LIGHTS_TURN_ON - 1)){
			for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
				alFrame[chasePowerOnIndex + i] = ((uint32_t)(powerOn_G * POWER_ON_CHASE_BRIGHTNESSX) << 16) 
													  | ((uint32_t)(powerOn_R * POWER_ON_CHASE_BRIGHTNESSX) << 8) 
													  | (uint32_t)(powerOn_B * POWER_ON_CHASE_BRIGHTNESSX);
			}
				
			if((chasePowerOnIndex - powerOnStartIndex) > CHASE_LIGHTS_INTERVAL){
				for(int i = 0; i < CHASE_LIGHTS_TURN_ON; i++){
					alFrame[chasePowerOnIndex - CHASE_LIGHTS_INTERVAL + i] = ((uint32_t)(powerOn_G * POWER_ON_CHASE_BRIGHTNESSX) << 16) 
																			    | ((uint32_t)(powerOn_R * POWER_ON_CHASE_BRIGHTNESSX) << 8) 
																				| (uint32_t)(powerOn_B * POWER_ON_CHASE_BRIGHTNESSX);
				}
			}
		}

		chasePowerOnIndex++;
		if(chasePowerOnIndex > (chasePowerOnLightMaxIndexPos - CHASE_LIGHTS_TURN_ON - 1)){
			chasePowerOnIndex = powerOnStartIndex;
		}

		return;
	}
	#else	// Power-On Animation Breathing Lights
	static float alLinkagePowerOnBrightnessY = 0.0f;
	static bool alLinkagePowerOnReverse = true;

	powerOnStartIndex = buttonLedsCount;

	uint32_t now = to_ms_since_boot(get_absolute_time());

	if(now < POWER_ON_EFFECT_DURATION){ 
		powerOn_R = 0;
		powerOn_G = 0;
		powerOn_B = 255;

		if((now > POWER_ON_EFFECT_DURATION) && (alLinkagePowerOnBrightnessY == 0.0f) && (alLinkagePowerOnReverse == true)){
			alLinkagePowerOnBrightnessY = 0.0f;
			alLinkagePowerOnReverse = true;
			return;
		}

		if(alLinkagePowerOnReverse){
			alLinkagePowerOnBrightnessY += 0.1f;
			if(alLinkagePowerOnBrightnessY > 1.0f){
				alLinkagePowerOnBrightnessY = 1.0f;
				alLinkagePowerOnReverse = false;
			}
		}
		else{
			alLinkagePowerOnBrightnessY -= 0.1f;
			if(alLinkagePowerOnBrightnessY < 0.0f){
				alLinkagePowerOnBrightnessY = 0.0f;
				alLinkagePowerOnReverse = true;
			}
		}

		for(int i = 0; i < (FRAME_MAX - powerOnStartIndex); i++){
			alFrame[powerOnStartIndex + i] = ((uint32_t)(powerOn_G * alLinkagePowerOnBrightnessY) << 16) 
									  | ((uint32_t)(powerOn_R * alLinkagePowerOnBrightnessY) << 8) 
									  | (uint32_t)(powerOn_B * alLinkagePowerOnBrightnessY);
		}
		
		return;
	}
	#endif

	switch(preEffect){
		case AnimationEffects::EFFECT_STATIC_COLOR:
		case AnimationEffects::EFFECT_RAINBOW:
		case AnimationEffects::EFFECT_CUSTOM_THEME:
			break;
	
		case AnimationEffects::EFFECT_CHASE:
		case AnimationEffects::EFFECT_STATIC_THEME:
			buttonLedsCount = buttonLedsCount - 4;
			break;

		default:
			break;
	}

	multipleOfButtonLedsCount = (FRAME_MAX - alLinkageStartIndex) / buttonLedsCount;
	remainderOfButtonLedsCount = (FRAME_MAX - alLinkageStartIndex) % buttonLedsCount;

	if(as.ambientLightLinlageOnOffFlag == true){
		for(int i = 0; i < (FRAME_MAX - alLinkageStartIndex); i++){
			alFrame[alLinkageStartIndex + i] = 0x0;
		}
	}
	else{
		for(int i = 0; i < multipleOfButtonLedsCount; i++){
			for(int j = 0; j < buttonLedsCount; j++){
				alFrame[alLinkageStartIndex + i*buttonLedsCount + j] = as.linkageFrame[j].value(Animation::format, preLinkageBrightnessX);
			}
		}

		if(remainderOfButtonLedsCount != 0){
			for(int k = 0; k < remainderOfButtonLedsCount; k++){
				alFrame[alLinkageStartIndex + multipleOfButtonLedsCount * buttonLedsCount + k] = as.linkageFrame[k].value(Animation::format, preLinkageBrightnessX);
			}
		}
	}
}

void NeoPicoLEDAddon::process()
{
    const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
    if (!isValidPin(ledOptions.dataPin) || !time_reached(this->nextRunTime))
        return;

    // Get turbo options (turbo RGB led)
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    AnimationHotkey action = animationHotkeys(gamepad);
    AnimationOptions animationOptions = AnimationStore.getAnimationOptions();
	bool CustomLinkageMode = animationOptions.ambientLightCustomLinkageModeFlag;
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
        int32_t pledIndexes[] = { ledOptions.pledIndex1, ledOptions.pledIndex2, ledOptions.pledIndex3, ledOptions.pledIndex4 };
        for (int i = 0; i < PLED_COUNT; i++) {
            if (pledIndexes[i] < 0 || pledIndexes[i] > 99)
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

    // Turbo LED is a separate RGB that is on if turbo is on, and off if its off
    if ( turboOptions.turboLedType == PLED_TYPE_RGB ) { // RGB or PWM?
        if ( gamepad->auxState.turbo.activity == 1) { // Turbo is on (active sensor)
            if (turboOptions.turboLedIndex >= 0 && turboOptions.turboLedIndex < 100) { // Double check index value
                float brightness = as.GetBrightnessX();
                frame[turboOptions.turboLedIndex] = ((RGB)turboOptions.turboLedColor).value(neopico->GetFormat(), brightness);
            }
        }
    }

    // Case RGB LEDs for a single static color go here
    if ( ledOptions.caseRGBType == CASE_RGB_TYPE_STATIC &&
        ledOptions.caseRGBIndex >= 0 &&
        ledOptions.caseRGBCount > 0 ) {
        float brightness = as.GetBrightnessX();
        uint32_t colorVal = ((RGB)ledOptions.caseRGBColor).value(neopico->GetFormat(), brightness);
        for(int i = 0; i < ledOptions.caseRGBCount; i++) {
            frame[ledOptions.caseRGBIndex+i] = colorVal;
        }
    }

    // Merge with custom lighting linkage here
    if(as.alCustomLinkageModeFlag == true){
		CustomLinkageMode = !CustomLinkageMode;
		as.alCustomLinkageModeFlag = false;
	} 

	if(CustomLinkageMode == true){
		this->ambientLightLinkage(frame); // Linkage mode
	}
	else{
		this->ambientLightCustom(frame); //Custom mode
	}

	AnimationStation::options.ambientLightCustomLinkageModeFlag = CustomLinkageMode;	// save

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
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
    uint8_t buttonCount = setupButtonPositions();
    vector<vector<Pixel>> pixels = createLEDLayout(static_cast<ButtonLayout>(ledOptions.ledLayout), ledOptions.ledsPerButton, buttonCount);
    matrix.setup(pixels, ledOptions.ledsPerButton);
    ledCount = matrix.getLedCount();
    if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0)
        ledCount += PLED_COUNT;

    if (turboOptions.turboLedType == PLED_TYPE_RGB)
        ledCount += 1;

    if (ledOptions.caseRGBType == CASE_RGB_TYPE_STATIC ) {
        ledCount += ledOptions.caseRGBCount;
    }

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

    if(gamepad->pressedS2() && (gamepad->pressedS1() == false)) // start (not start+back) 
	{ 
		if(gamepad->pressedL1()) // LB
		{ 
			action = AMBIENT_LIGHT_EFFECTS_CHANGE;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L1 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedL2()) // LT
		{
			action = AMBIENT_LIGHT_EFFECTS_ON_OFF;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L2 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedB4() && (gamepad->pressedB3() == false)) // Y  not X+Y
		{
			action = AMBIENT_LIGHT_EFFECTS_BRIGHTNESS_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B4 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedB2()) // B
		{
			action = AMBIENT_LIGHT_EFFECTS_BRIGHTNESS_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B2 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedR1()) // RB
		{
			action = AMBIENT_LIGHT_EFFECTS_PARAMETER_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R1 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedR2()) // RT
		{
			action = AMBIENT_LIGHT_EFFECTS_PARAMETER_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R2 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedL3()) // LS
		{
			action = AMBIENT_LIGHT_EFFECTS_FRAME_SPEED_UP;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_L3 | GAMEPAD_MASK_S2);
		}
		else if(gamepad->pressedR3()) // RS
		{
			action = AMBIENT_LIGHT_EFFECTS_FRAME_SPEED_DOWN;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_R3 | GAMEPAD_MASK_S2);
		}
		else if((gamepad->pressedB1()) && (gamepad->pressedB3())) // A + X
		{
			action = AMBIENT_LIGHT_EFFECTS_CUSTOM_LINKAGE;
			gamepad->state.buttons &= ~(GAMEPAD_MASK_B1 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_S2);
		}
	}

    return action;
}
