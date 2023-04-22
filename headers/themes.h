/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef LED_THEMES_H_
#define LED_THEMES_H_

#include "BoardConfig.h"
#include <vector>
#include "AnimationStation.hpp"
#include "helper.h"

using namespace std;

static map<uint32_t, RGB> themeStaticRainbow({
	{ GAMEPAD_MASK_DL, ColorRed },
	{ GAMEPAD_MASK_DD, ColorOrange },
	{ GAMEPAD_MASK_DR, ColorYellow },
	{ GAMEPAD_MASK_DU, ColorOrange },
	{ GAMEPAD_MASK_B3, ColorGreen },
	{ GAMEPAD_MASK_B1, ColorGreen },
	{ GAMEPAD_MASK_B4, ColorAqua },
	{ GAMEPAD_MASK_B2, ColorAqua },
	{ GAMEPAD_MASK_R1, ColorBlue },
	{ GAMEPAD_MASK_R2, ColorBlue },
	{ GAMEPAD_MASK_L1, ColorMagenta },
	{ GAMEPAD_MASK_L2, ColorMagenta },
});

static map<uint32_t, RGB> themeGuiltyGearTypeA({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorPink },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_R1, ColorRed },
	{ GAMEPAD_MASK_R2, ColorOrange },
});

static map<uint32_t, RGB> themeGuiltyGearTypeB({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorRed },
	{ GAMEPAD_MASK_B3, ColorPink },
	{ GAMEPAD_MASK_B4, ColorBlue },
	{ GAMEPAD_MASK_R1, ColorGreen },
	{ GAMEPAD_MASK_R2, ColorOrange },
});

static map<uint32_t, RGB> themeGuiltyGearTypeC({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorOrange },
	{ GAMEPAD_MASK_B3, ColorPink },
	{ GAMEPAD_MASK_B4, ColorBlue },
	{ GAMEPAD_MASK_R1, ColorGreen },
	{ GAMEPAD_MASK_R2, ColorRed },
});

static map<uint32_t, RGB> themeGuiltyGearTypeD({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorPink },
	{ GAMEPAD_MASK_B1, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_R1, ColorOrange },
});

static map<uint32_t, RGB> themeGuiltyGearTypeE({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorPink },
	{ GAMEPAD_MASK_B1, ColorGreen },
	{ GAMEPAD_MASK_B4, ColorBlue },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_R1, ColorOrange },
});

static map<uint32_t, RGB> themeNeoGeo({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorRed },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorGreen },
	{ GAMEPAD_MASK_L1, ColorBlue },
});

static map<uint32_t, RGB> themeNeoGeoCurved({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorRed },
	{ GAMEPAD_MASK_B3, ColorYellow },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_R1, ColorBlue },
});

static map<uint32_t, RGB> themeNeoGeoModern({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorRed },
	{ GAMEPAD_MASK_B1, ColorYellow },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorBlue },
});

static map<uint32_t, RGB> themeSixButtonFighter({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B1, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_B2, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorRed },
	{ GAMEPAD_MASK_R2, ColorRed },
});

static map<uint32_t, RGB> themeSixButtonFighterPlus({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B1, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_B2, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorRed },
	{ GAMEPAD_MASK_R2, ColorRed },
	{ GAMEPAD_MASK_L1, ColorGreen },
	{ GAMEPAD_MASK_L2, ColorGreen },
});

static map<uint32_t, RGB> themeStreetFighter2({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorRed },
	{ GAMEPAD_MASK_B1, ColorRed },
	{ GAMEPAD_MASK_B4, ColorWhite },
	{ GAMEPAD_MASK_B2, ColorWhite },
	{ GAMEPAD_MASK_R1, ColorBlue },
	{ GAMEPAD_MASK_R2, ColorBlue },
	{ GAMEPAD_MASK_L1, ColorBlack },
	{ GAMEPAD_MASK_L2, ColorBlack },
});

static map<uint32_t, RGB> themeTekken({
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_B3, ColorYellow },
	{ GAMEPAD_MASK_B1, ColorAqua },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorPink },
	{ GAMEPAD_MASK_R1, ColorRed },
});

