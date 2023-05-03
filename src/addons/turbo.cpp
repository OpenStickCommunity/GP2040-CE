#include "addons/turbo.h"

#include "hardware/adc.h"

#include "storagemanager.h"

#include <algorithm>

#define TURBO_SHOT_MIN 2
#define TURBO_SHOT_MAX 30

bool TurboInput::available() {
	const AddonOptions& options = Storage::getInstance().getAddonOptions();
    return options.TurboInputEnabled;
}

void TurboInput::setup()
{
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint32_t now = getMillis();

    // Setup TURBO Key GPIO
    if (  options.pinButtonTurbo != (uint8_t)-1 ) {
        gpio_init( options.pinButtonTurbo);             // Initialize pin
        gpio_set_dir( options.pinButtonTurbo, GPIO_IN); // Set as INPUT
        gpio_pull_up( options.pinButtonTurbo);          // Set as PULLUP
    }

    // Turbo Dial
    uint8_t turboShotCount = options.turboShotCount;
    if ( options.pinShmupDial != (uint8_t)-1 ) {
        adc_gpio_init(options.pinShmupDial);
        adcShmupDial = 26 - options.pinShmupDial;
        adc_select_input(adcShmupDial);
        dialValue = adc_read(); // setup initial Dial + Turbo Speed
        turboShotCount = (dialValue / turboDialIncrements) + TURBO_SHOT_MIN;
    } else {
        dialValue = 0;
    }

    // Setup Turbo LED if available
    if (options.pinTurboLED != (uint8_t)-1) {
        gpio_init(options.pinTurboLED);
        gpio_set_dir(options.pinTurboLED, GPIO_OUT);
        gpio_put(options.pinTurboLED, 1);
    }

    // SHMUP Mode
    if ( options.shmupMode == 1 ) {
        shmupBtnPin[0] = options.pinShmupBtn1; // Charge Buttons Pins
        shmupBtnPin[1] = options.pinShmupBtn2;
        shmupBtnPin[2] = options.pinShmupBtn3;
        shmupBtnPin[3] = options.pinShmupBtn4;
        for (uint8_t i = 0; i < 4; i++) {
            if ( shmupBtnPin[i] != (uint8_t)-1 ) {
                gpio_init(shmupBtnPin[i]);
                gpio_set_dir(shmupBtnPin[i], GPIO_IN);
                gpio_pull_up(shmupBtnPin[i]);
            }
        }
        shmupBtnMask[0] = options.shmupBtnMask1; // Charge Buttons Assignment
        shmupBtnMask[1] = options.shmupBtnMask2;
        shmupBtnMask[2] = options.shmupBtnMask3;
        shmupBtnMask[3] = options.shmupBtnMask4;
        alwaysEnabled = options.shmupAlwaysOn1 | options.shmupAlwaysOn2 |
                            options.shmupAlwaysOn3 | options.shmupAlwaysOn4;
        turboButtonsPressed = alwaysEnabled;
    } else {
        // SHMUP mode off
        alwaysEnabled = 0;
        turboButtonsPressed = 0;
    }

    bDebState = false;
    uDebTime = now;
    debChargeState = 0;
    for(uint8_t i = 0; i < 4; i++) {
        debChargeTime[i] = now;
    }
    debounceMS = gamepad->debounceMS;
    turboDialIncrements = 0xFFF / (TURBO_SHOT_MAX - TURBO_SHOT_MIN); // 12-bit ADC
    incrementValue = 0;
    lastPressed = 0;
    lastDpad = 0;
    uIntervalMS = (uint32_t)(1000.0 / turboShotCount);
    bTurboState = false;
    bTurboFlicker = false;
    nextTimer = getMillis();
}

void TurboInput::read(const AddonOptions & options)
{
    // Get Charge Buttons
    if ( options.shmupMode == 1 ) {
        chargeState = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if ( shmupBtnPin[i] != (uint8_t)-1 ) { // if pin, get the GPIO
                chargeState |= (!gpio_get(shmupBtnPin[i]) ? shmupBtnMask[i] : 0);
            }
        }
    }

    // Get TURBO Key State
    bTurboState = !gpio_get(options.pinButtonTurbo);
}

