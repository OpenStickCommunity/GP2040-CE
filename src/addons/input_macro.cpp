#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

#define INPUT_HOLD_MS 16

static MacroOptions inputMacroOptions;

bool InputMacro::available() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;
	return inputMacroOptions.enabled;
}

int macroPosition = -1;
int macroInputPosition = 0;
bool bootselPressed = 0;
uint32_t macroInputHoldTime = INPUT_HOLD_MS;
bool prevBootselPressed = 0;
bool isProcessing = 0;
bool trigger = false;
bool hasInit = false;
uint32_t heldAt = 0;

void InputMacro::setup() {
    macroInputPosition = 0;
    isProcessing = 0;
    bootselPressed = 0;
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
                    bootselPressed = true;
                    macroPosition = i; break;
                }
            } else {
                if ((allPins & 1 << macro.macroTriggerPin)) {
                    bootselPressed = true;
                    macroPosition = i; break;
                }
            }
        } 

        if (macroPosition != -1) {
            isProcessing = false;
            heldAt = 0;
        } else {
            prevBootselPressed = false; 
            return;
        }
    }

    Macro& macro = inputMacroOptions.macroList[macroPosition];

    if (macro.useMacroTriggerButton) {
        bootselPressed = (allPins & 1 << inputMacroOptions.pin)
            && (gamepad->state.buttons & macro.macroTriggerButton);
    } else {
        bootselPressed = (allPins & 1 << macro.macroTriggerPin);
    }

    // light_up(bootselPressed);
    if (!isProcessing) {
        switch (macro.macroType) {
            case ON_RELEASE_TOGGLE:
            case ON_RELEASE:
                trigger = prevBootselPressed && !bootselPressed;
                break;
            case ON_HOLD:
                trigger = !prevBootselPressed && bootselPressed;
                break;
            case ON_HOLD_REPEAT:
                trigger = bootselPressed;
                break;
            default:
                break;
        }
    } else {
        switch (macro.macroType) {
            case ON_RELEASE_TOGGLE:
                if (prevBootselPressed && !bootselPressed)
                    trigger = false;
                break;
            case ON_HOLD_REPEAT:
                trigger = bootselPressed;
                break;
        }
    }

    prevBootselPressed = bootselPressed;

    MacroInput& macroInput = macro.macroInputs[macroInputPosition];
    uint32_t macroInputDuration = macroInput.duration + macroInput.waitDuration;
    uint32_t currentMillis = getMillis();

    if (!isProcessing && trigger) {
        isProcessing = 1;
        heldAt = currentMillis;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_MS : macroInputDuration;
    }
    
    if (!isProcessing) return;

    if (!trigger && macro.interruptible) {
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

    if ((currentMillis - heldAt) <= macroInput.duration) {
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

    if ((currentMillis - heldAt) >= macroInputHoldTime) {
        heldAt = currentMillis; macroInputPosition++;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_MS : macroInputDuration;
    }
    
    if (isProcessing && macroInputPosition >= (macro.macroInputs_count)) {
        macroInputPosition = 0;
        trigger = trigger && (macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT);
        isProcessing = trigger;
        macroPosition = ((macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT) && trigger) ? macroPosition : -1;
        if ((macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT) && !trigger) {
            heldAt = 0;
            macroInputHoldTime = INPUT_HOLD_MS;
        }
    }
}