#include "addons/turbo.h"

#include "hardware/adc.h"

#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include <algorithm>
#include <cmath>

#define TURBO_SHOT_MIN 2
#define TURBO_SHOT_MAX 30
#define TURBO_DIAL_INCREMENTS (0xFFF / (TURBO_SHOT_MAX - TURBO_SHOT_MIN)) // 12-bit ADC
#define TURBO_LOOP_OFFSET 50 // Extra time to compensate for loop runtime variance, turbo lags a bit otherwise

#ifndef TURBO_LED_STATE_OFF
#define TURBO_LED_STATE_OFF 0
#endif

#ifndef TURBO_LED_STATE_ON
#define TURBO_LED_STATE_ON 1
#endif

bool TurboInput::available() {
    // Turbo Button initialized by void Gamepad::setup()
    hasTurboAssigned = false;
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        if ( pinMappings[pin].action == GpioAction::BUTTON_PRESS_TURBO ) {
            hasTurboAssigned = true;
            turboPinMask = 1 << pin;
            break;
        }
    }
    return Storage::getInstance().getAddonOptions().turboOptions.enabled && (hasTurboAssigned == true);
}

void TurboInput::setup(){
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
        shotCount = (dialValue / TURBO_DIAL_INCREMENTS) + TURBO_SHOT_MIN;
    } else {
        dialValue = 0;
    }

    // Setup Turbo PWM LED if available (RGB is handled in neopicoleds.cpp)
    if (isValidPin(options.ledPin) && options.turboLedType == PLED_TYPE_PWM) {
        hasLedPin = true;
        gpio_init(options.ledPin);
        gpio_set_dir(options.ledPin, GPIO_OUT);
        gpio_put(options.ledPin, 1);
    }

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	gamepad->auxState.turbo.enabled = true;
    gamepad->auxState.turbo.active = 1;

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
        turboButtonsMask = alwaysEnabled;
    } else {
        // SHMUP mode off
        alwaysEnabled = 0;
        turboButtonsMask = 0;
    }

    lastButtons = 0;
    bDebState = false;
    uDebTime = now;
    debChargeState = 0;
    for(uint8_t i = 0; i < 4; i++) {
        debChargeTime[i] = now;
    }
    incrementValue = 0;
    lastPressed = 0;
    lastDpad = 0;
    bTurboFlicker = false;
    updateInterval(shotCount);
    nextTimer = getMicro();
    encoderValue = shotCount;
}

/**
 * @brief Reset the turbo pin mask.
 */
void TurboInput::reinit()
{
    turboPinMask = 0;
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        if ( pinMappings[pin].action == GpioAction::BUTTON_PRESS_TURBO ) {
            turboPinMask = 1 << pin;
            break;
        }
    }
}

void TurboInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    uint16_t buttonsPressed = gamepad->state.buttons & TURBO_BUTTON_MASK;
    uint8_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    if (!options.enabled && (!hasTurboAssigned == true)) return;

    // Check for TURBO pin enabled
    if (gamepad->debouncedGpio & turboPinMask) {
        if (buttonsPressed && (lastPressed != buttonsPressed)) {
            // Only toggle state for buttons changed that are pressed
            turboButtonsMask ^= (lastPressed ^ buttonsPressed) & ~lastPressed; // Toggle Turbo
            gamepad->turboState.buttons = turboButtonsMask; //turboButtonsMask & TURBO_BUTTON_MASK; //&= TURBO_BUTTON_MASK;
            if (options.shmupModeEnabled) {
                turboButtonsMask |= alwaysEnabled;  // SHMUP Always-on Buttons Set
            }

            // Reset Turbo flicker on a new button press
            bTurboFlicker = false;
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

    uint64_t now = getMicro();

    // Use the dial to modify our turbo shot speed (don't save on dial modify)
    if (hasShmupDial && nextAdcRead < now) {
        adc_select_input(adcShmupDial);
        dialValue = adc_read();
        uint8_t shotCount = (dialValue / TURBO_DIAL_INCREMENTS) + TURBO_SHOT_MIN;
        if (shotCount != options.shotCount) {
            updateInterval(shotCount);
        }
        nextAdcRead = now + 100000; // Sample every 100ms
    }

    // Check if we've reached the next timer right before applying turbo state
    if (nextTimer < now) {
        bTurboFlicker ^= true;
        nextTimer = now + uIntervalUS - TURBO_LOOP_OFFSET;
    }

    // Set TURBO LED
    // OFF: No turbo buttons enabled
    // ON: 1 or more turbo buttons enabled
    // BLINK: OFF on turbo shot, ON on turbo flicker
    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
    if (turboButtonsMask) {
        if (gamepad->state.buttons & turboButtonsMask)
            processedGamepad->auxState.turbo.activity = bTurboFlicker ? TURBO_LED_STATE_ON : TURBO_LED_STATE_OFF;
        else
            processedGamepad->auxState.turbo.activity = TURBO_LED_STATE_ON;
    } else {
        processedGamepad->auxState.turbo.activity = TURBO_LED_STATE_OFF;
    }

    // PWM LED Pin
    if ( hasLedPin ) {
        if ( processedGamepad->auxState.turbo.activity == TURBO_LED_STATE_ON ) {
            gpio_put(options.ledPin, TURBO_LED_STATE_ON);
        } else {
            gpio_put(options.ledPin, TURBO_LED_STATE_OFF);
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
            gamepad->state.buttons &= ~(turboButtonsMask & ~(chargeState));  // Do not flicker charge buttons
        } else {
            gamepad->state.buttons &= ~(turboButtonsMask);
        }
    }
}

void TurboInput::updateInterval(uint8_t shotCount) {
    uIntervalUS = (uint32_t)std::floor(1000000.0 / (shotCount * 2));
}

void TurboInput::updateTurboShotCount(uint8_t shotCount) {
    TurboOptions& options = Storage::getInstance().getAddonOptions().turboOptions;
    shotCount = std::clamp<uint8_t>(shotCount, TURBO_SHOT_MIN, TURBO_SHOT_MAX);
    if (shotCount != options.shotCount) {
        options.shotCount = shotCount;
        Storage::getInstance().save();
    }
    updateInterval(shotCount);
}
