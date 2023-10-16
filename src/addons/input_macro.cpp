#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

bool InputMacro::available() {
    return Storage::getInstance().getAddonOptions().macroOptions.enabled;
}

void InputMacro::setup() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;

	gpio_init(inputMacroOptions.pin);             // Initialize pin
	gpio_set_dir(inputMacroOptions.pin, GPIO_IN); // Set as INPUT
	gpio_pull_up(inputMacroOptions.pin);          // Set as PULLUP

    if (inputMacroOptions.macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    }

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
    FocusModeOptions& focusModeOptions = Storage::getInstance().getAddonOptions().focusModeOptions;
    if (focusModeOptions.enabled && focusModeOptions.macroLockEnabled) return;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
	uint32_t allPins = ~gpio_get_all();
    macroInputPressed = false;
    uint64_t currentMicros = getMicro();

    if (macroPosition == -1 || inputMacroOptions.macroList[macroPosition].interruptible) {
        int newMacroPosition = -1;
        for (int i = 0; i < inputMacroOptions.macroList_count; i++) {
            Macro& macro = inputMacroOptions.macroList[i];
            if (!macro.enabled) continue;

            if (macro.useMacroTriggerButton) {
                if (macro.macroTriggerButton == 0) continue;
                if ((allPins & 1 << inputMacroOptions.pin) &&
                    ((gamepad->state.buttons & macro.macroTriggerButton) ||
                    (gamepad->state.dpad & (macro.macroTriggerButton >> 16)))) {
                    macroInputPressed = true;
                    newMacroPosition = i; break;
                }
            } else {
                if (!isValidPin(macro.macroTriggerPin)) continue;
                if ((allPins & 1 << macro.macroTriggerPin)) {
                    macroInputPressed = true;
                    newMacroPosition = i; break;
                }
            }
        } 

        if (macroPosition == -1 && newMacroPosition == -1)
            return;

        if (macroPosition != -1 && newMacroPosition != -1) {
            if (newMacroPosition != macroPosition ||
                (newMacroPosition == macroPosition &&
                 inputMacroOptions.macroList[newMacroPosition].macroType == ON_PRESS &&
                 isMacroRunning && (macroTriggerDebounceStartTime != 0 || (!prevMacroInputPressed && macroInputPressed)))) {
                    if (macroTriggerDebounceStartTime == 0) {
                        macroTriggerDebounceStartTime = currentMicros;
                    }

                    if (macroTriggerDebounceStartTime != 0) {
                        if (((currentMicros - macroTriggerDebounceStartTime) > 500)) {
                            macroTriggerDebounceStartTime = 0;
                            if (macroInputPressed) {
                                reset();
                                return;
                            }
                        }
                    }
            }
        }

        if (newMacroPosition != -1 && !isMacroRunning) {
            macroPosition = newMacroPosition;
            macroStartTime = 0;
        }
    }

    Macro& macro = inputMacroOptions.macroList[macroPosition];

    if (macro.useMacroTriggerButton) {
        macroInputPressed = (allPins & 1 << inputMacroOptions.pin) &&
                            ((gamepad->state.buttons & macro.macroTriggerButton) ||
                             (gamepad->state.dpad & (macro.macroTriggerButton >> 16)));
    } else {
        macroInputPressed = (allPins & 1 << macro.macroTriggerPin);
    }

    if (!isMacroRunning && macroInputPressed && macroTriggerDebounceStartTime == 0) {
        macroTriggerDebounceStartTime = currentMicros;
        return;
    }

    if (macroTriggerDebounceStartTime != 0) {
        if (((currentMicros - macroTriggerDebounceStartTime) > 500)) {
            macroTriggerDebounceStartTime = 0;
        } else {
            return;
        }
    }

    if (!isMacroRunning) {
        switch (macro.macroType) {
            case ON_TOGGLE:
                isMacroTriggerHeld = prevMacroInputPressed && !macroInputPressed;
                break;
            case ON_PRESS:
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
            case ON_PRESS:
                break; // no-op
            case ON_TOGGLE:
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
    macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_US : macroInputDuration;

    if (!isMacroRunning && isMacroTriggerHeld) {
        isMacroRunning = true;
        macroStartTime = currentMicros;
    }
    
    if (!isMacroRunning)
        return;
    
    if ((!isMacroTriggerHeld && macro.interruptible)) {
        reset();
        return;
    }

    if (!macro.interruptible && macro.exclusive) {
        gamepad->state.dpad = 0;
        gamepad->state.buttons = 0;
    } else {
        if (macro.useMacroTriggerButton) {
            gamepad->state.dpad &= ~(macro.macroTriggerButton >> 16);
            gamepad->state.buttons &= ~macro.macroTriggerButton;
        }
        if (macro.interruptible && (gamepad->state.buttons != 0 || gamepad->state.dpad != 0)) {
            reset();
            return;
        }
    }

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

        if (boardLedEnabled) {
            gpio_put(BOARD_LED_PIN, (gamepad->state.dpad || gamepad->state.buttons) ? 1 : 0);
        }
    } else {
        if (boardLedEnabled) {
            gpio_put(BOARD_LED_PIN, 0);
        }
    }

    if ((currentMicros - macroStartTime) >= macroInputHoldTime) {
        macroStartTime = currentMicros; macroInputPosition++;
        MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
        uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
        macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
    }
    
    if (isMacroRunning && macroInputPosition >= (macro.macroInputs_count)) {
        macroInputPosition = 0;
        bool isMacroTypeLoopable = macro.macroType == ON_TOGGLE || macro.macroType == ON_HOLD_REPEAT;
        isMacroTriggerHeld = isMacroTriggerHeld && isMacroTypeLoopable;
        isMacroRunning = isMacroTriggerHeld;
        macroPosition = (isMacroTypeLoopable && isMacroTriggerHeld) ? macroPosition : -1;
        macroStartTime = currentMicros;
    }
}

void InputMacro::reset() {
    macroPosition = -1;
    isMacroRunning = false;
    macroStartTime = 0;
    prevMacroInputPressed = false;
    macroInputPosition = 0;
}