#include "addons/snes_input.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"

bool SNESpadInput::available() {
    const SNESOptions& snesOptions = Storage::getInstance().getAddonOptions().snesOptions;

    return (snesOptions.enabled &&
        isValidPin(snesOptions.clockPin) &&
        isValidPin(snesOptions.latchPin) &&
        isValidPin(snesOptions.dataPin));
}

void SNESpadInput::setup() {
    const SNESOptions& snesOptions = Storage::getInstance().getAddonOptions().snesOptions;
    nextTimer = getMillis();

#if SNES_PAD_DEBUG==true
    stdio_init_all();
#endif

    uIntervalMS = 0;

    snes = new SNESpad(
        snesOptions.clockPin,
        snesOptions.latchPin,
        snesOptions.dataPin);
    snes->begin();
    snes->start();

    // Run during setup to catch boot selection mode
    snes->poll();

    if (snes->type == SNES_PAD_BASIC) {
        buttonA = snes->buttonA;
        buttonB = snes->buttonB;
        buttonX = snes->buttonX;
        buttonY = snes->buttonY;
        buttonL = snes->buttonL;
        buttonR = snes->buttonR;
        dpadUp = snes->directionUp;
        dpadDown = snes->directionDown;
        dpadLeft = snes->directionLeft;
        dpadRight = snes->directionRight;
        buttonSelect = snes->buttonSelect;
        buttonStart = snes->buttonStart;

    } else if (snes->type == SNES_PAD_NES) {
        buttonA = snes->buttonA;
        buttonB = snes->buttonB;
        buttonX = false;
        buttonY = false;
        buttonL = false;
        buttonR = false;
        dpadUp = snes->directionUp;
        dpadDown = snes->directionDown;
        dpadLeft = snes->directionLeft;
        dpadRight = snes->directionRight;
        buttonSelect = snes->buttonSelect;
        buttonStart = snes->buttonStart;

    } else if (snes->type == SNES_PAD_MOUSE){
        buttonA = snes->buttonA;
        buttonB = snes->buttonB;
        buttonX = false;
        buttonY = false;
        buttonL = false;
        buttonR = false;
        dpadUp = false;
        dpadDown = false;
        dpadLeft = false;
        dpadRight = false;
        buttonSelect = false;
        buttonStart = false;

        leftX = map(snes->mouseX,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        leftY = map(snes->mouseY,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    }
}

void SNESpadInput::process() {
    if (nextTimer < getMillis()) {
        snes->poll();

        uint16_t joystickMid = GetJoystickMidValue(Storage::getInstance().getGamepadOptions().inputMode);

        leftX = joystickMid;
        leftY = joystickMid;
        rightX = joystickMid;
        rightY = joystickMid;

        if (snes->type == SNES_PAD_BASIC) {
            buttonA = snes->buttonA;
            buttonB = snes->buttonB;
            buttonX = snes->buttonX;
            buttonY = snes->buttonY;
            buttonL = snes->buttonL;
            buttonR = snes->buttonR;
            dpadUp = snes->directionUp;
            dpadDown = snes->directionDown;
            dpadLeft = snes->directionLeft;
            dpadRight = snes->directionRight;
            buttonSelect = snes->buttonSelect;
            buttonStart = snes->buttonStart;

        } else if (snes->type == SNES_PAD_NES) {
            buttonA = snes->buttonA;
            buttonB = snes->buttonB;
            buttonX = false;
            buttonY = false;
            buttonL = false;
            buttonR = false;
            dpadUp = snes->directionUp;
            dpadDown = snes->directionDown;
            dpadLeft = snes->directionLeft;
            dpadRight = snes->directionRight;
            buttonSelect = snes->buttonSelect;
            buttonStart = snes->buttonStart;

        } else if (snes->type == SNES_PAD_MOUSE){
            buttonA = snes->buttonA;
            buttonB = snes->buttonB;
            buttonX = false;
            buttonY = false;
            buttonL = false;
            buttonR = false;
            dpadUp = false;
            dpadDown = false;
            dpadLeft = false;
            dpadRight = false;
            buttonSelect = false;
            buttonStart = false;

            leftX = map(snes->mouseX,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(snes->mouseY,0,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);

        }

        nextTimer = getMillis() + uIntervalMS;
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.lx = leftX;
    gamepad->state.ly = leftY;
    gamepad->state.rx = rightX;
    gamepad->state.ry = rightY;

    gamepad->hasAnalogTriggers = false;
    gamepad->hasLeftAnalogStick = true;

    if (buttonA) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (buttonB) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (buttonX) gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (buttonY) gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (buttonL) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (buttonR) gamepad->state.buttons |= GAMEPAD_MASK_R1;
    if (buttonSelect) gamepad->state.buttons |= GAMEPAD_MASK_S1;
    if (buttonStart) gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (dpadUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t SNESpadInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
