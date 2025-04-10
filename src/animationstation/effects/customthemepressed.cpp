#include "customthemepressed.h"
#include "GamepadState.h"
#include "storagemanager.h"

CustomThemePressed::CustomThemePressed(PixelMatrix &matrix) : Animation(matrix) {
  this->filtered = true;

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUpPressed);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDownPressed);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeftPressed);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRightPressed);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1Pressed);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2Pressed);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3Pressed);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4Pressed);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1Pressed);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1Pressed);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2Pressed);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2Pressed);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1Pressed);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2Pressed);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1Pressed);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2Pressed);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3Pressed);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3Pressed);
	}
}

CustomThemePressed::CustomThemePressed(PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pixels(&pixels) {
  this->filtered = true;

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUpPressed);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDownPressed);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeftPressed);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRightPressed);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1Pressed);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2Pressed);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3Pressed);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4Pressed);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1Pressed);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1Pressed);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2Pressed);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2Pressed);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1Pressed);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2Pressed);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1Pressed);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2Pressed);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3Pressed);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3Pressed);
	}
}

bool CustomThemePressed::Animate(RGB (&frame)[100]) {
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index || this->notInFilter(matrix->pixels[r][c]))
        continue;

      auto itr = theme.find(matrix->pixels[r][c].mask);
      if (itr != theme.end())
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++)
          frame[matrix->pixels[r][c].positions[p]] = itr->second;
      else
        for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++)
          frame[matrix->pixels[r][c].positions[p]] = defaultColor;
    }
  }
  return true;
}

bool CustomThemePressed::HasTheme() {
  return theme.size() > 0;
}
