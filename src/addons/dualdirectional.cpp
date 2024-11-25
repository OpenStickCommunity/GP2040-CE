#include "addons/dualdirectional.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "types.h"

bool DualDirectionalInput::available() {
    return Storage::getInstance().getAddonOptions().dualDirectionalOptions.enabled;
}

void DualDirectionalInput::setup() {
    const DualDirectionalOptions& options = Storage::getInstance().getAddonOptions().dualDirectionalOptions;

    mapDpadUp    = new GamepadButtonMapping(GAMEPAD_MASK_UP);
    mapDpadDown  = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
    mapDpadLeft  = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
    mapDpadRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch (pinMappings[pin].action) {
            case GpioAction::BUTTON_PRESS_DDI_UP:    mapDpadUp->pinMask |= 1 << pin; break;
            case GpioAction::BUTTON_PRESS_DDI_DOWN:  mapDpadDown->pinMask |= 1 << pin; break;
            case GpioAction::BUTTON_PRESS_DDI_LEFT:  mapDpadLeft->pinMask |= 1 << pin; break;
            case GpioAction::BUTTON_PRESS_DDI_RIGHT: mapDpadRight->pinMask |= 1 << pin; break;
            default:                                 break;
        }
    }

    dualState = 0;

    lastGPUD = DIRECTION_NONE;
    lastGPLR = DIRECTION_NONE;

    lastDualUD = DIRECTION_NONE;
    lastDualLR = DIRECTION_NONE;
}

/**
 * Reinitialize masks and DDI states.
 */
void DualDirectionalInput::reinit()
{
    delete mapDpadUp;
    delete mapDpadDown;
    delete mapDpadLeft;
    delete mapDpadRight;
    this->setup();
}


uint8_t DualDirectionalInput::updateDpadDDI(uint8_t dpad, DpadDirection direction)
{
	static bool inList[] = {false, false, false, false, false}; // correspond to DpadDirection: none, up, down, left, right
	static list<DpadDirection> dpadList;

	if(dpad & getMaskFromDirection(direction))
	{
		if(!inList[direction])
		{
			dpadList.push_back(direction);
			inList[direction] = true;
		}
	}
	else
	{
		if(inList[direction])
		{
			dpadList.remove(direction);
			inList[direction] = false;
		}
	}

	if(dpadList.empty()) {
		return 0;
	}
	else {
		return getMaskFromDirection(dpadList.back());
	}
}

/**
 * @brief Filter diagonals out of the dpad, making the device work as a 4-way lever.
 *
 * The most recent cardinal direction wins.
 *
 * @param dpad The GameState.dpad value.
 * @return uint8_t The new dpad value.
 */
uint8_t DualDirectionalInput::filterToFourWayModeDDI(uint8_t dpad)
{
	updateDpadDDI(dpad, DIRECTION_UP);
	updateDpadDDI(dpad, DIRECTION_DOWN);
	updateDpadDDI(dpad, DIRECTION_LEFT);
	return updateDpadDDI(dpad, DIRECTION_RIGHT);
}


void DualDirectionalInput::preprocess()
{
    const DualDirectionalOptions& options = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t values = gamepad->debouncedGpio;

    dualState = 0
            | ((values & mapDpadUp->pinMask)    ? mapDpadUp->buttonMask : 0)
            | ((values & mapDpadDown->pinMask)  ? mapDpadDown->buttonMask : 0)
            | ((values & mapDpadLeft->pinMask)  ? mapDpadLeft->buttonMask : 0)
            | ((values & mapDpadRight->pinMask) ? mapDpadRight->buttonMask : 0);

    const SOCDMode socdMode = getSOCDMode(gamepad->getOptions());

    // 4-way before SOCD, might have better history without losing any coherent functionality
    if (options.fourWayMode) {
        dualState = filterToFourWayModeDDI(dualState);
    }

    // SOCD clean the dual inputs based on the mode in the gamepad config
    SOCDDualClean(socdMode);
}

