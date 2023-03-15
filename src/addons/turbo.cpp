#include "addons/turbo.h"

#include "hardware/adc.h"

#include "storagemanager.h"

#define TURBO_DEBOUNCE_MILLIS 5

#define TURBO_SHOT_MIN 5
#define TURBO_SHOT_MAX 30

bool TurboInput::available() {
	AddonOptions options = Storage::getInstance().getAddonOptions();
    pinButtonTurbo = options.pinButtonTurbo;
    return options.TurboInputEnabled;
}

void TurboInput::setup()
{
    // Setup TURBO Key GPIO
    if ( pinButtonTurbo != -1 ) {
        gpio_init(pinButtonTurbo);             // Initialize pin
        gpio_set_dir(pinButtonTurbo, GPIO_IN); // Set as INPUT
        gpio_pull_up(pinButtonTurbo);          // Set as PULLUP
    }
    
    // Setup Turbo LED if available
    AddonOptions options = Storage::getInstance().getAddonOptions();
    if (options.pinTurboLED != -1) {
        gpio_init(options.pinTurboLED);
        gpio_set_dir(options.pinTurboLED, GPIO_OUT);
        gpio_put(options.pinTurboLED, 1);
    }

    // SHMUP Mode
    shmupMode = options.shmupMode;
    if ( shmupMode == 1 ) {
        shmupBtnPin[0] = options.pinShmupBtn1; // Never Turbo aux buttons
        shmupBtnPin[1] = options.pinShmupBtn2;
        shmupBtnPin[2] = options.pinShmupBtn3;
        shmupBtnPin[3] = options.pinShmupBtn4;
        for (uint8_t i = 0; i < 4; i++) {
            if ( shmupBtnPin[i] != -1 ) {
                gpio_init(shmupBtnPin[i]);
                gpio_set_dir(shmupBtnPin[i], GPIO_IN);
                gpio_pull_up(shmupBtnPin[i]);
            }
        }
        shmupBtnMask[0] = options.shmupBtnMask1;
        shmupBtnMask[1] = options.shmupBtnMask2;
        shmupBtnMask[2] = options.shmupBtnMask3;
        shmupBtnMask[3] = options.shmupBtnMask4;
        alwaysEnabled = options.shmupAlwaysOn1 | options.shmupAlwaysOn2 |
                            options.shmupAlwaysOn3 | options.shmupAlwaysOn4;
        if ( options.pinShmupDial != -1 ) {
            adc_gpio_init(options.pinShmupDial);
        }
        buttonsEnabled = alwaysEnabled;
    } else {
        // SHMUP mode off
        buttonsEnabled = 0;
    }

    bDebState = false;
    uDebTime = getMillis();
    lastPressed = 0;
    lastDpad = 0;
    uIntervalMS = (uint32_t)(1000.0 / options.turboShotCount);
    bTurboState = false;
    bTurboFlicker = false;
    nextTimer = getMillis();
}

bool TurboInput::read()
{
    // Get the SHMUP no turbo buttons
    if ( shmupMode ) {
        for (uint8_t i = 0; i < 4; i++) {
            if ( shmupBtnPin[i] != -1 ) {
                !gpio_get(shmupBtnPin[i]);
            }
        }

        // Use the dial to modify our turbo shot speed
        if ( options.pinShmupDial != -1 ) {
            adc_select_input(26-options.pinShmupDial);
            uint16_t dial = adc_read();
        }
    }

    // Get TURBO Key State
    return(!gpio_get(pinButtonTurbo));
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
    AddonOptions options = Storage::getInstance().getAddonOptions();
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
            if ( options.shmupMode == 1 ) {
                buttonsEnabled |= alwaysEnabled;  // SHMUP Always-on Buttons Set
            }
            // Turn off button once turbo is toggled
            gamepad->state.buttons &= ~(TURBO_BUTTON_MASK);
        }
        if (dpadPressed & GAMEPAD_MASK_DOWN && (lastDpad != dpadPressed)) {
            if ( options.turboShotCount > TURBO_SHOT_MIN ) { // can't go lower than 2-shots per second
                options.turboShotCount--;
                Storage::getInstance().setAddonOptions(options);
                uIntervalMS = (uint32_t)(1000.0 / options.turboShotCount);
            }
        } else if ( dpadPressed & GAMEPAD_MASK_UP && (lastDpad != dpadPressed)) {
            if ( options.turboShotCount < TURBO_SHOT_MAX ) { // can't go higher than 60-shots per second
                options.turboShotCount++;
                Storage::getInstance().setAddonOptions(options);
                uIntervalMS = (uint32_t)(1000.0 / options.turboShotCount);
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
    if ( options.pinTurboLED != -1 ) {
        if ((gamepad->state.buttons & buttonsEnabled) && !bTurboFlicker) {
            gpio_put(options.pinTurboLED, 0);
        } else {
            gpio_put(options.pinTurboLED, 1);
        }	
    }

    // disable button during turbo flicker
    if (bTurboFlicker) { 
        gamepad->state.buttons &= ~(buttonsEnabled);
    }

    // set non-shmup buttons after we do the flicker
    if ( shmupMode == 1 ) {

    }

    if (getMillis() < nextTimer) {
        return; // don't flip if we haven't reached the next timer
    }

    bTurboFlicker ^= true; // Button ON/OFF State Reverse
    nextTimer = getMillis() + uIntervalMS; // interval to flicker-off button
}
