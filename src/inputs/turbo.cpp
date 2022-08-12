#include "inputs/turbo.h"

#include "storagemanager.h"

#define TURBO_DEBOUNCE_MILLIS 5

// Turbo will never be less than 2 shots, greater than 60 shots per second
#define TURBO_SHOT_MIN 2
#define TURBO_SHOT_MAX 60

bool TurboInput::available() {
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    return (boardOptions.pinButtonTurbo != -1);
}

void TurboInput::setup()
{
    // Setup TURBO Key
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    gpio_init(boardOptions.pinButtonTurbo);             // Initialize pin
    gpio_set_dir(boardOptions.pinButtonTurbo, GPIO_IN); // Set as INPUT
    gpio_pull_up(boardOptions.pinButtonTurbo);          // Set as PULLUP
    
    if (TURBO_LED_PIN != -1) {
        gpio_init(TURBO_LED_PIN);
        gpio_set_dir(TURBO_LED_PIN, GPIO_OUT);
        gpio_put(TURBO_LED_PIN, 0);
    }

    bDebState = false;
    uDebTime = getMillis();
    lastPressed = 0;
    lastDpad = 0;
    buttonsEnabled = 0;
    shotCount = boardOptions.turboShotCount;
    uIntervalMS = (uint32_t)(1000.0 / shotCount) / 2;
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
        if (dpadPressed && (lastDpad != dpadPressed)) {
            if ( dpadPressed == GAMEPAD_MASK_DOWN ) {
                if ( shotCount > TURBO_SHOT_MIN ) { // can't go lower than 2-shots per second
                    shotCount--;
                    uIntervalMS = (uint32_t)(1000.0 / shotCount) / 2;
                }
                gamepad->state.buttons &= ~(GAMEPAD_MASK_DPAD);
            } else if ( dpadPressed == GAMEPAD_MASK_UP ) {
                if ( shotCount < TURBO_SHOT_MAX ) { // can't go higher than 60-shots per second
                    shotCount++;
                    uIntervalMS = (uint32_t)(1000.0 / shotCount) / 2;
                }
                gamepad->state.buttons &= ~(GAMEPAD_MASK_DPAD);
            }
        }
        lastPressed = buttonsPressed; // save last pressed
        lastDpad = dpadPressed;
        return; // Holding TURBO cancels turbo functionality
    } else {
        lastPressed = 0; // disable last pressed
        lastDpad = 0; // disable last dpad
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

    // Set TURBO LED if a button is going
    if ( TURBO_LED_PIN != -1 ) {
        if ((gamepad->state.buttons & buttonsEnabled) && !bTurboFlicker) {
            gpio_put(TURBO_LED_PIN, 1);
        } else {
            gpio_put(TURBO_LED_PIN, 0);
        }
    }
}