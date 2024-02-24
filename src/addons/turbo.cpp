#include "addons/turbo.h"

#include "hardware/adc.h"

#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include <algorithm>

#define TURBO_SHOT_MIN 2
#define TURBO_SHOT_MAX 30

bool TurboInput::available() {
    // Turbo Button initialized by void Gamepad::setup()
    bool hasTurboAssigned = false;
    GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        if ( pinMappings[pin] == GpioAction::BUTTON_PRESS_TURBO ) {
            hasTurboAssigned = true;
            turboPin = pin;
            break;
        }
    }
    return Storage::getInstance().getAddonOptions().turboOptions.enabled && (hasTurboAssigned == true);
}

void TurboInput::setup()
{
    const TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    uint32_t now = getMillis();

    // Turbo Dial
    uint8_t shotCount = std::clamp<uint8_t>(options.shotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    if (isValidPin(options.shmupDialPin)) {
        hasShmupDial = true;
        adc_gpio_init(options.shmupDialPin);
        adcShmupDial = 26 - options.shmupDialPin;
        adc_select_input(adcShmupDial);
        dialValue = adc_read(); // setup initial Dial + Turbo Speed
        shotCount = (dialValue / turboDialIncrements) + TURBO_SHOT_MIN;
    } else {
        dialValue = 0;
    }

    // Setup Turbo LED if available
    if (isValidPin(options.ledPin)) {
        hasLedPin = true;
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
                shmupBtnPinMask[i] = 1 << shmupBtnPin[i];
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

    if (isValidPin(turboPin)) {
        turboPinMask = 1 << turboPin;
    }

    lastButtons = 0;
    bDebState = false;
    uDebTime = now;
    debChargeState = 0;
    for(uint8_t i = 0; i < 4; i++) {
        debChargeTime[i] = now;
    }
    turboDialIncrements = 0xFFF / (TURBO_SHOT_MAX - TURBO_SHOT_MIN); // 12-bit ADC
    incrementValue = 0;
    lastPressed = 0;
    lastDpad = 0;
    bTurboFlicker = false;
    updateInterval(shotCount);
    nextTimer = getMillis();
}

void TurboInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    uint16_t buttonsPressed = gamepad->state.buttons & TURBO_BUTTON_MASK;
    uint8_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    // Check for TURBO pin enabled
    if (gamepad->debouncedGpio & turboPinMask) {
        if (buttonsPressed && (lastPressed != buttonsPressed)) {
            turboButtonsPressed ^= buttonsPressed; // Toggle Turbo
            gamepad->turboState.buttons = turboButtonsPressed; //turboButtonsPressed & TURBO_BUTTON_MASK; //&= TURBO_BUTTON_MASK;
            if (options.shmupModeEnabled) {
                turboButtonsPressed |= alwaysEnabled;  // SHMUP Always-on Buttons Set
            }
        }

        if (dpadPressed & GAMEPAD_MASK_DOWN && (lastDpad != dpadPressed)) {
            if (options.shotCount > TURBO_SHOT_MIN) { // can't go lower than 2-shots per second
                updateTurboShotCount(options.shotCount - 1);
            }
        }
        else if (dpadPressed & GAMEPAD_MASK_UP && (lastDpad != dpadPressed)) {
            if (options.shotCount < TURBO_SHOT_MAX) { // can't go higher than 30-shots per second
                updateTurboShotCount(options.shotCount + 1);
            }
        }
        lastPressed = buttonsPressed; // save last pressed
        lastDpad = dpadPressed;

        // Clear gamepad outputs since we're in turbo adjustment mode
        gamepad->clearState();
        return; // Holding TURBO cancels turbo functionality
    } else {
        lastPressed = 0; // disable last pressed
        lastDpad = 0; // disable last dpad
    }

    // Get Charge Buttons
    if (options.shmupModeEnabled) {
        chargeState = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if (shmupBtnPinMask[i] > 0) { // if pin, get the GPIO
                chargeState |= (!(gamepad->debouncedGpio & shmupBtnPinMask[i]) ? shmupBtnMask[i] : 0);
            }
        }
    }

    // Use the dial to modify our turbo shot speed (don't save on dial modify)
    if (hasShmupDial) {
        adc_select_input(adcShmupDial);
        uint16_t rawValue = adc_read();
        if ( rawValue != dialValue ) {
            updateTurboShotCount((rawValue / turboDialIncrements) + TURBO_SHOT_MIN);
        }
        dialValue = rawValue;
    }

    uint64_t now = getMicro();

    // Reset Turbo flicker on a new button press
    if ((lastButtons & turboButtonsPressed) == 0 && (gamepad->state.buttons & turboButtonsPressed) != 0) {
        bTurboFlicker = false; // reset flicker state to ON
        nextTimer = now + uIntervalUS - uOffset; // interval to flicker-off button
    }
    // Check if we've reached the next timer right before applying turbo state
    else if (nextTimer < now) {
        bTurboFlicker ^= true;
        nextTimer = now + uIntervalUS - uOffset;
    }

    // Set TURBO LED if a button is going or turbo is too fast
    if (hasLedPin) {
        if ((gamepad->state.buttons & turboButtonsPressed) && !bTurboFlicker) {
            gpio_put(options.ledPin, 0);
        } else {
            gpio_put(options.ledPin, 1);
        }
    }

    // Button updates
    lastButtons = gamepad->state.buttons;

    // set charge buttons (mix mode)
    if (options.shmupModeEnabled) {
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
}

void TurboInput::updateInterval(uint8_t shotCount)
{
    uIntervalUS = (uint32_t)(1000000.0 / (shotCount * 2));
}

void TurboInput::updateTurboShotCount(uint8_t shotCount)
{
    TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    options.shotCount = std::clamp<uint8_t>(shotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    Storage::getInstance().save();
    updateInterval(shotCount);
}
