#include "addons/tilt.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

bool TiltInput::available() {
    const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
    return options.enabled;
}

void TiltInput::setup() {
	const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
	tiltSOCDMode = options.tiltSOCDMode;

	mapAnalogLSXNeg = new GamepadButtonMapping(0);
	mapAnalogLSXPos = new GamepadButtonMapping(0);
	mapAnalogLSYNeg = new GamepadButtonMapping(0);
	mapAnalogLSYPos = new GamepadButtonMapping(0);
	mapAnalogRSXNeg = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapAnalogRSXPos = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapAnalogRSYNeg = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapAnalogRSYPos = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapAnalogModLow = new GamepadButtonMapping(ANALOG_DIRECTION_MOD_LOW);
	mapAnalogModHigh = new GamepadButtonMapping(ANALOG_DIRECTION_MOD_HIGH);

	EventManager::getInstance().registerEventHandler(GP_EVENT_PROFILE_CHANGE, GPEVENT_CALLBACK(this->handleProfileChange(event)));

	reloadMappings();

	uint32_t now = getMillis();
	for (int i = 0; i < 4; i++) {
		dpadTime[i] = now;
	}
}

void TiltInput::preprocess()
{
	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;

	// Need to invert since we're using pullups
	tiltLeftState = 0
		| ((values & mapAnalogLSXNeg->pinMask)    ? mapAnalogLSXNeg->buttonMask : 0)
		| ((values & mapAnalogLSXPos->pinMask)    ? mapAnalogLSXPos->buttonMask : 0)
		| ((values & mapAnalogLSYNeg->pinMask)    ? mapAnalogLSYNeg->buttonMask : 0)
		| ((values & mapAnalogLSYPos->pinMask)    ? mapAnalogLSYPos->buttonMask : 0);

	tiltRightState = 0
		| ((values & mapAnalogRSXNeg->pinMask)    ? mapAnalogRSXNeg->buttonMask : 0)
		| ((values & mapAnalogRSXPos->pinMask)    ? mapAnalogRSXPos->buttonMask : 0)
		| ((values & mapAnalogRSYNeg->pinMask)    ? mapAnalogRSYNeg->buttonMask : 0)
		| ((values & mapAnalogRSYPos->pinMask)    ? mapAnalogRSYPos->buttonMask : 0);

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void TiltInput::process()
{
	const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
	
	// don't process if no pins are bound. we can pause by disabling the addon, but pins are required.
	if (!options.enabled || ((mapAnalogModLow->pinMask == 0) && (mapAnalogModHigh->pinMask == 0))) return;

	SOCDTiltClean(tiltSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	// Set Tilt Output
	OverrideGamepad(gamepad, tiltLeftState, tiltRightState);
}

void TiltInput::reloadMappings() {
    const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;

	tilt1FactorLeftX = options.factorTilt1LeftX;
	tilt1FactorLeftY = options.factorTilt1LeftY;
	tilt1FactorRightX = options.factorTilt1RightX;
	tilt1FactorRightY = options.factorTilt1RightY;
	tilt2FactorLeftX = options.factorTilt2LeftX;
	tilt2FactorLeftY = options.factorTilt2LeftY;
	tilt2FactorRightX = options.factorTilt2RightX;
	tilt2FactorRightY = options.factorTilt2RightY;

	mapAnalogLSXNeg->pinMask  = 0;
	mapAnalogLSXPos->pinMask  = 0;
	mapAnalogLSYNeg->pinMask  = 0;
	mapAnalogLSYPos->pinMask  = 0;
	mapAnalogRSXNeg->pinMask  = 0;
	mapAnalogRSXPos->pinMask  = 0;
	mapAnalogRSYNeg->pinMask  = 0;
	mapAnalogRSYPos->pinMask  = 0;
	mapAnalogModLow->pinMask  = 0;
	mapAnalogModHigh->pinMask = 0;

	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		switch (pinMappings[pin].action) {
			case GpioAction::ANALOG_DIRECTION_LS_X_NEG:	mapAnalogLSXNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_X_POS:	mapAnalogLSXPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_NEG:	mapAnalogLSYNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_POS:	mapAnalogLSYPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_NEG:	mapAnalogRSXNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_POS:	mapAnalogRSXPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_NEG:	mapAnalogRSYNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_POS:	mapAnalogRSYPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_MOD_LOW:	mapAnalogModLow->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_MOD_HIGH:	mapAnalogModHigh->pinMask |= 1 << pin; break;
			default:	break;
		}
	}

	tiltLeftState = 0;
	tiltRightState = 0;

	lastGPUD = DIRECTION_NONE;
	lastGPLR = DIRECTION_NONE;

	leftLastTiltUD = DIRECTION_NONE;
	leftLastTiltLR = DIRECTION_NONE;

	rightLastTiltUD = DIRECTION_NONE;
	rightLastTiltLR = DIRECTION_NONE;
}

void TiltInput::OverrideGamepad(Gamepad* gamepad, uint8_t dpad1, uint8_t dpad2) {
	Mask_t values = gamepad->debouncedGpio;

	bool pinTilt1Pressed = (values & mapAnalogModLow->pinMask);
	bool pinTilt2Pressed = (values & mapAnalogModHigh->pinMask);

	// Scales input from 0-100% of maximum input
	double scaledTilt1FactorLeftX  = 1.0 - (tilt1FactorLeftX  / 100.0);
	double scaledTilt1FactorLeftY  = 1.0 - (tilt1FactorLeftY  / 100.0);
	double scaledTilt1FactorRightX = 1.0 - (tilt1FactorRightX / 100.0);
	double scaledTilt1FactorRightY = 1.0 - (tilt1FactorRightY / 100.0);
	double scaledTilt2FactorLeftX  = 1.0 - (tilt2FactorLeftX  / 100.0);
	double scaledTilt2FactorLeftY  = 1.0 - (tilt2FactorLeftY  / 100.0);
	double scaledTilt2FactorRightX = 1.0 - (tilt2FactorRightX / 100.0);
	double scaledTilt2FactorRightY = 1.0 - (tilt2FactorRightY / 100.0);

	uint16_t midValue = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		midValue = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

	uint16_t leftXValue = midValue;
	uint16_t leftYValue = midValue;
	uint16_t rightXValue = midValue;
	uint16_t rightYValue = midValue;

    if (pinTilt1Pressed && pinTilt2Pressed) {
        if (dpad2 > 0) {
            // right inputs act as dpad
            gamepad->state.dpad |= dpad2;
        } else {
            // prioritize tilt 1
            leftXValue = getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask))) * scaledTilt1FactorLeftX;
            leftYValue = getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask))) * scaledTilt1FactorLeftY;

            rightXValue = getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask))) * scaledTilt1FactorRightX;
            rightYValue = getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask))) * scaledTilt1FactorRightY;
        }
    } else {
        // analog input mode
        if (pinTilt1Pressed) {
            leftXValue = getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask))) * scaledTilt1FactorLeftX;
            leftYValue = getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask))) * scaledTilt1FactorLeftY;

            rightXValue = getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask))) * scaledTilt1FactorRightX;
            rightYValue = getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask))) * scaledTilt1FactorRightY;
        } else if (pinTilt2Pressed) {
            leftXValue = getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask))) * scaledTilt2FactorLeftX;
            leftYValue = getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask))) * scaledTilt2FactorLeftY;

            rightXValue = getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask))) * scaledTilt2FactorRightX;
            rightYValue = getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask)) + (midValue - getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask))) * scaledTilt2FactorRightY;
        } else {
            leftXValue = getAnalogValue((values & mapAnalogLSXNeg->pinMask), (values & mapAnalogLSXPos->pinMask));
            leftYValue = getAnalogValue((values & mapAnalogLSYNeg->pinMask), (values & mapAnalogLSYPos->pinMask));

            rightXValue = getAnalogValue((values & mapAnalogRSXNeg->pinMask), (values & mapAnalogRSXPos->pinMask));
            rightYValue = getAnalogValue((values & mapAnalogRSYNeg->pinMask), (values & mapAnalogRSYPos->pinMask));
        }
    }

    gamepad->state.lx = leftXValue;
    gamepad->state.ly = leftYValue;

    gamepad->state.rx = rightXValue;
    gamepad->state.ry = rightYValue;
}

