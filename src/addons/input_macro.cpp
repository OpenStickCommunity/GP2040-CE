#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

bool InputMacro::available() {
    // Macro Button initialized by void Gamepad::setup()
    bool hasMacroAssigned = false;
    GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        if ( pinMappings[pin] == GpioAction::BUTTON_PRESS_MACRO ) {
            hasMacroAssigned = true;
            macroPin = pin;
            break;
        }
    }
    return Storage::getInstance().getAddonOptions().macroOptions.enabled && (hasMacroAssigned == true);
}

void InputMacro::setup() {
    inputMacroOptions = Storage::getInstance().getAddonOptions().macroOptions;

    // inputMacroOptions.pin = macro pin
    if (inputMacroOptions.macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    }

    macroPinMask = macroPin << 1;
}

uint32_t InputMacro::checkMacroPress() {
    if ( inputMacroOptions.macroList[activeMacroIdx].interruptible == false )
        return -1;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t allPins = gamepad->debouncedGpio;
    
    // Go through our macro list
    for(int i = 0; i < MAX_MACRO_INPUT_LIMIT; i++) {
        if ( inputMacroOptions.macroList[i].enabled == false ) // Skip
            continue;
        
        Macro * macro = &inputMacroOptions.macroList[i];
        if ( macro->useMacroTriggerButton ) {
            if ((allPins & macroPinMask) &&
                ((gamepad->state.buttons & macro->macroTriggerButton) ||
                    (gamepad->state.dpad & (macro->macroTriggerButton >> 16)) )) {
                return i;
            }
        } else {
            if (!isValidPin(macro->macroTriggerPin))
                continue;
            uint32_t triggerPinMask = 1 << macro->macroTriggerPin;
            if ( allPins & triggerPinMask ) {
                return i;
            }
        }
    }

    return -1;
}

void InputMacro::runCurrentMacro() {

    uint32_t pressedMacro = checkMacroPress();
    uint64_t currentMicros = getMicro();

    // Do nothing if no macro was pressed, no active macro, or current macro is not
    if (pressedMacro == -1 && activeMacroIdx == -1) {
        return;
    }

    // Different macro behaviors
    if (pressedMacro != -1) {
        if ( inputMacroOptions.macroList[pressedMacro].macroType == ON_PRESS ) {
            // check that this button was not held
            activeMacroIdx = pressedMacro;

        } else if ( inputMacroOptions.macroList[pressedMacro].macroType == ON_HOLD_REPEAT ) {

        } else if ( inputMacroOptions.macroList[pressedMacro].macroType == ON_TOGGLE ) {
             
        }
    }

    if ( activeMacroIdx != lastMacroIdx ) {
        // New Macro Pressed!
        if ( inputMacroOptions.macroList[activeMacroIdx].macroType == ON_PRESS ) {

        }


        lastMacroIdx = activeMacroIdx;
    }
}

void InputMacro::preprocess()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint16_t buttonsPressed = gamepad->state.buttons;
    uint8_t dpadPressed = gamepad->state.dpad & GAMEPAD_MASK_DPAD;

    FocusModeOptions& focusModeOptions = Storage::getInstance().getAddonOptions().focusModeOptions;
    if (focusModeOptions.enabled && focusModeOptions.macroLockEnabled)
        return;

    runCurrentMacro();

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

    // Huh...
    if (!isMacroTriggerHeld && (!isMacroRunning || macro.interruptible)) {
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
