#include "addons/absolute_analog.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

// Scale -255..255 into the gamepad's 16-bit axis range
static uint16_t scaleAxis(int32_t value) {
    if (value > ABSOLUTE_ANALOG_AXIS_MAX) value = ABSOLUTE_ANALOG_AXIS_MAX;
    if (value < -ABSOLUTE_ANALOG_AXIS_MAX) value = -ABSOLUTE_ANALOG_AXIS_MAX;

    int32_t scaled = (int32_t)GAMEPAD_JOYSTICK_MID +
        ((value * ((int32_t)GAMEPAD_JOYSTICK_MID + 1)) / ABSOLUTE_ANALOG_AXIS_MAX);

    if (scaled < (int32_t)GAMEPAD_JOYSTICK_MIN) scaled = (int32_t)GAMEPAD_JOYSTICK_MIN;
    if (scaled > (int32_t)GAMEPAD_JOYSTICK_MAX) scaled = (int32_t)GAMEPAD_JOYSTICK_MAX;

    return (uint16_t)scaled;
}

void applyAbsoluteAnalogValue(GamepadState& state, uint8_t index) {
    const AbsoluteAnalogOptions& options = Storage::getInstance().getAddonOptions().absoluteAnalogOptions;
    if (!options.enabled || index >= options.entries_count) return;

    const AbsoluteAnalogEntry& entry = options.entries[index];
    if (!entry.enabled) return;

    if (entry.stick == ABSOLUTE_ANALOG_STICK_RIGHT) {
        state.rx = scaleAxis(entry.x);
        state.ry = scaleAxis(entry.y);
    } else {
        state.lx = scaleAxis(entry.x);
        state.ly = scaleAxis(entry.y);
    }
}

bool AbsoluteAnalogAddon::available() {
    const AbsoluteAnalogOptions& options = Storage::getInstance().getAddonOptions().absoluteAnalogOptions;
    return options.enabled;
}

void AbsoluteAnalogAddon::setup() {
    for (uint8_t index = 0; index < ABSOLUTE_ANALOG_COUNT; index++) {
        pinMasks[index] = 0;
    }

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch (pinMappings[pin].action) {
            case GpioAction::ABSOLUTE_ANALOG_1: pinMasks[0] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_2: pinMasks[1] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_3: pinMasks[2] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_4: pinMasks[3] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_5: pinMasks[4] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_6: pinMasks[5] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_7: pinMasks[6] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_8: pinMasks[7] |= 1 << pin; break;
            case GpioAction::ABSOLUTE_ANALOG_9: pinMasks[8] |= 1 << pin; break;
            default:    break;
        }
    }
}

void AbsoluteAnalogAddon::reinit() {
    setup();
}

void AbsoluteAnalogAddon::process() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    for (uint8_t index = 0; index < ABSOLUTE_ANALOG_COUNT; index++) {
        if (pinMasks[index] && (gamepad->debouncedGpio & pinMasks[index])) {
            applyAbsoluteAnalogValue(gamepad->state, index);
        }
    }
}
