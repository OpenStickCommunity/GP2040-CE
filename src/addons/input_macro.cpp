#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

bool InputMacro::available() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;
	return inputMacroOptions.enabled;
}

void InputMacro::setup() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;

	gpio_init(inputMacroOptions.pin);             // Initialize pin
	gpio_set_dir(inputMacroOptions.pin, GPIO_IN); // Set as INPUT
	gpio_pull_up(inputMacroOptions.pin);          // Set as PULLUP

    for (int i = 0; i < inputMacroOptions.macroList_count; i++) {
        Macro& macro = inputMacroOptions.macroList[i];
        if (!macro.enabled) continue;
        if (!macro.useMacroTriggerButton && !isValidPin(macro.macroTriggerPin)) continue;

        gpio_init(macro.macroTriggerPin);             // Initialize pin
        gpio_set_dir(macro.macroTriggerPin, GPIO_IN); // Set as INPUT
        gpio_pull_up(macro.macroTriggerPin);          // Set as PULLUP
    }
}

void InputMacro::preprocess()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
	uint32_t allPins = ~gpio_get_all();

    if (macroPosition == -1) {
        for (int i = 0; i < inputMacroOptions.macroList_count; i++) {
            Macro& macro = inputMacroOptions.macroList[i];
            if (!macro.enabled) continue;
            if (!macro.useMacroTriggerButton && !isValidPin(macro.macroTriggerPin)) continue;
            if (macro.useMacroTriggerButton && macro.macroTriggerButton == 0) continue;

            if (macro.useMacroTriggerButton) {
                if ((allPins & 1 << inputMacroOptions.pin) && (gamepad->state.buttons & macro.macroTriggerButton)) {
                    macroInputPressed = true;
                    macroPosition = i; break;
                }
            } else {
                if ((allPins & 1 << macro.macroTriggerPin)) {
                    macroInputPressed = true;
                    macroPosition = i; break;
                }
            }
        } 

        if (macroPosition != -1) {
            isMacroRunning = false;
            macroStartTime = 0;
        } else {
            prevMacroInputPressed = false; 
            return;
        }
    }

    Macro& macro = inputMacroOptions.macroList[macroPosition];

    if (macro.useMacroTriggerButton) {
        macroInputPressed = (allPins & 1 << inputMacroOptions.pin)
            && (gamepad->state.buttons & macro.macroTriggerButton);
    } else {
        macroInputPressed = (allPins & 1 << macro.macroTriggerPin);
    }

    uint32_t currentMillis = getMillis();

    if (!isMacroRunning && macroInputPressed && macroTriggerDebounceStartTime == 0) {
        macroTriggerDebounceStartTime = currentMillis;
        return;
    }

    if (macroTriggerDebounceStartTime != 0) {
        if (((currentMillis - macroTriggerDebounceStartTime) > 5)) {
            macroTriggerDebounceStartTime = 0;
        } else {
            return;
        }
    }

    if (!isMacroRunning) {
        switch (macro.macroType) {
            case ON_RELEASE_TOGGLE:
            case ON_RELEASE:
                isMacroTriggerHeld = prevMacroInputPressed && !macroInputPressed;
                break;
            case ON_HOLD:
                isMacroTriggerHeld = !prevMacroInputPressed && macroInputPressed;
                break;
            case ON_HOLD_REPEAT:
                isMacroTriggerHeld = macroInputPressed;
                break;
            default:
                break;
        }
    } else {
        switch (macro.macroType) {
            case ON_RELEASE_TOGGLE:
                if (prevMacroInputPressed && !macroInputPressed)
                    isMacroTriggerHeld = false;
                break;
            case ON_HOLD_REPEAT:
                isMacroTriggerHeld = macroInputPressed;
                break;
        }
    }
    prevMacroInputPressed = macroInputPressed;

    MacroInput& macroInput = macro.macroInputs[macroInputPosition];
    uint32_t macroInputDuration = macroInput.duration + macroInput.waitDuration;

    if (!isMacroRunning && isMacroTriggerHeld) {
        isMacroRunning = true;
        macroStartTime = currentMillis;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_MS : macroInputDuration;
    }
    
    if (!isMacroRunning || (!isMacroTriggerHeld && macro.interruptible)) {
        macroPosition = -1;
        return;
    }

    if (macro.exclusive) {
        gamepad->state.dpad = 0;
        gamepad->state.buttons = 0;
    } else {
        if (macro.macroTriggerButton & GAMEPAD_MASK_DU) {
            gamepad->state.dpad ^= GAMEPAD_MASK_UP;
        }
        if (macro.macroTriggerButton & GAMEPAD_MASK_DD) {
            gamepad->state.dpad ^= GAMEPAD_MASK_DOWN;
        }
        if (macro.macroTriggerButton & GAMEPAD_MASK_DL) {
            gamepad->state.dpad ^= GAMEPAD_MASK_LEFT;
        }
        if (macro.macroTriggerButton & GAMEPAD_MASK_DR) {
            gamepad->state.dpad ^= GAMEPAD_MASK_RIGHT;
        }
        gamepad->state.buttons ^= macro.macroTriggerButton;
    }

    if ((currentMillis - macroStartTime) <= macroInput.duration) {
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
    }

    if ((currentMillis - macroStartTime) >= macroInputHoldTime) {
        macroStartTime = currentMillis; macroInputPosition++;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_MS : macroInputDuration;
    }
    
    if (isMacroRunning && macroInputPosition >= (macro.macroInputs_count)) {
        macroInputPosition = 0;
        bool isMacroTypeLoopable = macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT;
        isMacroTriggerHeld = isMacroTriggerHeld && isMacroTypeLoopable;
        isMacroRunning = isMacroTriggerHeld;
        macroPosition = (isMacroTypeLoopable && isMacroTriggerHeld) ? macroPosition : -1;
        if (isMacroTypeLoopable && !isMacroTriggerHeld) {
            macroStartTime = 0;
            macroInputHoldTime = INPUT_HOLD_MS;
        }
    }
}