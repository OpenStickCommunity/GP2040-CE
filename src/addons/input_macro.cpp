#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

bool InputMacro::available() {
    // Macro Button initialized by void Gamepad::setup()
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
            case GpioAction::BUTTON_PRESS_MACRO_1:
            case GpioAction::BUTTON_PRESS_MACRO_2:
            case GpioAction::BUTTON_PRESS_MACRO_3:
            case GpioAction::BUTTON_PRESS_MACRO_4:
            case GpioAction::BUTTON_PRESS_MACRO_5:
            case GpioAction::BUTTON_PRESS_MACRO_6:
                return true;
            default:
                break;
        }
    }
    return false;
}

void InputMacro::setup() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
                macroButtonMask = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_1:
                macroPinMasks[0] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_2:
                macroPinMasks[1] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_3:
                macroPinMasks[2] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_4:
                macroPinMasks[3] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_5:
                macroPinMasks[4] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_6:
                macroPinMasks[5] = 1 << pin;
                break;
            default:
                break;
        }
    }

    inputMacroOptions = &Storage::getInstance().getAddonOptions().macroOptions;
    if (inputMacroOptions->macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    } else {
        boardLedEnabled = false;
    }
    boardLedEnabled = false;
    prevMacroInputPressed = false;
    reset();
}


void InputMacro::reset() {
    macroPosition = -1;
    pressedMacro = -1;
    isMacroRunning = false;
    macroStartTime = 0;
    macroInputPosition = 0;
    isMacroTriggerHeld = false;
    macroInputHoldTime = INPUT_HOLD_US;
    if (boardLedEnabled) {
        gpio_put(BOARD_LED_PIN, 0);
    }
}

void InputMacro::restart(Macro& macro) {
    macroStartTime = currentMicros;
    macroInputPosition = 0;
    MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
    uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
    macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
}

void InputMacro::checkMacroPress() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t allPins = gamepad->debouncedGpio;

    // Go through our macro list
    pressedMacro = -1;
    for(int i = 0; i < MAX_MACRO_LIMIT; i++) {
        if ( inputMacroOptions->macroList[i].enabled == false ) // Skip disabled macros
            continue;
        Macro * macro = &inputMacroOptions->macroList[i];
        if ( macro->useMacroTriggerButton ) {
            // Use Gamepad Button for Macro Trigger
            if ((allPins & macroButtonMask) &&
                ((gamepad->state.buttons & macro->macroTriggerButton) ||
                    (gamepad->state.dpad & (macro->macroTriggerButton >> 16))) ) {
                pressedMacro = i;
                break;
            }
        } else if ( allPins & macroPinMasks[i] ) {
            // Use Pin Manager for Macro Trigger
            pressedMacro = i;
            break;
        }
    }
}

void InputMacro::checkMacroAction() {
    bool macroInputPressed = (pressedMacro != -1); // Was any macro input pressed?

    // Is our pressed macro button different from our current macro AND no macro is running?
    if ( pressedMacro != macroPosition && !isMacroRunning ) {
        macroPosition = pressedMacro; // move our position to that macro
    }

    bool newPress = macroInputPressed && (prevMacroInputPressed ^ macroInputPressed);

    // Check to see if we should change the current macro (or turn off based on input)
    if ( inputMacroOptions->macroList[macroPosition].macroType == ON_PRESS ) {
        // START Macro: On Press or On Hold Repeat
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        }
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT ) {
        isMacroTriggerHeld = macroInputPressed;
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_TOGGLE ) {
        //isMacroTriggerHeld = macroInputPressed;
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        } else if (isMacroRunning && newPress) {
            // STOP Macro: Toggle on new press
            reset(); // Stop Macro: Toggle
            prevMacroInputPressed = macroInputPressed;
            return;
        }
    }

    prevMacroInputPressed = macroInputPressed;
    if (!isMacroRunning && isMacroTriggerHeld) {
        // New Macro to run
        macroPosition = pressedMacro; // Set current macro
        Macro& macro = inputMacroOptions->macroList[macroPosition];
        MacroInput& macroInput = macro.macroInputs[macroInputPosition];
        uint32_t macroInputDuration = macroInput.duration + macroInput.waitDuration;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_US : macroInputDuration;
        isMacroRunning = true;
        macroStartTime = getMicro(); // current time
    }
}

void InputMacro::runCurrentMacro() {
    // Do nothing if macro is not currently running
    if (!isMacroRunning ||
            macroPosition == -1)
        return;

    Macro& macro = inputMacroOptions->macroList[macroPosition];

    // Stop Macro if released (ON PRESS & ON HOLD REPEAT)
    if (inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT &&
            !isMacroTriggerHeld ) {
        reset();
        return;
    }

    MacroInput& macroInput = macro.macroInputs[macroInputPosition];
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    currentMicros = getMicro();

    if (!macro.interruptible && macro.exclusive) {
        // Prevent any other inputs from modifying our input (Exclusive)
        gamepad->state.dpad = 0;
        gamepad->state.buttons = 0;
    } else {
        if (macro.useMacroTriggerButton) {
            // Remove the trigger button from the input state
            gamepad->state.dpad &= ~(macro.macroTriggerButton >> 16);
            gamepad->state.buttons &= ~macro.macroTriggerButton;
        }
        if (macro.interruptible &&
            (gamepad->state.buttons != 0 || gamepad->state.dpad != 0)) {
            // Macro is interruptible and a user pressed something
            reset();
            return;
        }
    }

    // Have we elapsed the input hold time?
    if ((currentMicros - macroStartTime) >= macroInputHoldTime) {
        macroStartTime = currentMicros;
        macroInputPosition++;
        
        if (macroInputPosition >= (macro.macroInputs_count)) {
            if ( macro.macroType == ON_PRESS ) {
                reset(); // On press = no more macro
            } else {
                restart(macro); // On Hold-Repeat or On Toggle = start macro again
            }
        } else {
            MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
            uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
            macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
        }
    }

    // Check if we should still hold this macro input based on duration
    if ((currentMicros - macroStartTime) <= macroInput.duration) {
        uint32_t buttonMask = macroInput.buttonMask;
        if (buttonMask & GAMEPAD_MASK_DU) {
            gamepad->state.dpad |= GAMEPAD_MASK_UP;
        }
        if (buttonMask & GAMEPAD_MASK_DD) {
            gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
        }
        if (buttonMask & GAMEPAD_MASK_DL) {
            gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
        }
        if (buttonMask & GAMEPAD_MASK_DR) {
            gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
        }
        gamepad->state.buttons |= buttonMask;

        // Macro LED is on if we're currently running and inputs are doing something (wait-timers turn it off)
        if (boardLedEnabled) {
            gpio_put(BOARD_LED_PIN, (gamepad->state.dpad || gamepad->state.buttons) ? 1 : 0);
        }
    }
}

void InputMacro::preprocess()
{
    FocusModeOptions * focusModeOptions = &Storage::getInstance().getAddonOptions().focusModeOptions;
    if (focusModeOptions->enabled && focusModeOptions->macroLockEnabled)
        return;

    checkMacroPress();
    checkMacroAction();
    runCurrentMacro();
}

void InputMacro::reinit() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
                macroButtonMask = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_1:
                macroPinMasks[0] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_2:
                macroPinMasks[1] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_3:
                macroPinMasks[2] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_4:
                macroPinMasks[3] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_5:
                macroPinMasks[4] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_6:
                macroPinMasks[5] = 1 << pin;
                break;
            default:
                break;
        }
    }
}