void DualDirectionalInput::process()
{
    const DualDirectionalOptions& options = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    uint8_t dualOut = dualState;
    const SOCDMode socdMode = getSOCDMode(gamepad->getOptions());
    uint8_t gamepadDpad = gpadToBinary(gamepad->getActiveDpadMode(), gamepad->state);

    // in mixed mode, we need to combine/re-clean the gamepad and DDI outputs to create a coherent behavior
    // reminder that combination mode none with the DDI output set to the same thing as the gamepad
    // output is, in practice, the same behavior as mixed mode, so it also is addressed here
    if (options.combineMode == DualDirectionalCombinationMode::MIXED_MODE ||
            (options.combineMode == DualDirectionalCombinationMode::NONE_MODE &&
             gamepad->getActiveDpadMode() == options.dpadMode)) {
        if ( socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL ) {
            // neutral/up priority SOCD cleaning are pretty simple, they just need to be re-neutralized
            dualOut = SOCDCombine(socdMode, gamepadDpad);
        } else if ( socdMode != SOCD_MODE_BYPASS ) {
            // else if not bypass, what's left is first/last input wins SOCD, which need a complicated re-clean
            dualOut = SOCDGamepadClean(dualOut | gamepadDpad, socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY);
        } else {
            // this is bypass SOCD, just OR them together
            dualOut |= gamepadDpad;
        }
        OverrideGamepad(gamepad, gamepad->getActiveDpadMode(), dualOut);
    } else if (options.combineMode != DualDirectionalCombinationMode::NONE_MODE) {
        // this is either of the override modes, which we will treat the same way --- they replace
        // the gamepad entirely in certain conditions: DDI Override if it has any data,
        // Gamepad Override if gamepad doesn't have any data
        if ((options.combineMode == DualDirectionalCombinationMode::DUAL_MODE && dualOut != 0) ||
                (options.combineMode == DualDirectionalCombinationMode::GAMEPAD_MODE && gamepadDpad == 0)) {
            OverrideGamepad(gamepad, gamepad->getActiveDpadMode(), dualOut);
        }
    } else {
        // the DDI and gamepad outputs don't need to be mixed, so just apply DDI output to the gamepad
        OverrideGamepad(gamepad, options.dpadMode, dualOut);
    }
}

void DualDirectionalInput::OverrideGamepad(Gamepad * gamepad, DpadMode mode, uint8_t dpad) {
    switch (mode) {
        case DPAD_MODE_LEFT_ANALOG:
            gamepad->state.lx = dpadToAnalogX(dpad);
            gamepad->state.ly = dpadToAnalogY(dpad);
            break;
        case DPAD_MODE_RIGHT_ANALOG:
            gamepad->state.rx = dpadToAnalogX(dpad);
            gamepad->state.ry = dpadToAnalogY(dpad);
            break;
        case DPAD_MODE_DIGITAL:
            gamepad->state.dpad = dpad;
            break;
    }
}