void TurboInput::debounce()
{
    uint32_t uNowTime = getMillis();

    // Debounce turbo button
    if ((bDebState != bTurboState) && ((uNowTime - uDebTime) > debounceMS)) {
        bDebState ^= true;
        uDebTime = uNowTime;
    }
    bTurboState = bDebState;

    // Debounce charge states
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((debChargeState & shmupBtnMask[i]) != (chargeState & shmupBtnMask[i]) && (uNowTime - debChargeTime[i]) > debounceMS)
		{
			debChargeState ^= shmupBtnMask[i];
			debChargeTime[i] = uNowTime;
		}
	}
    chargeState = debChargeState;
}

void TurboInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    uint16_t buttonsPressed = gamepad->state.buttons & TURBO_BUTTON_MASK;
    uint16_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    // Get Turbo Button States
    read(options);
    debounce();

    // Set TURBO Enable Buttons
    if (bTurboState) {
        if (buttonsPressed && (lastPressed != buttonsPressed)) {
            turboButtonsPressed ^= buttonsPressed; // Toggle Turbo
            if ( options.shmupMode == 1 ) {
                turboButtonsPressed |= alwaysEnabled;  // SHMUP Always-on Buttons Set
            }
            // Turn off button once turbo is toggled
            gamepad->state.buttons &= ~(TURBO_BUTTON_MASK);
        }
        if (dpadPressed & GAMEPAD_MASK_DOWN && (lastDpad != dpadPressed)) {
            if ( options.turboShotCount > TURBO_SHOT_MIN ) { // can't go lower than 2-shots per second
                updateTurboShotCount(options.turboShotCount - 1);
            }
        } else if ( dpadPressed & GAMEPAD_MASK_UP && (lastDpad != dpadPressed)) {
            if ( options.turboShotCount < TURBO_SHOT_MAX ) { // can't go higher than 60-shots per second
                updateTurboShotCount(options.turboShotCount + 1);
            }
        }
        lastPressed = buttonsPressed; // save last pressed
        lastDpad = dpadPressed;
        return; // Holding TURBO cancels turbo functionality
    } else {
        lastPressed = 0; // disable last pressed
        lastDpad = 0; // disable last dpad
    }

    // Use the dial to modify our turbo shot speed (don't save on dial modify)
    if ( options.pinShmupDial != (uint8_t)-1 ) {
        adc_select_input(adcShmupDial);
        uint16_t rawValue = adc_read();
        if ( rawValue != dialValue ) {
            updateTurboShotCount((rawValue / turboDialIncrements) + TURBO_SHOT_MIN);
        }
        dialValue = rawValue;
    }

    // Set TURBO LED if a button is going or turbo is too fast
    if ( options.pinTurboLED != (uint8_t)-1 ) {
        if ((gamepad->state.buttons & turboButtonsPressed) && !bTurboFlicker) {
            gpio_put(options.pinTurboLED, 0);
        } else {
            gpio_put(options.pinTurboLED, 1);
        }
    }

    // set charge buttons (mix mode)
    if ( options.shmupMode == 1 ) {
        gamepad->state.buttons |= chargeState;  // Inject Mask into button states
    }

    // Disable button during turbo flicker
    if (bTurboFlicker) {
        if ( options.shmupMode && options.shmupMixMode == CHARGE_PRIORITY) {
            gamepad->state.buttons &= ~(turboButtonsPressed & ~(chargeState));  // Do not flicker charge buttons
        } else {
            gamepad->state.buttons &= ~(turboButtonsPressed);
        }
    }

    if (getMillis() < nextTimer) {
        return; // don't flip if we haven't reached the next timer
    }

    bTurboFlicker ^= true; // Button ON/OFF State Reverse
    nextTimer = getMillis() + uIntervalMS; // interval to flicker-off button
}

void TurboInput::updateTurboShotCount(uint8_t turboShotCount)
{
    AddonOptions options = Storage::getInstance().getAddonOptions();
    options.turboShotCount = std::clamp<uint8_t>(turboShotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    Storage::getInstance().setAddonOptions(options);
    uIntervalMS = (uint32_t)(1000.0 / options.turboShotCount);
}
