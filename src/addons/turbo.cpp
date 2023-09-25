#include "addons/turbo.h"

#include "hardware/adc.h"

#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include <algorithm>

#define TURBO_SHOT_MIN 2
#define TURBO_SHOT_MAX 30

bool TurboInput::available() {
    return Storage::getInstance().getAddonOptions().turboOptions.enabled;
}

void TurboInput::setup()
{
    const TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint32_t now = getMillis();

    // Setup TURBO Key GPIO
    if (  isValidPin(options.buttonPin) ) {
        gpio_init( options.buttonPin);             // Initialize pin
        gpio_set_dir( options.buttonPin, GPIO_IN); // Set as INPUT
        gpio_pull_up( options.buttonPin);          // Set as PULLUP
    }

    // Turbo Dial
    uint8_t shotCount = std::clamp<uint8_t>(options.shotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    if ( isValidPin(options.shmupDialPin) ) {
        adc_gpio_init(options.shmupDialPin);
        adcShmupDial = 26 - options.shmupDialPin;
        adc_select_input(adcShmupDial);
        dialValue = adc_read(); // setup initial Dial + Turbo Speed
        shotCount = (dialValue / turboDialIncrements) + TURBO_SHOT_MIN;
    } else {
        dialValue = 0;
    }

    // Setup Turbo LED if available
    if ( isValidPin(options.ledPin) ) {
        gpio_init(options.ledPin);
        gpio_set_dir(options.ledPin, GPIO_OUT);
        gpio_put(options.ledPin, 1);
    }

    // SHMUP Mode
    if ( options.shmupModeEnabled ) {
        shmupBtnPin[0] = options.shmupBtn1Pin; // Charge Buttons Pins
        shmupBtnPin[1] = options.shmupBtn2Pin;
        shmupBtnPin[2] = options.shmupBtn3Pin;
        shmupBtnPin[3] = options.shmupBtn4Pin;
        for (uint8_t i = 0; i < 4; i++) {
            if ( isValidPin(shmupBtnPin[i]) ) {
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

    lastButtons = 0;
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
    uIntervalMS = (uint32_t)(1000.0 / shotCount);
    bTurboState = false;
    bTurboFlicker = false;
    nextTimer = getMillis();
}

void TurboInput::read(const TurboOptions & options)
{
    // Get Charge Buttons
    if ( options.shmupModeEnabled ) {
        chargeState = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if ( shmupBtnPin[i] != (uint8_t)-1 ) { // if pin, get the GPIO
                chargeState |= (!gpio_get(shmupBtnPin[i]) ? shmupBtnMask[i] : 0);
            }
        }
    }

    // Get TURBO Key State
    bTurboState = !gpio_get(options.buttonPin);
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
    const TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    uint16_t buttonsPressed = gamepad->state.buttons & TURBO_BUTTON_MASK;
    uint16_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    // Get Turbo Button States
    read(options);
    debounce();

    // Set TURBO Enable Buttons
    if (bTurboState) {
        if (buttonsPressed && (lastPressed != buttonsPressed)) {
            turboButtonsPressed ^= buttonsPressed; // Toggle Turbo
            if ( options.shmupModeEnabled ) {
                turboButtonsPressed |= alwaysEnabled;  // SHMUP Always-on Buttons Set
            }
            // Turn off button once turbo is toggled
            gamepad->state.buttons &= ~(TURBO_BUTTON_MASK);
        }
        if (dpadPressed & GAMEPAD_MASK_DOWN && (lastDpad != dpadPressed)) {
            if ( options.shotCount > TURBO_SHOT_MIN ) { // can't go lower than 2-shots per second
                updateTurboShotCount(options.shotCount - 1);
            }
        } else if ( dpadPressed & GAMEPAD_MASK_UP && (lastDpad != dpadPressed)) {
            if ( options.shotCount < TURBO_SHOT_MAX ) { // can't go higher than 60-shots per second
                updateTurboShotCount(options.shotCount + 1);
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
    if ( isValidPin(options.shmupDialPin) ) {
        adc_select_input(adcShmupDial);
        uint16_t rawValue = adc_read();
        if ( rawValue != dialValue ) {
            updateTurboShotCount((rawValue / turboDialIncrements) + TURBO_SHOT_MIN);
        }
        dialValue = rawValue;
    }

    // Reset Turbo flicker on a new button press
    if((lastButtons & turboButtonsPressed) == 0 && (gamepad->state.buttons & turboButtonsPressed) != 0){
        bTurboFlicker = false; // reset flicker state to ON
        nextTimer = getMillis() + uIntervalMS; // interval to flicker-off button
    }
    lastButtons = gamepad->state.buttons;

    // Set TURBO LED if a button is going or turbo is too fast
    if ( isValidPin(options.ledPin) ) {
        if ((gamepad->state.buttons & turboButtonsPressed) && !bTurboFlicker) {
            gpio_put(options.ledPin, 0);
        } else {
            gpio_put(options.ledPin, 1);
        }
    }

    // set charge buttons (mix mode)
    if ( options.shmupModeEnabled ) {
        gamepad->state.buttons |= chargeState;  // Inject Mask into button states
    }

    // Disable button during turbo flicker
    if (bTurboFlicker) {
        if ( options.shmupModeEnabled && options.shmupMixMode == SHMUP_MIX_MODE_CHARGE_PRIORITY) {
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

void TurboInput::updateTurboShotCount(uint8_t shotCount)
{
    TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    options.shotCount = std::clamp<uint8_t>(shotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    Storage::getInstance().save();
    uIntervalMS = (uint32_t)(1000.0 / options.shotCount);
}