static map<uint32_t, RGB> themePlayStation({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorBlue },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorMagenta },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_R1, ColorBlack },
	{ GAMEPAD_MASK_R2, ColorBlack },
	{ GAMEPAD_MASK_L1, ColorBlack },
	{ GAMEPAD_MASK_L2, ColorBlack },
});

static map<uint32_t, RGB> themePlayStationAll({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorBlue },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorMagenta },
	{ GAMEPAD_MASK_B4, ColorGreen },
	{ GAMEPAD_MASK_R1, ColorWhite },
	{ GAMEPAD_MASK_R2, ColorWhite },
	{ GAMEPAD_MASK_L1, ColorWhite },
	{ GAMEPAD_MASK_L2, ColorWhite },
});

static map<uint32_t, RGB> themeSuperFamicom({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorYellow },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorGreen },
	{ GAMEPAD_MASK_B4, ColorBlue },
	{ GAMEPAD_MASK_R1, ColorBlack },
	{ GAMEPAD_MASK_R2, ColorBlack },
	{ GAMEPAD_MASK_L1, ColorBlack },
	{ GAMEPAD_MASK_L2, ColorBlack },
});

static map<uint32_t, RGB> themeSuperFamicomAll({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorYellow },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorGreen },
	{ GAMEPAD_MASK_B4, ColorBlue },
	{ GAMEPAD_MASK_R1, ColorWhite },
	{ GAMEPAD_MASK_R2, ColorWhite },
	{ GAMEPAD_MASK_L1, ColorWhite },
	{ GAMEPAD_MASK_L2, ColorWhite },
});

static map<uint32_t, RGB> themeXbox({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorBlack },
	{ GAMEPAD_MASK_R2, ColorBlack },
	{ GAMEPAD_MASK_L1, ColorBlack },
	{ GAMEPAD_MASK_L2, ColorBlack },
});

static map<uint32_t, RGB> themeXboxAll({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorWhite },
	{ GAMEPAD_MASK_R2, ColorWhite },
	{ GAMEPAD_MASK_L1, ColorWhite },
	{ GAMEPAD_MASK_L2, ColorWhite },
});

static map<uint32_t, RGB> themeFightboard({
	{ GAMEPAD_MASK_DU, ColorWhite },
	{ GAMEPAD_MASK_DD, ColorWhite },
	{ GAMEPAD_MASK_DL, ColorWhite },
	{ GAMEPAD_MASK_DR, ColorWhite },
	{ GAMEPAD_MASK_B1, ColorGreen },
	{ GAMEPAD_MASK_B2, ColorRed },
	{ GAMEPAD_MASK_B3, ColorBlue },
	{ GAMEPAD_MASK_B4, ColorYellow },
	{ GAMEPAD_MASK_R1, ColorPurple },
	{ GAMEPAD_MASK_R2, ColorAqua },
	{ GAMEPAD_MASK_L1, ColorOrange },
	{ GAMEPAD_MASK_L2, ColorPink },
});

static map<uint32_t, RGB> customTheme;
static map<uint32_t, RGB> customThemePressed;

