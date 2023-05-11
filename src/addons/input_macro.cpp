#include "addons/input_macro.h"
#include "storagemanager.h"

#include "GamepadState.h"

#include "hardware/gpio.h"

#define INPUT_HOLD_MS 100

bool InputMacro::available() {
	return true;
}

Macros macros;

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
    macros = Storage::getInstance().getMacrosForInit();
}

void InputMacro::preprocess()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint8_t otherButtonsPressed = ((GAMEPAD_MASK_B1 | GAMEPAD_MASK_B2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4) & gamepad->state.buttons);
    if (!hasInit) { if (getMillis() < 1000) return; else hasInit = true; } 
    bootselPressed = otherButtonsPressed;

    if (!isProcessing && gamepad->pressedF2() && !prevBootselPressed) {
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
    Macro& macro = macros.list[macroPosition];

    // light_up(bootselPressed);
    if (!isProcessing) {
        switch (macro.type) {
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
        switch (macro.type) {
            case ON_RELEASE_TOGGLE:
                if (prevBootselPressed && !bootselPressed)
                    trigger = false;
                break;
        }
    }

    prevBootselPressed = bootselPressed;

    auto inputs = macro.inputs;

    if (!isProcessing && trigger) {
        isProcessing = 1;
        heldAt = getMillis();
        shouldHold = inputs[position].duration == -1 ? INPUT_HOLD_MS : inputs[position].duration;
    }
    
    if (!isProcessing) return;
    
    gamepad->state = inputs[position].state;
    
    if ((getMillis() - heldAt) >= shouldHold) {
        heldAt = getMillis(); position++;
        shouldHold = inputs[position].duration == -1 ? INPUT_HOLD_MS : inputs[position].duration;
    }
    
    if (isProcessing && position >= (macro.size)) {
        position = 0;
        trigger = trigger && macro.type == ON_RELEASE_TOGGLE;
        isProcessing = trigger;
        macroPosition = (macro.type == ON_RELEASE_TOGGLE && trigger) ? macroPosition : -1;
        if (macro.type == ON_RELEASE_TOGGLE && !trigger) {
            heldAt = 0;
            shouldHold = INPUT_HOLD_MS;
        }
    }
}