uint16_t TiltInput::getAnalogValue(bool isMin, bool isMax) {
	uint16_t midValue = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		midValue = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

	if (isMin && !isMax) {
		return GAMEPAD_JOYSTICK_MIN;
	} else if (!isMin && isMax) {
		return GAMEPAD_JOYSTICK_MAX;
	} else {
		return midValue;
	}
}

void TiltInput::SOCDTiltClean(SOCDMode socdMode) {
    // Left Stick SOCD Cleaning
	// Tilt SOCD Last-Win Clean
	switch (tiltLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			tiltLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
			leftLastTiltUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && leftLastTiltUD != DIRECTION_NONE) {
			tiltLeftState ^= (leftLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			tiltLeftState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			leftLastTiltUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		leftLastTiltUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		leftLastTiltUD = DIRECTION_DOWN;
		break;
	default:
		leftLastTiltUD = DIRECTION_NONE;
		break;
	}
	switch (tiltLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			tiltLeftState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			leftLastTiltLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (leftLastTiltLR != DIRECTION_NONE)
				tiltLeftState ^= (leftLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				leftLastTiltLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		leftLastTiltLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		leftLastTiltLR = DIRECTION_RIGHT;
		break;
	default:
		leftLastTiltLR = DIRECTION_NONE;
		break;
	}
	// Right Stick SOCD Cleaning
	// Tilt SOCD Last-Win Clean
	switch (tiltRightState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			tiltRightState ^= GAMEPAD_MASK_DOWN; // Remove Down
			rightLastTiltUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && rightLastTiltUD != DIRECTION_NONE) {
			tiltRightState ^= (rightLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			tiltRightState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			rightLastTiltUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		rightLastTiltUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		rightLastTiltUD = DIRECTION_DOWN;
		break;
	default:
		rightLastTiltUD = DIRECTION_NONE;
		break;
	}
    
	switch (tiltRightState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			tiltRightState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			rightLastTiltLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (rightLastTiltLR != DIRECTION_NONE)
				tiltRightState ^= (rightLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				rightLastTiltLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		rightLastTiltLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		rightLastTiltLR = DIRECTION_RIGHT;
		break;
	default:
		rightLastTiltLR = DIRECTION_NONE;
		break;
	}
}

void TiltInput::handleProfileChange(GPEvent* e) {
	reloadMappings();
}
