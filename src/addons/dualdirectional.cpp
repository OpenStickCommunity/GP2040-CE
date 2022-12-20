#include "addons/dualdirectional.h"
#include "storagemanager.h"

#define DUAL_DEBOUNCE_MILLIS 5

bool DualDirectionalInput::available() {
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    return (boardOptions.pinDualDirDown != (uint8_t)-1 &&
        boardOptions.pinDualDirUp != (uint8_t)-1 &&
        boardOptions.pinDualDirLeft != (uint8_t)-1 &&
        boardOptions.pinDualDirRight != (uint8_t)-1);
}

void DualDirectionalInput::setup() {
    // Setup TURBO Key
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    uint8_t pinDualDir[4] = {boardOptions.pinDualDirDown,
                        boardOptions.pinDualDirUp,
                        boardOptions.pinDualDirLeft,
                        boardOptions.pinDualDirRight};

    for (int i = 0; i < 4; i++) {
        gpio_init(pinDualDir[i]);             // Initialize pin
        gpio_set_dir(pinDualDir[i], GPIO_IN); // Set as INPUT
        gpio_pull_up(pinDualDir[i]);          // Set as PULLUP
    }

    dDebState = 0;
    dualState = 0;

    uint32_t now = getMillis();
    for(int i = 0; i < 4; i++) {
        dpadTime[i] = now;
    }
}

void DualDirectionalInput::debounce()
{
	uint32_t now = getMillis();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

	for (int i = 0; i < 4; i++)
	{
		if ((dDebState & dpadMasks[i]) != (dualState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
    dualState = dDebState;
}

void DualDirectionalInput::preprocess()
{
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

 	// Need to invert since we're using pullups
	dualState = (!gpio_get(boardOptions.pinDualDirUp) ? gamepad->mapDpadUp->buttonMask : 0)
		| (!gpio_get(boardOptions.pinDualDirDown) ? gamepad->mapDpadDown->buttonMask : 0)
		| (!gpio_get(boardOptions.pinDualDirLeft) ? gamepad->mapDpadLeft->buttonMask  : 0)
		| (!gpio_get(boardOptions.pinDualDirRight) ? gamepad->mapDpadRight->buttonMask : 0);

    // Debounce our directional pins
    debounce();

    // Convert gamepad from process() output to uint8 value
    uint8_t gamepadState = gamepad->state.dpad;

    // Combined Mode
    if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_MIXED ) {
        gamepadState = gamepadState | dualState;
    }
    // Gamepad Overwrite Mode
    else if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_GAMEPAD ) {
        if ( gamepadState != 0 && (gamepadState != dualState)) {
            dualState = gamepadState;
        }
    }
    // Dual Overwrite Mode
    else if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_DUAL ) {
        if ( dualState != 0 && gamepadState != dualState) {
            gamepadState = dualState;
        }
    }
    
    gamepad->state.dpad = gamepadState;
}

void DualDirectionalInput::process()
{
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    uint8_t dualOut = 0;

    // If we're in mixed mode
    if (boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_MIXED) {
        switch(gamepad->options.dpadMode) { // Convert gamepad to dual if we're in mixed
            case DPAD_MODE_DIGITAL:
                dualOut = gamepad->state.dpad;
                break;
            case DPAD_MODE_LEFT_ANALOG:
                if ( gamepad->state.lx == GAMEPAD_JOYSTICK_MIN) {
                    dualOut = dualOut | GAMEPAD_MASK_LEFT;
                } else if ( gamepad->state.lx == GAMEPAD_JOYSTICK_MAX) {
                    dualOut = dualOut | GAMEPAD_MASK_RIGHT;
                }
                if ( gamepad->state.ly == GAMEPAD_JOYSTICK_MIN) {
                    dualOut = dualOut | GAMEPAD_MASK_UP;
                } else if ( gamepad->state.ly == GAMEPAD_JOYSTICK_MAX) {
                    dualOut = dualOut | GAMEPAD_MASK_DOWN;
                }
                break;
            case DPAD_MODE_RIGHT_ANALOG:
                if ( gamepad->state.rx == GAMEPAD_JOYSTICK_MIN) {
                    dualOut = dualOut | GAMEPAD_MASK_LEFT;
                } else if ( gamepad->state.rx == GAMEPAD_JOYSTICK_MAX) {
                    dualOut = dualOut | GAMEPAD_MASK_RIGHT;
                }
                if ( gamepad->state.ry == GAMEPAD_JOYSTICK_MIN) {
                    dualOut = dualOut | GAMEPAD_MASK_UP;
                } else if ( gamepad->state.ry == GAMEPAD_JOYSTICK_MAX) {
                    dualOut = dualOut | GAMEPAD_MASK_DOWN;
                }
                break;
        }
    } else {
        dualOut = dualState;
    }

    // Modify the Dual Directional if the state is not correct
    switch (boardOptions.dualDirDpadMode)
    {
        case DPAD_MODE_LEFT_ANALOG:
            gamepad->state.lx = dpadToAnalogX(dualOut);
            gamepad->state.ly = dpadToAnalogY(dualOut);
            break;
        case DPAD_MODE_RIGHT_ANALOG:
            gamepad->state.rx = dpadToAnalogX(dualOut);
            gamepad->state.ry = dpadToAnalogY(dualOut);
            break;
        case DPAD_MODE_DIGITAL:
            gamepad->state.dpad = dualOut;
            break;
    }
}
