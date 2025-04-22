#ifndef _GPGAMEPADEVENT_H_
#define _GPGAMEPADEVENT_H_

#include "gamepad.h"

class GPGamepadEvent : public GPEvent {
    public:
        GPGamepadEvent() {}
        GPGamepadEvent(GamepadState currState) { this->state = currState; }
        GPGamepadEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) {
            this->state.dpad = dpad;
            this->state.buttons = buttons;
            this->state.aux = aux;
        }
        GPGamepadEvent(uint16_t lx, uint16_t ly, uint16_t rx, uint16_t ry, uint8_t lt, uint8_t rt) {
            this->state.lx = lx;
            this->state.ly = ly;
            this->state.rx = rx;
            this->state.ry = ry;
            this->state.lt = lt;
            this->state.rt = rt;
        }
        GPGamepadEvent(uint8_t dpad, uint16_t buttons, uint16_t aux, uint16_t lx, uint16_t ly, uint16_t rx, uint16_t ry, uint8_t lt, uint8_t rt) {
            this->state.dpad = dpad;
            this->state.buttons = buttons;
            this->state.aux = aux;
            this->state.lx = lx;
            this->state.ly = ly;
            this->state.rx = rx;
            this->state.ry = ry;
            this->state.lt = lt;
            this->state.rt = rt;
        }
        virtual ~GPGamepadEvent() {}
        
        GamepadState state;
    private:
};

class GPButtonEvent : public GPGamepadEvent {
    public:
        GPButtonEvent() {}
        GPButtonEvent(GamepadState currState) : GPGamepadEvent(currState) {}
        GPButtonEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) : GPGamepadEvent(dpad, buttons, aux) {}
        ~GPButtonEvent() {}
    private:
};

class GPButtonUpEvent : public GPButtonEvent {
    public:
        GPButtonUpEvent() {}
        GPButtonUpEvent(GamepadState currState) : GPButtonEvent(currState) {}
        GPButtonUpEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) : GPButtonEvent(dpad, buttons, aux) {}
        ~GPButtonUpEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_BUTTON_UP;
};

class GPButtonDownEvent : public GPButtonEvent {
    public:
        GPButtonDownEvent() {}
        GPButtonDownEvent(GamepadState currState) : GPButtonEvent(currState) {}
        GPButtonDownEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) : GPButtonEvent(dpad, buttons, aux) {}
        ~GPButtonDownEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_BUTTON_DOWN;
};

class GPButtonProcessedUpEvent : public GPButtonEvent {
    public:
        GPButtonProcessedUpEvent() {}
        GPButtonProcessedUpEvent(GamepadState currState) : GPButtonEvent(currState) {}
        GPButtonProcessedUpEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) : GPButtonEvent(dpad, buttons, aux) {}
        ~GPButtonProcessedUpEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_BUTTON_PROCESSED_UP;
};

class GPButtonProcessedDownEvent : public GPButtonEvent {
    public:
        GPButtonProcessedDownEvent() {}
        GPButtonProcessedDownEvent(GamepadState currState) : GPButtonEvent(currState) {}
        GPButtonProcessedDownEvent(uint8_t dpad, uint16_t buttons, uint16_t aux) : GPButtonEvent(dpad, buttons, aux) {}
        ~GPButtonProcessedDownEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_BUTTON_PROCESSED_DOWN;
};

class GPAnalogEvent : public GPGamepadEvent {
    public:
        GPAnalogEvent() {}
        GPAnalogEvent(GamepadState currState) : GPGamepadEvent(currState) {}
        GPAnalogEvent(uint16_t lx, uint16_t ly, uint16_t rx, uint16_t ry, uint8_t lt, uint8_t rt) : GPGamepadEvent(lx, ly, rx, ry, lt, rt) {}
        ~GPAnalogEvent() {}
    private:
};

class GPAnalogMoveEvent : public GPAnalogEvent {
    public:
        GPAnalogMoveEvent() {}
        GPAnalogMoveEvent(GamepadState currState) : GPAnalogEvent(currState) {}
        GPAnalogMoveEvent(uint16_t lx, uint16_t ly, uint16_t rx, uint16_t ry, uint8_t lt, uint8_t rt) : GPAnalogEvent(lx, ly, rx, ry, lt, rt) {}
        ~GPAnalogMoveEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_ANALOG_MOVE;
};

class GPAnalogProcessedMoveEvent : public GPAnalogEvent {
    public:
        GPAnalogProcessedMoveEvent() {}
        GPAnalogProcessedMoveEvent(GamepadState currState) : GPAnalogEvent(currState) {}
        GPAnalogProcessedMoveEvent(uint16_t lx, uint16_t ly, uint16_t rx, uint16_t ry, uint8_t lt, uint8_t rt) : GPAnalogEvent(lx, ly, rx, ry, lt, rt) {}
        ~GPAnalogProcessedMoveEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_ANALOG_PROCESSED_MOVE;
};

#endif