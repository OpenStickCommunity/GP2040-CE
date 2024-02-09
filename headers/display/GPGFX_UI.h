#ifndef _GPGFX_UI_H_
#define _GPGFX_UI_H_

#include <string>
#include <math.h>

#include "GPGFX_core.h"
#include "GPGFX.h"
#include "config.pb.h"
#include "GamepadState.h"

extern uint32_t getMillis();
extern uint64_t getMicro();

class GPGFX_UI {
    public:
        GPGFX_UI();
        GPGFX_UI(GPGFX* renderer) { setRenderer(renderer); }

        void setRenderer(GPGFX* renderer) { _renderer = renderer; }
        GPGFX* getRenderer() { return _renderer; }

        GamepadState getGamepadState() { return _gamepadState; }
        virtual void setGamepadState(GamepadState state) { _gamepadState = state; }

        DisplayOptions getDisplayOptions() { return _displayOptions; }
        void setDisplayOptions(DisplayOptions options) { _displayOptions = options; }

        bool getConfigMode() { return _configMode; }
        void setConfigMode(bool isConfigMode) { _configMode = isConfigMode; }

	    inline bool __attribute__((always_inline)) pressedButton(const uint16_t mask) { return (_gamepadState.buttons & mask) == mask; }
        inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask) { return (_gamepadState.dpad & mask) == mask; }
        inline bool __attribute__((always_inline)) pressedUp()    { return pressedDpad(GAMEPAD_MASK_UP); }
        inline bool __attribute__((always_inline)) pressedDown()  { return pressedDpad(GAMEPAD_MASK_DOWN); }
        inline bool __attribute__((always_inline)) pressedLeft()  { return pressedDpad(GAMEPAD_MASK_LEFT); }
        inline bool __attribute__((always_inline)) pressedRight() { return pressedDpad(GAMEPAD_MASK_RIGHT); }
        inline bool __attribute__((always_inline)) pressedB1()    { return pressedButton(GAMEPAD_MASK_B1); }
        inline bool __attribute__((always_inline)) pressedB2()    { return pressedButton(GAMEPAD_MASK_B2); }
        inline bool __attribute__((always_inline)) pressedB3()    { return pressedButton(GAMEPAD_MASK_B3); }
        inline bool __attribute__((always_inline)) pressedB4()    { return pressedButton(GAMEPAD_MASK_B4); }
        inline bool __attribute__((always_inline)) pressedL1()    { return pressedButton(GAMEPAD_MASK_L1); }
        inline bool __attribute__((always_inline)) pressedR1()    { return pressedButton(GAMEPAD_MASK_R1); }
        inline bool __attribute__((always_inline)) pressedL2()    { return pressedButton(GAMEPAD_MASK_L2); }
        inline bool __attribute__((always_inline)) pressedR2()    { return pressedButton(GAMEPAD_MASK_R2); }
        inline bool __attribute__((always_inline)) pressedS1()    { return pressedButton(GAMEPAD_MASK_S1); }
        inline bool __attribute__((always_inline)) pressedS2()    { return pressedButton(GAMEPAD_MASK_S2); }
        inline bool __attribute__((always_inline)) pressedL3()    { return pressedButton(GAMEPAD_MASK_L3); }
        inline bool __attribute__((always_inline)) pressedR3()    { return pressedButton(GAMEPAD_MASK_R3); }
        inline bool __attribute__((always_inline)) pressedA1()    { return pressedButton(GAMEPAD_MASK_A1); }
        inline bool __attribute__((always_inline)) pressedA2()    { return pressedButton(GAMEPAD_MASK_A2); }

        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    protected:
        GamepadState _gamepadState;
    private:
        GPGFX* _renderer;
        DisplayOptions _displayOptions;
        bool _configMode;
};

#endif