void addStaticThemes(LEDOptions options)
{
	// Rainbow theme on a Stickless layout should use green for up button
	themeStaticRainbow[GAMEPAD_MASK_DU] = (options.ledLayout == BUTTON_LAYOUT_STICKLESS) ? ColorGreen : ColorOrange;

	StaticTheme::ClearThemes();

	StaticTheme::AddTheme(themeStaticRainbow);

	StaticTheme::AddTheme(themeXbox);
	StaticTheme::AddTheme(themeXboxAll);
	StaticTheme::AddTheme(themeSuperFamicom);
	StaticTheme::AddTheme(themeSuperFamicomAll);
	StaticTheme::AddTheme(themePlayStation);
	StaticTheme::AddTheme(themePlayStationAll);

	StaticTheme::AddTheme(themeNeoGeo);
	StaticTheme::AddTheme(themeNeoGeoCurved);
	StaticTheme::AddTheme(themeNeoGeoModern);
	StaticTheme::AddTheme(themeSixButtonFighter);
	StaticTheme::AddTheme(themeSixButtonFighterPlus);

	StaticTheme::AddTheme(themeStreetFighter2);
	StaticTheme::AddTheme(themeTekken);
	StaticTheme::AddTheme(themeGuiltyGearTypeA);
	StaticTheme::AddTheme(themeGuiltyGearTypeB);
	StaticTheme::AddTheme(themeGuiltyGearTypeC);
	StaticTheme::AddTheme(themeGuiltyGearTypeD);
	StaticTheme::AddTheme(themeGuiltyGearTypeE);

	StaticTheme::AddTheme(themeFightboard);

	if (options.useCustomLeds)
	{
		customTheme[GAMEPAD_MASK_DU] = RGB(options.customColorUp);
		customTheme[GAMEPAD_MASK_DD] = RGB(options.customColorDown);
		customTheme[GAMEPAD_MASK_DL] = RGB(options.customColorLeft);
		customTheme[GAMEPAD_MASK_DR] = RGB(options.customColorRight);
		customTheme[GAMEPAD_MASK_B1] = RGB(options.customColorB1);
		customTheme[GAMEPAD_MASK_B2] = RGB(options.customColorB2);
		customTheme[GAMEPAD_MASK_B3] = RGB(options.customColorB3);
		customTheme[GAMEPAD_MASK_B4] = RGB(options.customColorB4);
		customTheme[GAMEPAD_MASK_L1] = RGB(options.customColorL1);
		customTheme[GAMEPAD_MASK_R1] = RGB(options.customColorR1);
		customTheme[GAMEPAD_MASK_L2] = RGB(options.customColorL2);
		customTheme[GAMEPAD_MASK_R2] = RGB(options.customColorR2);
		customTheme[GAMEPAD_MASK_S1] = RGB(options.customColorS1);
		customTheme[GAMEPAD_MASK_S2] = RGB(options.customColorS2);
		customTheme[GAMEPAD_MASK_A1] = RGB(options.customColorA1);
		customTheme[GAMEPAD_MASK_A2] = RGB(options.customColorA2);
		customTheme[GAMEPAD_MASK_L3] = RGB(options.customColorL3);
		customTheme[GAMEPAD_MASK_R3] = RGB(options.customColorR3);
		CustomTheme::SetCustomTheme(customTheme);

		customThemePressed[GAMEPAD_MASK_DU] = RGB(options.customColorUpPressed);
		customThemePressed[GAMEPAD_MASK_DD] = RGB(options.customColorDownPressed);
		customThemePressed[GAMEPAD_MASK_DL] = RGB(options.customColorLeftPressed);
		customThemePressed[GAMEPAD_MASK_DR] = RGB(options.customColorRightPressed);
		customThemePressed[GAMEPAD_MASK_B1] = RGB(options.customColorB1Pressed);
		customThemePressed[GAMEPAD_MASK_B2] = RGB(options.customColorB2Pressed);
		customThemePressed[GAMEPAD_MASK_B3] = RGB(options.customColorB3Pressed);
		customThemePressed[GAMEPAD_MASK_B4] = RGB(options.customColorB4Pressed);
		customThemePressed[GAMEPAD_MASK_L1] = RGB(options.customColorL1Pressed);
		customThemePressed[GAMEPAD_MASK_R1] = RGB(options.customColorR1Pressed);
		customThemePressed[GAMEPAD_MASK_L2] = RGB(options.customColorL2Pressed);
		customThemePressed[GAMEPAD_MASK_R2] = RGB(options.customColorR2Pressed);
		customThemePressed[GAMEPAD_MASK_S1] = RGB(options.customColorS1Pressed);
		customThemePressed[GAMEPAD_MASK_S2] = RGB(options.customColorS2Pressed);
		customThemePressed[GAMEPAD_MASK_A1] = RGB(options.customColorA1Pressed);
		customThemePressed[GAMEPAD_MASK_A2] = RGB(options.customColorA2Pressed);
		customThemePressed[GAMEPAD_MASK_L3] = RGB(options.customColorL3Pressed);
		customThemePressed[GAMEPAD_MASK_R3] = RGB(options.customColorR3Pressed);
		CustomThemePressed::SetCustomTheme(customThemePressed);
	}
}

#endif
