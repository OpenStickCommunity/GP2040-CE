#include "addons/turbo.h"

#include "storagemanager.h"

#define TURBO_DEBOUNCE_MILLIS 5

#define TURBO_SHOT_MIN 5
#define TURBO_SHOT_MAX 30

bool TurboInput::available() {
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    return (boardOptions.pinButtonTurbo != (uint8_t)-1);
}

void TurboInput::setup()
{
    // Setup TURBO Key
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    gpio_init(boardOptions.pinButtonTurbo);             // Initialize pin
    gpio_set_dir(boardOptions.pinButtonTurbo, GPIO_IN); // Set as INPUT
    gpio_pull_up(boardOptions.pinButtonTurbo);          // Set as PULLUP
    
    if (boardOptions.pinTurboLED != -1) {
        gpio_init(boardOptions.pinTurboLED);
        gpio_set_dir(boardOptions.pinTurboLED, GPIO_OUT);
        gpio_put(boardOptions.pinTurboLED, 1);
    }

    bDebState = false;
    uDebTime = getMillis();
    lastPressed = 0;
    lastDpad = 0;
    buttonsEnabled = 0;
    uIntervalMS = (uint32_t)(1000.0 / boardOptions.turboShotCount);
    bTurboState = false;
    bTurboFlicker = false;
    nextTimer = getMillis();
}

bool TurboInput::read()
{
    // Get TURBO Key State
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    return(!gpio_get(boardOptions.pinButtonTurbo));
}

void TurboInput::debounce()
{
    uint32_t uNowTime = getMillis();
    if ((bDebState != bTurboState) && ((uNowTime - uDebTime) > TURBO_DEBOUNCE_MILLIS)) {
        bDebState ^= true;
        uDebTime = uNowTime;
    }
    bTurboState = bDebState;
}

void TurboInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    uint16_t buttonsPressed = gamepad->state.buttons & TURBO_BUTTON_MASK;
    uint16_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    // Get TURBO Button State
    bTurboState = read();
#if TURBO_DEBOUNCE_MILLIS > 0
    debounce();
#endif

    // Set TURBO Enable Buttons 
    if (bTurboState) {
        if (buttonsPressed && (lastPressed != buttonsPressed)) {
            buttonsEnabled ^= buttonsPressed; // Toggle Turbo
            // Turn off button once turbo is toggled
            gamepad->state.buttons &= ~(TURBO_BUTTON_MASK);
        }
        if (dpadPressed & GAMEPAD_MASK_DOWN && (lastDpad != dpadPressed)) {
            if ( boardOptions.turboShotCount > TURBO_SHOT_MIN ) { // can't go lower than 2-shots per second
                boardOptions.turboShotCount--;
                Storage::getInstance().setBoardOptions(boardOptions);
                uIntervalMS = (uint32_t)(1000.0 / boardOptions.turboShotCount);
            }
        } else if ( dpadPressed & GAMEPAD_MASK_UP && (lastDpad != dpadPressed)) {
            if ( boardOptions.turboShotCount < TURBO_SHOT_MAX ) { // can't go higher than 60-shots per second
                boardOptions.turboShotCount++;
                Storage::getInstance().setBoardOptions(boardOptions);
                uIntervalMS = (uint32_t)(1000.0 / boardOptions.turboShotCount);
            }
        }
        lastPressed = buttonsPressed; // save last pressed
        lastDpad = dpadPressed;
        return; // Holding TURBO cancels turbo functionality
    } else {
        lastPressed = 0; // disable last pressed
        lastDpad = 0; // disable last dpad
    }


    // Set TURBO LED if a button is going or turbo is too fast
    if ( boardOptions.pinTurboLED != -1 ) {
        if ((gamepad->state.buttons & buttonsEnabled) && !bTurboFlicker) {
            gpio_put(boardOptions.pinTurboLED, 0);
        } else {
            gpio_put(boardOptions.pinTurboLED, 1);
        }	
    }

    // disable button during turbo flicker
    if (bTurboFlicker) { 
        gamepad->state.buttons &= ~(buttonsEnabled);
    }

    if (getMillis() < nextTimer) {
        return; // don't flip if we haven't reached the next timer
    }

    bTurboFlicker ^= true; // Button ON/OFF State Reverse
    nextTimer = getMillis() + uIntervalMS; // interval to flicker-off button
}