#include "statictheme.h"
#include "storagemanager.h"
#include "enums.pb.h"

StaticTheme::StaticTheme(PixelMatrix &matrix) : Animation(matrix) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.themeIndex >= themes.size()) {
    animationOptions.themeIndex = 0;
  }

	const std::map<uint32_t, RGB> themeGuiltyGearTypeA({
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

	const std::map<uint32_t, RGB> themeGuiltyGearTypeB({
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

	const std::map<uint32_t, RGB> themeGuiltyGearTypeC({
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

	const std::map<uint32_t, RGB> themeGuiltyGearTypeD({
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

	const std::map<uint32_t, RGB> themeGuiltyGearTypeE({
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

	const std::map<uint32_t, RGB> themeNeoGeo({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B3, ColorRed },
		{ GAMEPAD_MASK_B4, ColorYellow },
		{ GAMEPAD_MASK_R1, ColorGreen },
		{ GAMEPAD_MASK_L1, ColorBlue },
	});

	const std::map<uint32_t, RGB> themeNeoGeoCurved({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B1, ColorRed },
		{ GAMEPAD_MASK_B3, ColorYellow },
		{ GAMEPAD_MASK_B4, ColorGreen },
		{ GAMEPAD_MASK_R1, ColorBlue },
	});

	const std::map<uint32_t, RGB> themeNeoGeoModern({
		{ GAMEPAD_MASK_DU, ColorWhite },
		{ GAMEPAD_MASK_DD, ColorWhite },
		{ GAMEPAD_MASK_DL, ColorWhite },
		{ GAMEPAD_MASK_DR, ColorWhite },
		{ GAMEPAD_MASK_B3, ColorRed },
		{ GAMEPAD_MASK_B1, ColorYellow },
		{ GAMEPAD_MASK_B4, ColorGreen },
		{ GAMEPAD_MASK_B2, ColorBlue },
	});

	const std::map<uint32_t, RGB> themeSixButtonFighter({
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

	const std::map<uint32_t, RGB> themeSixButtonFighterPlus({
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

	const std::map<uint32_t, RGB> themeStreetFighter2({
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

	const std::map<uint32_t, RGB> themeTekken({
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

	const std::map<uint32_t, RGB> themePlayStation({
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

	const std::map<uint32_t, RGB> themePlayStationAll({
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

	const std::map<uint32_t, RGB> themeSuperFamicom({
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

	const std::map<uint32_t, RGB> themeSuperFamicomAll({
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

	const std::map<uint32_t, RGB> themeXbox({
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

	const std::map<uint32_t, RGB> themeXboxAll({
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

	const std::map<uint32_t, RGB> themeFightboard({
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
  std::map<uint32_t, RGB> themeStaticRainbow({
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

  LEDOptions & ledOptions = Storage::getInstance().getLedOptions();
	themeStaticRainbow[GAMEPAD_MASK_DU] = (ledOptions.ledLayout == BUTTON_LAYOUT_STICKLESS) ? ColorGreen : ColorOrange;

	ClearThemes();
	AddTheme(themeStaticRainbow);
	AddTheme(themeXbox);
	AddTheme(themeXboxAll);
	AddTheme(themeSuperFamicom);
	AddTheme(themeSuperFamicomAll);
	AddTheme(themePlayStation);
	AddTheme(themePlayStationAll);
	AddTheme(themeNeoGeo);
	AddTheme(themeNeoGeoCurved);
	AddTheme(themeNeoGeoModern);
	AddTheme(themeSixButtonFighter);
	AddTheme(themeSixButtonFighterPlus);
	AddTheme(themeStreetFighter2);
	AddTheme(themeTekken);
	AddTheme(themeGuiltyGearTypeA);
	AddTheme(themeGuiltyGearTypeB);
	AddTheme(themeGuiltyGearTypeC);
	AddTheme(themeGuiltyGearTypeD);
	AddTheme(themeGuiltyGearTypeE);
	AddTheme(themeFightboard);
}

bool StaticTheme::Animate(RGB (&frame)[100]) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (themes.size() > 0) {
    UpdateTime();
    UpdatePresses(frame);

    for (size_t r = 0; r != matrix->pixels.size(); r++) {
      for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
        if (matrix->pixels[r][c].index == NO_PIXEL.index)
          continue;

        // Count down the timer
        DecrementFadeCounter(matrix->pixels[r][c].index);

        std::map<uint32_t, RGB> theme = themes.at(animationOptions.themeIndex);

        auto itr = theme.find(matrix->pixels[r][c].mask);
        if (itr != theme.end()) {
          for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
            // Interpolate from hitColor (color the button was assigned when pressed) back to the theme color
            frame[matrix->pixels[r][c].positions[p]] = BlendColor(hitColor[matrix->pixels[r][c].index], itr->second, times[matrix->pixels[r][c].index]);
          }
        } else {
          for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
            frame[matrix->pixels[r][c].positions[p]] = defaultColor;
          }
        }
      }
    }
  }
  return true;
}

void StaticTheme::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.themeIndex < StaticTheme::themes.size() - 1) {
    animationOptions.themeIndex++;
  } else {
    animationOptions.themeIndex = 0;
  }
}

void StaticTheme::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.themeIndex > 0) {
    animationOptions.themeIndex--;
  } else {
    animationOptions.themeIndex = StaticTheme::themes.size() - 1;
  }
}
