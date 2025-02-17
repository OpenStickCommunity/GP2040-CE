#ifndef REACTIVELEDS_H_
#define REACTIVELEDS_H_

#include "gpaddon.h"

#ifndef REACTIVE_LED_ENABLED
#define REACTIVE_LED_ENABLED 0
#endif

#ifndef REACTIVE_LED_COUNT
#define REACTIVE_LED_COUNT 10
#endif

#ifndef REACTIVE_LED_DELAY
#define REACTIVE_LED_DELAY 1
#endif

#ifndef REACTIVE_LED_MAX_BRIGHTNESS
#define REACTIVE_LED_MAX_BRIGHTNESS 255
#endif

#ifndef REACTIVE_LED_FADE_INC
#define REACTIVE_LED_FADE_INC 1
#endif

// Reactive LED Module
#define ReactiveLEDName "ReactiveLED"

// Reactive LED
class ReactiveLEDAddon : public GPAddon
{
    public:
        virtual bool available();
        virtual void setup();
        virtual void preprocess() {}
        virtual void process();
        virtual void postprocess(bool sent) {}
        virtual void reinit() {}
        virtual std::string name() { return ReactiveLEDName; }
    private:
        struct ReactiveLEDPinState {
            uint16_t pinNumber = -1;
            ReactiveLEDMode modeDown = ReactiveLEDMode::REACTIVE_LED_STATIC_ON;
            ReactiveLEDMode modeUp = ReactiveLEDMode::REACTIVE_LED_STATIC_OFF;
            GpioAction action = GpioAction::NONE;
            uint8_t value = 0;
            bool currState = false;
            bool prevState = false;
            uint32_t lastUpdate;
            uint32_t currUpdate;
        };

        ReactiveLEDPinState ledPins[REACTIVE_LED_COUNT];

        void setLEDByMode(ReactiveLEDPinState &ledState, bool pressed);
};

#endif