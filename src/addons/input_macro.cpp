#include "addons/input_macro.h"
#include "storagemanager.h"

#include "GamepadState.h"

#include "hardware/gpio.h"

#define INPUT_HOLD_MS 100

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
}

void InputMacro::preprocess()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint16_t otherButtonsPressed = (gamepad->state.buttons);
    if (!hasInit) { if (getMillis() < 1000) return; else hasInit = true; }
    bootselPressed = otherButtonsPressed;

    if (!isProcessing && !gpio_get(inputMacroOptions.pin) && !prevBootselPressed) {
        switch (gamepad->state.buttons & ((1 << 14) - 1)) {
            case GAMEPAD_MASK_B1: macroPosition = 0; break;
            case GAMEPAD_MASK_B2: macroPosition = 1; break;
            case GAMEPAD_MASK_B3: macroPosition = 2; break;
            case GAMEPAD_MASK_B4: macroPosition = 3; break;
            case GAMEPAD_MASK_L1: macroPosition = 4; break;
            case GAMEPAD_MASK_R1: macroPosition = 5; break;
            case GAMEPAD_MASK_L2: macroPosition = 6; break;
            case GAMEPAD_MASK_R2: macroPosition = 7; break;
            case GAMEPAD_MASK_S1: macroPosition = 8; break;
            case GAMEPAD_MASK_S2: macroPosition = 9; break;
            case GAMEPAD_MASK_L3: macroPosition = 10; break;
            case GAMEPAD_MASK_R3: macroPosition = 11; break;
            case GAMEPAD_MASK_A1: macroPosition = 12; break;
            case GAMEPAD_MASK_A2: macroPosition = 13; break;
            default: break; 
        }
    }

    if (macroPosition == -1) {
        prevBootselPressed = otherButtonsPressed; 
        return;
    }

    Macro& macro = inputMacroOptions.macroList[macroPosition];
    if (!macro.enabled) {
        macroPosition = -1;
        return;
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
        shouldHold = macroInputs[position].duration == -1 ? INPUT_HOLD_MS : macroInputs[position].duration;
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