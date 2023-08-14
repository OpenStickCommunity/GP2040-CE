#include "addons/input_macro.h"
#include "storagemanager.h"

#include "GamepadState.h"

#include "hardware/gpio.h"

#define INPUT_HOLD_MS 100

bool InputMacro::available() {
	return true;
}

static MacroOptions inputMacroOptions;

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
}

void InputMacro::preprocess()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint8_t otherButtonsPressed = ((GAMEPAD_MASK_B1 | GAMEPAD_MASK_B2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4) & gamepad->state.buttons);
    if (!hasInit) { if (getMillis() < 1000) return; else hasInit = true; } 
    bootselPressed = otherButtonsPressed;

    if (!isProcessing && gamepad->pressedS2() && !prevBootselPressed) {
        switch (gamepad->state.buttons & (GAMEPAD_MASK_B1 | GAMEPAD_MASK_B2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4)) {
            case GAMEPAD_MASK_B1: macroPosition = 0; break;
            case GAMEPAD_MASK_B2: macroPosition = 1; break;
            case GAMEPAD_MASK_B3: macroPosition = 2; break;
            case GAMEPAD_MASK_B4: macroPosition = 3; break;
            default: break; 
        }
    }
    if (macroPosition == -1) {
        prevBootselPressed = otherButtonsPressed; 
        return;
    }
    Macro& macro = inputMacroOptions.macroList[macroPosition];

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
        }
    }

    prevBootselPressed = bootselPressed;

    auto macroInputs = macro.macroInputs;

    if (!isProcessing && trigger) {
        isProcessing = 1;
        heldAt = getMillis();
        shouldHold = macroInputs[position].duration == -1 ? INPUT_HOLD_MS : macroInputs[position].duration;
    }
    
    if (!isProcessing) return;
    
    gamepad->state.dpad = macroInputs[position].state.dpad;
    gamepad->state.buttons = macroInputs[position].state.buttons;
    
    if ((getMillis() - heldAt) >= shouldHold) {
        heldAt = getMillis(); position++;
        shouldHold = macroInputs[position].duration == -1 ? INPUT_HOLD_MS : macroInputs[position].duration;
    }
    
    if (isProcessing && position >= (macro.macroInputsSize)) {
        position = 0;
        trigger = trigger && macro.macroType == ON_RELEASE_TOGGLE;
        isProcessing = trigger;
        macroPosition = (macro.macroType == ON_RELEASE_TOGGLE && trigger) ? macroPosition : -1;
        if (macro.macroType == ON_RELEASE_TOGGLE && !trigger) {
            heldAt = 0;
            shouldHold = INPUT_HOLD_MS;
        }
    }
}