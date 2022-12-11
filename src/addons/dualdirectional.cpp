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
    dpadState = 0;

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
		if ((dDebState & dpadMasks[i]) != (dpadState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
    dpadState = dDebState;
}

void DualDirectionalInput::process()
{
    BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

 	// Need to invert since we're using pullups
	dpadState = (!gpio_get(boardOptions.pinDualDirUp) ? gamepad->mapDpadUp->buttonMask : 0)
		| (!gpio_get(boardOptions.pinDualDirDown) ? gamepad->mapDpadDown->buttonMask : 0)
		| (!gpio_get(boardOptions.pinDualDirLeft) ? gamepad->mapDpadLeft->buttonMask  : 0)
		| (!gpio_get(boardOptions.pinDualDirRight) ? gamepad->mapDpadRight->buttonMask : 0);

    // Debounce our directional pins
    debounce();

    // Dual Direction Mixed Combinational Logic
    uint8_t combinedGamepad = gamepad->state.dpad;
    uint8_t combinedDual = runSOCDCleaner(gamepad->options.socdMode, dpadState);
    if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_MIXED ) {
        // Combined SOCD(Gamepad) and SOCD(Dual) and SOCD clean it
        combinedGamepad = runSOCDCleaner(gamepad->options.socdMode, combinedGamepad | combinedDual);
        combinedDual = combinedGamepad;
    } else if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_GAMEPAD ) {
        if ( combinedGamepad != 0 && (combinedGamepad != combinedDual)) {
            combinedDual = combinedGamepad;
        }
    } else if ( boardOptions.dualDirCombineMode == DUAL_COMBINE_MODE_DUAL ) {
        if ( combinedDual != 0 && (combinedGamepad != combinedDual)) {
            combinedGamepad = combinedDual;
        }
    } 
    
    // Modify Gamepad if the state was changed
    if ( gamepad->state.dpad != combinedGamepad ) {
        switch (gamepad->options.dpadMode)
        {
            case DPAD_MODE_LEFT_ANALOG:
                gamepad->state.lx = dpadToAnalogX(combinedGamepad);
                gamepad->state.ly = dpadToAnalogY(combinedGamepad);
                break;
            case DPAD_MODE_RIGHT_ANALOG:
                gamepad->state.rx = dpadToAnalogX(combinedGamepad);
                gamepad->state.ry = dpadToAnalogY(combinedGamepad);
                break;
            case DPAD_MODE_DIGITAL:
                gamepad->state.dpad = combinedGamepad;
                break;
        }
    }

    // Modify the Dual Directional if the state is not correct
    switch (boardOptions.dualDirDpadMode)
    {
        case DPAD_MODE_LEFT_ANALOG:
            gamepad->state.lx = dpadToAnalogX(combinedDual);
            gamepad->state.ly = dpadToAnalogY(combinedDual);
            break;
        case DPAD_MODE_RIGHT_ANALOG:
            gamepad->state.rx = dpadToAnalogX(combinedDual);
            gamepad->state.ry = dpadToAnalogY(combinedDual);
            break;
        case DPAD_MODE_DIGITAL:
            gamepad->state.dpad = combinedDual;
            break;
    }
}
