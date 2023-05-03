#include "addons/reverse.h"
#include "storagemanager.h"
#include "GamepadEnums.h"

bool ReverseInput::available() {
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    pinButtonReverse = options.pinButtonReverse;
	return (options.ReverseInputEnabled &&
        pinButtonReverse != (uint8_t)-1);
}

void ReverseInput::setup()
{
    // Setup Reverse Input Button
    gpio_init(pinButtonReverse);             // Initialize pin
    gpio_set_dir(pinButtonReverse, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinButtonReverse);          // Set as PULLUP

    // Setup Reverse LED if available
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    pinLED = options.pinReverseLED;
    if (pinLED != (uint8_t)-1) {
        gpio_init(options.pinReverseLED);
        gpio_set_dir(options.pinReverseLED, GPIO_OUT);
        gpio_put(options.pinReverseLED, 1);
    }

    actionUp = options.reverseActionUp;
    actionDown = options.reverseActionDown;
    actionLeft = options.reverseActionLeft;
    actionRight = options.reverseActionDown;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
	mapDpadUp    = gamepad->mapDpadUp;
	mapDpadDown  = gamepad->mapDpadDown;
	mapDpadLeft  = gamepad->mapDpadLeft;
	mapDpadRight = gamepad->mapDpadRight;

    invertXAxis = gamepad->options.invertXAxis;
    invertYAxis = gamepad->options.invertYAxis;

    state = false;
}

void ReverseInput::update() {
    state = !gpio_get(pinButtonReverse);
}

uint8_t ReverseInput::input(uint8_t valueMask, uint16_t buttonMask, uint16_t buttonMaskReverse, uint8_t action, bool invertAxis) {
    if (state && action == 2) {
        return 0;
    }
    bool invert = (state && action == 1) ? !invertAxis : invertAxis;
    return (valueMask ? (invert ? buttonMaskReverse : buttonMask) : 0);
}

void ReverseInput::process()
{
    // Update Reverse State
    update();

    uint32_t values = ~gpio_get_all();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.dpad = 0
        | input(values & mapDpadUp->pinMask,    mapDpadUp->buttonMask,      mapDpadDown->buttonMask,    actionUp,       invertYAxis)
        | input(values & mapDpadDown->pinMask,  mapDpadDown->buttonMask,    mapDpadUp->buttonMask,      actionDown,     invertYAxis)
        | input(values & mapDpadLeft->pinMask,  mapDpadLeft->buttonMask,    mapDpadRight->buttonMask,   actionLeft,     invertXAxis)
        | input(values & mapDpadRight->pinMask, mapDpadRight->buttonMask,   mapDpadLeft->buttonMask,    actionRight,    invertXAxis)
    ;

    if (pinLED != (uint8_t)-1) {
        gpio_put(pinLED, !state);
    }
}
