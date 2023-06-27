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

#include "config.pb.h"

using namespace std;

void addStaticThemes(const LEDOptions& options, const AnimationOptions& animationOptions)
{
	map<uint32_t, RGB> themeStaticRainbow({
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

	const map<uint32_t, RGB> themeGuiltyGearTypeA({
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

	const map<uint32_t, RGB> themeGuiltyGearTypeB({
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

	const map<uint32_t, RGB> themeGuiltyGearTypeC({
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

	const map<uint32_t, RGB> themeGuiltyGearTypeD({
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

	const map<uint32_t, RGB> themeGuiltyGearTypeE({
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

	const map<uint32_t, RGB> themeNeoGeo({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B3, ColorRed },
		{ GAMEPAD_MASK_B4, ColorYellow },
		{ GAMEPAD_MASK_R1, ColorGreen },
		{ GAMEPAD_MASK_L1, ColorBlue },
	});

	const map<uint32_t, RGB> themeNeoGeoCurved({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B1, ColorRed },
		{ GAMEPAD_MASK_B3, ColorYellow },
		{ GAMEPAD_MASK_B4, ColorGreen },
		{ GAMEPAD_MASK_R1, ColorBlue },
	});

	const map<uint32_t, RGB> themeNeoGeoModern({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B3, ColorRed },
		{ GAMEPAD_MASK_B1, ColorYellow },
		{ GAMEPAD_MASK_B4, ColorGreen },
		{ GAMEPAD_MASK_B2, ColorBlue },
	});

	const map<uint32_t, RGB> themeSixButtonFighter({
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

	const map<uint32_t, RGB> themeSixButtonFighterPlus({
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

	const map<uint32_t, RGB> themeStreetFighter2({
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

	const map<uint32_t, RGB> themeTekken({
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

	const map<uint32_t, RGB> themePlayStation({
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

	const map<uint32_t, RGB> themePlayStationAll({
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

	const map<uint32_t, RGB> themeSuperFamicom({
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

	const map<uint32_t, RGB> themeSuperFamicomAll({
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

	const map<uint32_t, RGB> themeXbox({
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

	const map<uint32_t, RGB> themeXboxAll({
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

	const map<uint32_t, RGB> themeFightboard({
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

	if (animationOptions.hasCustomTheme)
	{
		map<uint32_t, RGB> customTheme;
		customTheme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUp);
		customTheme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDown);
		customTheme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeft);
		customTheme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRight);
		customTheme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1);
		customTheme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2);
		customTheme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3);
		customTheme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4);
		customTheme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1);
		customTheme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1);
		customTheme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2);
		customTheme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2);
		customTheme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1);
		customTheme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2);
		customTheme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1);
		customTheme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2);
		customTheme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3);
		customTheme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3);
		CustomTheme::SetCustomTheme(customTheme);

		map<uint32_t, RGB> customThemePressed;
		customThemePressed[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUpPressed);
		customThemePressed[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDownPressed);
		customThemePressed[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeftPressed);
		customThemePressed[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRightPressed);
		customThemePressed[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1Pressed);
		customThemePressed[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2Pressed);
		customThemePressed[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3Pressed);
		customThemePressed[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4Pressed);
		customThemePressed[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1Pressed);
		customThemePressed[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1Pressed);
		customThemePressed[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2Pressed);
		customThemePressed[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2Pressed);
		customThemePressed[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1Pressed);
		customThemePressed[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2Pressed);
		customThemePressed[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1Pressed);
		customThemePressed[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2Pressed);
		customThemePressed[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3Pressed);
		customThemePressed[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3Pressed);
		CustomThemePressed::SetCustomTheme(customThemePressed);
	}
}

#endif
