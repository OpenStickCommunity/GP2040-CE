#include "addons/reverse.h"
#include "storagemanager.h"
#include "GamepadEnums.h"

bool ReverseInput::available() {
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	return (boardOptions.pinButtonReverse != (uint8_t)-1);
}

void ReverseInput::setup()
{
    // Setup Reverse Input Button
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    gpio_init(boardOptions.pinButtonReverse);             // Initialize pin
    gpio_set_dir(boardOptions.pinButtonReverse, GPIO_IN); // Set as INPUT
    gpio_pull_up(boardOptions.pinButtonReverse);          // Set as PULLUP
    
    pinLED = boardOptions.pinReverseLED;
    if (pinLED != -1) {
        gpio_init(boardOptions.pinReverseLED);
        gpio_set_dir(boardOptions.pinReverseLED, GPIO_OUT);
        gpio_put(boardOptions.pinReverseLED, 1);
    }

    actionUp = boardOptions.reverseActionUp;
    actionDown = boardOptions.reverseActionDown;
    actionLeft = boardOptions.reverseActionLeft;
    actionRight = boardOptions.reverseActionDown;

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
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    state = !gpio_get(boardOptions.pinButtonReverse);
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
    
    if (pinLED != -1) {
        gpio_put(pinLED, !state);
    }
}
