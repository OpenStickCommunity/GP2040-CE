#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

#define INPUT_HOLD_MS 24

static MacroOptions inputMacroOptions;

bool InputMacro::available() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;
	return inputMacroOptions.enabled;
}

int macroPosition = -1;
int position = 0;
bool bootselPressed = 0;
int shouldHold = INPUT_HOLD_MS;
bool prevBootselPressed = 0;
bool isProcessing = 0;
bool trigger = false;
bool hasInit = false;
int64_t heldAt = 0;

void InputMacro::setup() {
    position = 0;
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
    uint16_t otherButtonsPressed = (gamepad->state.buttons);
    if (!hasInit) { if (getMillis() < 1000) return; else hasInit = true; }
    bootselPressed = otherButtonsPressed;

    if (!isProcessing && !prevBootselPressed) {
        for (int i = 0; i < inputMacroOptions.macroList_count; i++) {
            Macro& macro = inputMacroOptions.macroList[i];
            if (!macro.enabled) continue;
            if (!macro.useMacroTriggerButton && !isValidPin(macro.macroTriggerPin)) continue;
            if (macro.useMacroTriggerButton && macro.macroTriggerButton == 0) continue;

            if (macro.useMacroTriggerButton) {
                if (!gpio_get(inputMacroOptions.pin) && (gamepad->state.buttons & macro.macroTriggerButton)) {
                    bootselPressed = true;
                    macroPosition = i; break;
                }
            } else {
                if (!gpio_get(macro.macroTriggerPin)) {
                    bootselPressed = true;
                    macroPosition = i; break;
                }
            }
        } 
    }

    if (macroPosition == -1) {
        prevBootselPressed = false; 
        return;
    }

    Macro& macro = inputMacroOptions.macroList[macroPosition];
    if (macro.useMacroTriggerButton) {
        if (!gpio_get(inputMacroOptions.pin) && (gamepad->state.buttons & macro.macroTriggerButton)) {
            bootselPressed = true;
        }
    } else {
        if (!gpio_get(macro.macroTriggerPin)) {
            bootselPressed = true;
        }
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

    auto macroInputs = macro.macroInputs;

    if (!isProcessing && trigger) {
        isProcessing = 1;
        heldAt = getMillis();
        shouldHold = macroInputs[position].duration <= 0 ? INPUT_HOLD_MS : macroInputs[position].duration;
    }
    
    if (!isProcessing) return;

    uint32_t buttonMask = macroInputs[position].buttonMask;
    gamepad->state.dpad = 0;
    gamepad->state.buttons = 0;
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

    if ((getMillis() - heldAt) >= shouldHold) {
        heldAt = getMillis(); position++;
        shouldHold = macroInputs[position].duration == -1 ? INPUT_HOLD_MS : macroInputs[position].duration;
    }
    
    if (isProcessing && position >= (macro.macroInputs_count)) {
        position = 0;
        trigger = trigger && (macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT);
        isProcessing = trigger;
        macroPosition = ((macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT) && trigger) ? macroPosition : -1;
        if ((macro.macroType == ON_RELEASE_TOGGLE || macro.macroType == ON_HOLD_REPEAT) && !trigger) {
            heldAt = 0;
            shouldHold = INPUT_HOLD_MS;
        }
    }
}