uint8_t DualDirectionalInput::SOCDGamepadClean(uint8_t gamepadState, bool isLastWin) {
    // Gamepad SOCD Last-Win OR First-Win Clean
    switch (gamepadState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
        case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
            if (isLastWin) gamepadState ^= (lastGPUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
            else gamepadState ^= (lastGPUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
            break;
        case GAMEPAD_MASK_UP:
            gamepadState |= GAMEPAD_MASK_UP;
            lastGPUD = DIRECTION_UP;
            break;
        case GAMEPAD_MASK_DOWN:
            gamepadState |= GAMEPAD_MASK_DOWN;
            lastGPUD = DIRECTION_DOWN;
            break;
        default:
            lastGPUD = DIRECTION_NONE;
            break;
    }
    switch (gamepadState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
        case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
            if (lastGPLR != DIRECTION_NONE)
                if (isLastWin) gamepadState ^= (lastGPLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
                else gamepadState ^= (lastGPLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
            else
                lastGPLR = DIRECTION_NONE;
            break;
        case GAMEPAD_MASK_LEFT:
            gamepadState |= GAMEPAD_MASK_LEFT;
            lastGPLR = DIRECTION_LEFT;
            break;
        case GAMEPAD_MASK_RIGHT:
            gamepadState |= GAMEPAD_MASK_RIGHT;
            lastGPLR = DIRECTION_RIGHT;
            break;
        default:
            lastGPLR = DIRECTION_NONE;
            break;
    }
    return gamepadState;
}


uint8_t DualDirectionalInput::SOCDCombine(SOCDMode mode, uint8_t gamepadState) {
    uint8_t outState = dualState | gamepadState;

    if (mode == SOCD_MODE_BYPASS) {
        return outState;
    }

    switch (outState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
        case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
            if ( mode == SOCD_MODE_NEUTRAL )
                outState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN);
            else if ( mode == SOCD_MODE_UP_PRIORITY )
                outState = GAMEPAD_MASK_UP;
            break;
        default:
            break;
    }
    switch (outState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
        case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
            outState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT);
            break;
        default:
            break;
    }

    return outState;
}

void DualDirectionalInput::SOCDDualClean(SOCDMode socdMode) { 
    if (socdMode == SOCD_MODE_BYPASS) {
        return;
    }

    // Dual SOCD Last-Win Clean
    switch (dualState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
        case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
            if ( socdMode == SOCD_MODE_UP_PRIORITY ) {
                dualState ^= GAMEPAD_MASK_DOWN; // Remove Down
                lastDualUD = DIRECTION_UP; // We're in UP mode
            } else if ( socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastDualUD != DIRECTION_NONE ) {
                dualState ^= (lastDualUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
            } else if ( socdMode == SOCD_MODE_FIRST_INPUT_PRIORITY && lastDualUD != DIRECTION_NONE ) {
                dualState ^= (lastDualUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
            } else {
                dualState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
                lastDualUD = DIRECTION_NONE;
            }
            break;
        case GAMEPAD_MASK_UP:
            lastDualUD = DIRECTION_UP;
            break;
        case GAMEPAD_MASK_DOWN:
            lastDualUD = DIRECTION_DOWN;
            break;
        default:
            lastDualUD = DIRECTION_NONE;
            break;
    }
    switch (dualState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
        case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
            if ( socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL ) {
                dualState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
                lastDualLR = DIRECTION_NONE;
            } else if ( socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY || socdMode == SOCD_MODE_FIRST_INPUT_PRIORITY ) {
                if (lastDualLR != DIRECTION_NONE)
                    if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) dualState ^= (lastDualLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
                    else dualState ^= (lastDualLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT; // First Win
                else
                    lastDualLR = DIRECTION_NONE;
            }
            break;
        case GAMEPAD_MASK_LEFT:
            lastDualLR = DIRECTION_LEFT;
            break;
        case GAMEPAD_MASK_RIGHT:
            lastDualLR = DIRECTION_RIGHT;
            break;
        default:
            lastDualLR = DIRECTION_NONE;
            break;
    }
}

uint8_t DualDirectionalInput::gpadToBinary(DpadMode dpadMode, GamepadState state) {
    uint8_t out = 0;
    switch(dpadMode) { // Convert gamepad to dual if we're in mixed
        case DPAD_MODE_DIGITAL:
            out = state.dpad;
            break;
        case DPAD_MODE_LEFT_ANALOG:
            if ( state.lx == GAMEPAD_JOYSTICK_MIN) {
                out = out | GAMEPAD_MASK_LEFT;
            } else if ( state.lx == GAMEPAD_JOYSTICK_MAX) {
                out = out | GAMEPAD_MASK_RIGHT;
            }
            if ( state.ly == GAMEPAD_JOYSTICK_MIN) {
                out = out | GAMEPAD_MASK_UP;
            } else if ( state.ly == GAMEPAD_JOYSTICK_MAX) {
                out = out | GAMEPAD_MASK_DOWN;
            }
            break;
        case DPAD_MODE_RIGHT_ANALOG:
            if ( state.rx == GAMEPAD_JOYSTICK_MIN) {
                out = out | GAMEPAD_MASK_LEFT;
            } else if ( state.rx == GAMEPAD_JOYSTICK_MAX) {
                out = out | GAMEPAD_MASK_RIGHT;
            }
            if ( state.ry == GAMEPAD_JOYSTICK_MIN) {
                out = out | GAMEPAD_MASK_UP;
            } else if ( state.ry == GAMEPAD_JOYSTICK_MAX) {
                out = out | GAMEPAD_MASK_DOWN;
            }
            break;
    }

    return out;
}

const SOCDMode DualDirectionalInput::getSOCDMode(const GamepadOptions& options) {
    return Gamepad::resolveSOCDMode(options);
}
