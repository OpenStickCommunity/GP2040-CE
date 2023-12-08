#include "addons/reverse.h"
#include "storagemanager.h"
#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"

bool ReverseInput::available() {
    const ReverseOptions& options = Storage::getInstance().getAddonOptions().reverseOptions;
    pinButtonReverse = options.buttonPin;
	return options.enabled && isValidPin(options.buttonPin);
}

void ReverseInput::setup()
{
    // Setup Reverse Input Button
    gpio_init(pinButtonReverse);             // Initialize pin
    gpio_set_dir(pinButtonReverse, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinButtonReverse);          // Set as PULLUP

    // Setup Reverse LED if available
    const ReverseOptions& options = Storage::getInstance().getAddonOptions().reverseOptions;
    pinLED = 0xff;
    if (isValidPin(options.ledPin)) {
        pinLED = options.ledPin;
        gpio_init(pinLED);
        gpio_set_dir(pinLED, GPIO_OUT);
        gpio_put(pinLED, 1);
    }

    actionUp = options.actionUp;
    actionDown = options.actionDown;
    actionLeft = options.actionLeft;
    actionRight = options.actionRight;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
	mapDpadUp    = gamepad->mapDpadUp;
	mapDpadDown  = gamepad->mapDpadDown;
	mapDpadLeft  = gamepad->mapDpadLeft;
	mapDpadRight = gamepad->mapDpadRight;

    invertXAxis = gamepad->getOptions().invertXAxis;
    invertYAxis = gamepad->getOptions().invertYAxis;

    state = false;
}

void ReverseInput::update() {
    state = !gpio_get(pinButtonReverse);
}

uint8_t ReverseInput::input(uint32_t valueMask, uint16_t buttonMask, uint16_t buttonMaskReverse, uint8_t action, bool invertAxis) {
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

    if (pinLED != 0xff) {
        gpio_put(pinLED, !state);
    }
}
