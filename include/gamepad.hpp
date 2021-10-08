#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "BoardConfig.h"
#include "pico/stdlib.h"
#include <MPGS.h>

struct GamepadButtonMapping {
  const uint8_t pin;
  const uint32_t pinMask;
  const uint16_t buttonMask;
};

class Gamepad : public MPGS {
public:
  Gamepad(int debounceMS = 5, GamepadStorage *storage = &GamepadStore)
      : MPGS(debounceMS, storage) {}
  void setup();
  void read();

  inline bool __attribute__((always_inline)) pressedF1() {
#ifdef PIN_SETTINGS
    return state.aux & (1 << 0);
#else
    return MPGS::pressedF1();
#endif
  }
};

#endif