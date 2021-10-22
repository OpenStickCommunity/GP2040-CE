/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "BoardConfig.h"

#ifdef BOARD_LEDS_PIN

#include "pico/util/queue.h"

#include "AnimationStation.hpp"
#include "AnimationStorage.hpp"
#include "NeoPico.hpp"
#include "Pixel.hpp"
#include "leds.h"
#include "themes.h"

#ifdef LEDS_PER_PIXEL
PixelMatrix matrix = createLedButtonLayout(LED_LAYOUT, LEDS_PER_PIXEL);
#else
PixelMatrix matrix = createLedButtonLayout(LED_LAYOUT, ledPositions);
#endif

#ifdef LED_FORMAT
NeoPico neopico(BOARD_LEDS_PIN, matrix.getLedCount(), LED_FORMAT);
#else
NeoPico neopico(BOARD_LEDS_PIN, matrix.getLedCount());
#endif

AnimationStation as(matrix);
queue_t baseAnimationQueue;
queue_t buttonAnimationQueue;
queue_t animationSaveQueue;

void LEDs::trySave() {
  static int saveValue = 0;

  if (queue_try_remove(&animationSaveQueue, &saveValue))
    AnimationStore.save();
}

void LEDs::setup() {
  queue_init(&baseAnimationQueue, sizeof(AnimationHotkey), 1);
  queue_init(&buttonAnimationQueue, sizeof(uint32_t), 1);
  queue_init(&animationSaveQueue, sizeof(int), 1);

  AnimationStore.setup(&as);
  StaticTheme::AddTheme(themeStaticRainbow);
  StaticTheme::AddTheme(themeGuiltyGearTypeA);
  StaticTheme::AddTheme(themeGuiltyGearTypeD);
  StaticTheme::AddTheme(themeGuiltyGearCustom);
  StaticTheme::AddTheme(themeNeoGeo);
  StaticTheme::AddTheme(themeNeoGeoCurved);
  StaticTheme::AddTheme(themeNeoGeoModern);
  StaticTheme::AddTheme(themeSixButtonFighter);
  StaticTheme::AddTheme(themeSixButtonFighterPlus);
  StaticTheme::AddTheme(themeSuperFamicom);
  StaticTheme::AddTheme(themeXbox);
}

void LEDs::process(Gamepad *gamepad) {
  // We use queue_try_add here because if core1 explodes everywhere, we don't
  // care. It's not as important as handling inputs.

  AnimationHotkey action = animationHotkeys(gamepad);
  if (action != HOTKEY_LEDS_NONE) {
    queue_try_add(&baseAnimationQueue, &action);
  }

  uint32_t buttonState = gamepad->state.dpad << 16 | gamepad->state.buttons;
  queue_try_add(&buttonAnimationQueue, &buttonState);
}

void LEDs::loop() {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  AnimationHotkey action;
  uint32_t buttonState;

  if (queue_try_peek(&baseAnimationQueue, &action)) {
    queue_try_remove(&baseAnimationQueue, &action);
    as.HandleEvent(action);
    queue_try_add(&animationSaveQueue, 0);
  }

  if (queue_try_peek(&buttonAnimationQueue, &buttonState)) {
    queue_try_remove(&buttonAnimationQueue, &buttonState);

    std::vector<Pixel> pressed;

    for (size_t r = 0; r != matrix.pixels.size(); r++) {
      for (size_t c = 0; c != matrix.pixels[r].size(); c++) {
        if (buttonState & matrix.pixels[r][c].mask)
          pressed.push_back(matrix.pixels[r][c]);
      }
    }

    if (pressed.size() > 0) {
      as.HandlePressed(pressed);
    } else {
      as.ClearPressed();
    }
  }

  as.Animate();
  as.ApplyBrightness(frame);
  neopico.SetFrame(frame);
  neopico.Show();

  this->nextRunTime = make_timeout_time_ms(LEDs::intervalMS);
}

/**
 * @brief Create an LED layout using a 2x4 matrix.
 */
PixelMatrix createLedLayoutArcadeButtons(std::vector<uint8_t> *positions) {
  std::vector<std::vector<Pixel>> pixels = {
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

/**
 * @brief Create an LED layout using a 3x8 matrix.
 */
PixelMatrix createLedLayoutArcadeHitbox(std::vector<uint8_t> *positions) {
  std::vector<std::vector<Pixel>> pixels = {
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
          Pixel(LEDS_DPAD_UP, GAMEPAD_MASK_DU, positions[LEDS_DPAD_UP]),
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

/**
 * @brief Create an LED layout using a 2x7 matrix.
 */
PixelMatrix createLedLayoutArcadeWasd(std::vector<uint8_t> *positions) {
  std::vector<std::vector<Pixel>> pixels = {
      {
          NO_PIXEL,
          Pixel(LEDS_DPAD_LEFT, GAMEPAD_MASK_DL, positions[LEDS_DPAD_LEFT]),
      },
      {
          Pixel(LEDS_DPAD_UP, GAMEPAD_MASK_DU, positions[LEDS_DPAD_UP]),
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

PixelMatrix createLedButtonLayout(LedLayout layout, int ledsPerPixel) {
  std::vector<uint8_t> positions[LED_BUTTON_COUNT];
  for (int i = 0; i != LED_BUTTON_COUNT; i++) {
    positions[i].resize(ledsPerPixel);
    for (int l = 0; l != ledsPerPixel; l++)
      positions[i][l] = (i * ledsPerPixel) + l;
  }

  return createLedButtonLayout(layout, positions);
}

PixelMatrix createLedButtonLayout(LedLayout layout,
                                  std::vector<uint8_t> *positions) {
  switch (layout) {
  case LED_LAYOUT_ARCADE_BUTTONS:
    return createLedLayoutArcadeButtons(positions);

  case LED_LAYOUT_ARCADE_HITBOX:
    return createLedLayoutArcadeHitbox(positions);

  case LED_LAYOUT_ARCADE_WASD:
    return createLedLayoutArcadeWasd(positions);
  }
}

AnimationHotkey animationHotkeys(Gamepad *gamepad) {
  AnimationHotkey action = HOTKEY_LEDS_NONE;

  if (gamepad->pressedF1()) {
    if (gamepad->pressedB3()) {
      action = HOTKEY_LEDS_ANIMATION_UP;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_B3 | gamepad->f1Mask);
    } else if (gamepad->pressedB1()) {
      action = HOTKEY_LEDS_ANIMATION_DOWN;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_B1 | gamepad->f1Mask);
    } else if (gamepad->pressedB4()) {
      action = HOTKEY_LEDS_BRIGHTNESS_UP;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_B4 | gamepad->f1Mask);
    } else if (gamepad->pressedB2()) {
      action = HOTKEY_LEDS_BRIGHTNESS_DOWN;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_B2 | gamepad->f1Mask);
    } else if (gamepad->pressedR1()) {
      action = HOTKEY_LEDS_PARAMETER_UP;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_R1 | gamepad->f1Mask);
    } else if (gamepad->pressedR2()) {
      action = HOTKEY_LEDS_PARAMETER_DOWN;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_R2 | gamepad->f1Mask);
    } else if (gamepad->pressedL1()) {
      action = HOTKEY_LEDS_PRESS_PARAMETER_UP;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_L1 | gamepad->f1Mask);
    } else if (gamepad->pressedL2()) {
      action = HOTKEY_LEDS_PRESS_PARAMETER_DOWN;
      gamepad->state.buttons &= ~(GAMEPAD_MASK_L2 | gamepad->f1Mask);
    }
  }

  return action;
}

#endif
