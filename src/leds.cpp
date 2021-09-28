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


NeoPico neopico(BOARD_LEDS_PIN, Pixel::getPixelCount(pixels));
AnimationStation as(pixels);
queue_t animationQueue;

void LEDs::setup() {
  queue_init(&animationQueue, sizeof(AnimationHotkey), 1);

  AnimationStation::ConfigureBrightness(LED_BRIGHTNESS_MAXIMUM,
                                        LED_BRIGHTNESS_STEPS);
  AnimationStore.setup(&as);
}

void LEDs::process(MPGS *gamepad) {
  AnimationHotkey action = animationHotkeys(gamepad);
  if (action != HOTKEY_LEDS_NONE)
    queue_add_blocking(&animationQueue, &action);
}

void LEDs::loop() {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  if (queue_try_peek(&animationQueue, &action)) {
    queue_remove_blocking(&animationQueue, &action);
    as.HandleEvent(this->action);
    AnimationStore.save();
  }

  as.Animate();
  as.ApplyBrightness(frame);
  neopico.SetFrame(frame);
  neopico.Show();

  this->nextRunTime = make_timeout_time_ms(LEDs::intervalMS);
}

void configureAnimations(AnimationStation *as) {
  as->AddAnimation(new StaticColor(pixels, ColorBlack));
  as->AddAnimation(new StaticColor(pixels, LEDS_STATIC_COLOR_COLOR));
  as->AddAnimation(new Rainbow(pixels, LEDS_RAINBOW_CYCLE_TIME));
  as->AddAnimation(new Chase(pixels, LEDS_CHASE_CYCLE_TIME));

  for (size_t i = 0; i < customThemes.size(); i++)
    as->AddAnimation(&customThemes[i]);
}

AnimationHotkey animationHotkeys(MPGS *gamepad) {
  AnimationHotkey action = HOTKEY_LEDS_NONE;

  if (gamepad->pressedF1()) {
    if (gamepad->pressedB3()) {
      action = HOTKEY_LEDS_ANIMATION_UP;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_B3 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    } else if (gamepad->pressedB1()) {
      action = HOTKEY_LEDS_ANIMATION_DOWN;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_B1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    } else if (gamepad->pressedB4()) {
      action = HOTKEY_LEDS_BRIGHTNESS_UP;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_B4 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    } else if (gamepad->pressedB2()) {
      action = HOTKEY_LEDS_BRIGHTNESS_DOWN;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_B2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    } else if (gamepad->pressedR1()) {
      action = HOTKEY_LEDS_PARAMETER_UP;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_R1 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    } else if (gamepad->pressedR2()) {
      action = HOTKEY_LEDS_PARAMETER_DOWN;
      gamepad->state.buttons &=
          ~(GAMEPAD_MASK_R2 | GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    }
  }

  return action;
}

#endif
