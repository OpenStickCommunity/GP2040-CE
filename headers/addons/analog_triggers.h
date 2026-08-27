#ifndef ANALOG_TRIGGERS_H_
#define ANALOG_TRIGGERS_H_

#include "gpaddon.h"
#include "gamepad/GamepadState.h"

#ifndef ANALOG_TRIGGERS_ENABLED
#define ANALOG_TRIGGERS_ENABLED 0
#endif

#ifndef ANALOG_TRIGGERS_LEFT_PIN
#define ANALOG_TRIGGERS_LEFT_PIN -1
#endif

#ifndef ANALOG_TRIGGERS_RIGHT_PIN
#define ANALOG_TRIGGERS_RIGHT_PIN -1
#endif

#ifndef ANALOG_TRIGGERS_INNER_DEADZONE
#define ANALOG_TRIGGERS_INNER_DEADZONE 0.0f
#endif

#ifndef ANALOG_TRIGGERS_OUTER_DEADZONE
#define ANALOG_TRIGGERS_OUTER_DEADZONE 1.0f
#endif

// Analog Triggers Module Name
#define AnalogTriggersName "AnalogTriggers"

// Add-on standalone y aislado: lee 1 o 2 potenciometros conectados a pines
// ADC del RP2040 y los reporta como LT/RT analogicos reales (gamepad->state.lt/rt),
// sin tocar el add-on "Analog" (pensado para sticks, con calibracion de centro
// que no encaja con un pedal que descansa en un extremo).
class AnalogTriggerInput : public GPAddon
{
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return AnalogTriggersName; }
private:
    uint8_t readTriggerByte(uint8_t pinAdc, bool invert, float innerDeadzone, float outerDeadzone);

    int8_t leftTriggerPin;
    int8_t rightTriggerPin;
    uint8_t leftTriggerPinAdc;
    uint8_t rightTriggerPinAdc;
    bool leftTriggerInvert;
    bool rightTriggerInvert;
    float innerDeadzone;
    float outerDeadzone;
};

#endif
