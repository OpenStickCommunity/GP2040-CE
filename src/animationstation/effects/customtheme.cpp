#include "customtheme.h"
#include "storagemanager.h"

CustomTheme::CustomTheme(PixelMatrix &matrix) : Animation(matrix) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUp);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDown);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeft);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRight);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3);
	}
}

bool CustomTheme::Animate(RGB (&frame)[100]) {
  UpdateTime();
  UpdatePresses(frame);

  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      // Count down the timer
      DecrementFadeCounter(matrix->pixels[r][c].index);

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

  return true;
}

bool CustomTheme::HasTheme() {
  return theme.size() > 0;
}

void CustomTheme::ParameterUp() {
}

void CustomTheme::ParameterDown() {
}
