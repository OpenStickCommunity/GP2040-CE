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

#ifdef LED_FORMAT
NeoPico neopico(BOARD_LEDS_PIN, Pixel::getPixelCount(pixels), LED_FORMAT);
#else
NeoPico neopico(BOARD_LEDS_PIN, Pixel::getPixelCount(pixels));
#endif

AnimationStation as(pixels);
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
  queue_init(&buttonAnimationQueue, sizeof(GamepadState), 1);
	queue_init(&animationSaveQueue, sizeof(int), 1);

  AnimationStore.setup(&as);
}

void configureAnimations() {
  // StaticColor::SetDefaultColor(LEDS_STATIC_COLOR_COLOR);
  Rainbow::SetDefaultCycleTime(LEDS_RAINBOW_CYCLE_TIME);
  Chase::SetDefaultCycleTime(LEDS_CHASE_CYCLE_TIME);

  // for (size_t i = 0; i < customThemes.size(); i++)
  // as->AddAnimation(&customThemes[i]);
}

void LEDs::process(Gamepad *gamepad) {
  // We use queue_try_add here because if core1 explodes everywhere, we don't
  // care. It's not as important as handling inputs.

  AnimationHotkey action = animationHotkeys(gamepad);
  if (action != HOTKEY_LEDS_NONE) {
    queue_try_add(&baseAnimationQueue, &action);
  }

  queue_try_add(&buttonAnimationQueue, &gamepad->state);
}

void LEDs::loop() {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  AnimationHotkey action;
  GamepadState buttonState;

  if (queue_try_peek(&baseAnimationQueue, &action)) {
    queue_try_remove(&baseAnimationQueue, &action);
    as.HandleEvent(action);
		queue_try_add(&animationSaveQueue, 0);
  }

  if (queue_try_peek(&buttonAnimationQueue, &buttonState)) {
    queue_try_remove(&buttonAnimationQueue, &buttonState);

    std::vector<Pixel> pressed;

    for (size_t i = 0; i < pixels.size(); i++) {
      if ((pixels[i].index > 3 && buttonState.buttons & pixels[i].mask) ||
          (pixels[i].index <= 3 && buttonState.dpad & pixels[i].mask)) {
        pressed.push_back(pixels[i]);
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
