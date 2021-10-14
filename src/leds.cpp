/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "BoardConfig.h"

#ifdef BOARD_LEDS_PIN

#include "leds.h"
#include "themes.h"

void configureAnimations(AnimationStation *as)
{
	as->AddAnimation(new StaticColor(matrix, ColorBlack));
	as->AddAnimation(new StaticColor(matrix, LEDS_STATIC_COLOR_COLOR));
	as->AddAnimation(new Rainbow(matrix, LEDS_RAINBOW_CYCLE_TIME));
	as->AddAnimation(new Chase(matrix, LEDS_CHASE_CYCLE_TIME));

	for (size_t i = 0; i < customThemes.size(); i++)
		as->AddAnimation(&customThemes[i]);
}

AnimationHotkey animationHotkeys(MPG *gamepad)
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
	}

	return action;
}

PixelMatrix createStandardLayout(std::vector<uint8_t> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			NO_PIXEL,
			Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL, positions[LEDS_DPAD_LEFT]),
		},
		{
			Pixel(LEDS_DPAD_UP,   GAMEPAD_MASK_DU, positions[LEDS_DPAD_UP]),
			Pixel(LEDS_DPAD_DOWN, GAMEPAD_MASK_DD, positions[LEDS_DPAD_DOWN]),
		},
		{
			NO_PIXEL,
			Pixel(LEDS_DPAD_RIGHT, GAMEPAD_MASK_DR, positions[LEDS_DPAD_RIGHT]),
		},
		{
			Pixel(LEDS_BUTTON_B3, GAMEPAD_MASK_B3, positions[LEDS_BUTTON_B3]),
			Pixel(LEDS_BUTTON_B1, GAMEPAD_MASK_B1, positions[LEDS_BUTTON_B1]),
		},
		{
			Pixel(LEDS_BUTTON_B4, GAMEPAD_MASK_B4, positions[LEDS_BUTTON_B4]),
			Pixel(LEDS_BUTTON_B2, GAMEPAD_MASK_B2, positions[LEDS_BUTTON_B2]),
		},
		{
			Pixel(LEDS_BUTTON_R1, GAMEPAD_MASK_R1, positions[LEDS_BUTTON_R1]),
			Pixel(LEDS_BUTTON_R2, GAMEPAD_MASK_R2, positions[LEDS_BUTTON_R2]),
		},
		{
			Pixel(LEDS_BUTTON_L1, GAMEPAD_MASK_L1, positions[LEDS_BUTTON_L1]),
			Pixel(LEDS_BUTTON_L2, GAMEPAD_MASK_L2, positions[LEDS_BUTTON_L2]),
		},
	};

	return PixelMatrix(pixels);
}

PixelMatrix createHitboxLayout(std::vector<uint8_t> *positions)
{
	std::vector<std::vector<Pixel>> pixels =
	{
		{
			Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL, positions[LEDS_DPAD_LEFT]),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_DOWN, GAMEPAD_MASK_DD, positions[LEDS_DPAD_DOWN]),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			Pixel(LEDS_DPAD_RIGHT, GAMEPAD_MASK_DR, positions[LEDS_DPAD_RIGHT]),
			NO_PIXEL,
			NO_PIXEL,
		},
		{
			NO_PIXEL,
			NO_PIXEL,
			Pixel(LEDS_DPAD_UP,   GAMEPAD_MASK_DU, positions[LEDS_DPAD_UP]),
		},
		{
			Pixel(LEDS_BUTTON_B3, GAMEPAD_MASK_B3, positions[LEDS_BUTTON_B3]),
			Pixel(LEDS_BUTTON_B1, GAMEPAD_MASK_B1, positions[LEDS_BUTTON_B1]),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_B4, GAMEPAD_MASK_B4, positions[LEDS_BUTTON_B4]),
			Pixel(LEDS_BUTTON_B2, GAMEPAD_MASK_B2, positions[LEDS_BUTTON_B2]),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_R1, GAMEPAD_MASK_R1, positions[LEDS_BUTTON_R1]),
			Pixel(LEDS_BUTTON_R2, GAMEPAD_MASK_R2, positions[LEDS_BUTTON_R2]),
			NO_PIXEL,
		},
		{
			Pixel(LEDS_BUTTON_L1, GAMEPAD_MASK_L1, positions[LEDS_BUTTON_L1]),
			Pixel(LEDS_BUTTON_L2, GAMEPAD_MASK_L2, positions[LEDS_BUTTON_L2]),
			NO_PIXEL,
		},
	};

	return PixelMatrix(pixels);
}

PixelMatrix createLedButtonLayout(LedButtonLayout layout, int ledsPerPixel)
{
	std::vector<uint8_t> positions[LED_BUTTON_COUNT];
	for (int i = 0; i != LED_BUTTON_COUNT; i++)
	{
		positions[i].resize(ledsPerPixel);
		for (int l = 0; l != ledsPerPixel; l++)
			positions[i][l] = (i * ledsPerPixel) + l;
	}

	return createLedButtonLayout(layout, positions);
}

PixelMatrix createLedButtonLayout(LedButtonLayout layout, std::vector<uint8_t> *positions)
{
	switch (layout)
	{
		case LED_BUTTONS_STANDARD:
			return createStandardLayout(positions);

		case LED_BUTTONS_HITBOX:
			return createHitboxLayout(positions);
	}
}

#endif
