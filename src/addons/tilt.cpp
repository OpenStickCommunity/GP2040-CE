#include "addons/tilt.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

bool TiltInput::available() {
    const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
    return options.enabled && ((options.tilt1Pin != -1) || (options.tilt2Pin != -1));
}

void TiltInput::setup() {
	const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
	tiltSOCDMode = options.tiltSOCDMode;

	pinTilt1 = options.tilt1Pin;
	tilt1FactorLeftX = options.factorTilt1LeftX;
	tilt1FactorLeftY = options.factorTilt1LeftY;
	tilt1FactorRightX = options.factorTilt1RightX;
	tilt1FactorRightY = options.factorTilt1RightY;
	pinTilt2 = options.tilt2Pin;
	tilt2FactorLeftX = options.factorTilt2LeftX;
	tilt2FactorLeftY = options.factorTilt2LeftY;
	tilt2FactorRightX = options.factorTilt2RightX;
	tilt2FactorRightY = options.factorTilt2RightY;
	pinTiltLeftAnalogDown = options.tiltLeftAnalogDownPin;
	pinTiltLeftAnalogUp = options.tiltLeftAnalogUpPin;
	pinTiltLeftAnalogLeft = options.tiltLeftAnalogLeftPin;
	pinTiltLeftAnalogRight = options.tiltLeftAnalogRightPin;
	pinTiltRightAnalogDown = options.tiltRightAnalogDownPin;
	pinTiltRightAnalogUp = options.tiltRightAnalogUpPin;
	pinTiltRightAnalogLeft = options.tiltRightAnalogLeftPin;
	pinTiltRightAnalogRight = options.tiltRightAnalogRightPin;


	// Setup Tilt Key
	uint8_t pinTilt[10] = {
											pinTilt1,
											pinTilt2,
											pinTiltLeftAnalogDown,
											pinTiltLeftAnalogUp,
											pinTiltLeftAnalogLeft,
											pinTiltLeftAnalogRight,
											pinTiltRightAnalogDown,
											pinTiltRightAnalogUp,
											pinTiltRightAnalogLeft,
											pinTiltRightAnalogRight };

	for (int i = 0; i < 10; i++) {
		if (pinTilt[i] != (uint8_t)-1) {
			gpio_init(pinTilt[i]);             // Initialize pin
			gpio_set_dir(pinTilt[i], GPIO_IN); // Set as INPUT
			gpio_pull_up(pinTilt[i]);          // Set as PULLUP
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

	uint32_t now = getMillis();
	for (int i = 0; i < 4; i++) {
		dpadTime[i] = now;
	}
}

void TiltInput::preprocess()
{
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	// Need to invert since we're using pullups
	tiltLeftState = 0;
	if (pinTiltLeftAnalogUp != (uint8_t)-1) {
		tiltLeftState |= (!gpio_get(pinTiltLeftAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinTiltLeftAnalogDown != (uint8_t)-1) {
		tiltLeftState |= (!gpio_get(pinTiltLeftAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinTiltLeftAnalogLeft != (uint8_t)-1) {
		tiltLeftState |= (!gpio_get(pinTiltLeftAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinTiltLeftAnalogRight != (uint8_t)-1) {
		tiltLeftState |= (!gpio_get(pinTiltLeftAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	tiltRightState = 0;
	if (pinTiltRightAnalogUp != (uint8_t)-1) {
		tiltRightState |= (!gpio_get(pinTiltRightAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinTiltRightAnalogDown != (uint8_t)-1) {
		tiltRightState |= (!gpio_get(pinTiltRightAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinTiltRightAnalogLeft != (uint8_t)-1) {
		tiltRightState |= (!gpio_get(pinTiltRightAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinTiltRightAnalogRight != (uint8_t)-1) {
		tiltRightState |= (!gpio_get(pinTiltRightAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void TiltInput::process()
{
	SOCDTiltClean(tiltSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	// Set Tilt Output
	OverrideGamepad(gamepad, tiltLeftState, tiltRightState);
}

//The character's movement changes depending on the degree to which the stick is tilted.
//I added the functionality to allow the all - button controller to perform the operations that can be performed by the sticks.
//Two buttons, tilt1 and tilt2, have been implemented.
//While pressing these buttons, pressing the left or right analog stick will cause the character to walk or walk more slowly.
//Since this is an auxiliary function for appeals and such,
//pressing Tilt1 and Tilt2 at the same time will cause the light analog stick to correspond to each of the DPad methods.
void TiltInput::OverrideGamepad(Gamepad* gamepad, uint8_t dpad1, uint8_t dpad2) {
	bool pinTilt1Pressed = (pinTilt1 != (uint8_t)-1) ? !gpio_get(pinTilt1) : false;
	bool pinTilt2Pressed = (pinTilt2 != (uint8_t)-1) ? !gpio_get(pinTilt2) : false;

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

    if (pinTilt1Pressed && pinTilt2Pressed) {
        // inputs act as dpad
        gamepad->state.dpad |= dpad1|dpad2;
    } else {
        // analog input mode
        if (pinTilt1Pressed) {
            gamepad->state.lx = dpadToAnalogX(dpad1) + (midValue - dpadToAnalogX(dpad1)) * scaledTilt1FactorLeftX;
            gamepad->state.ly = dpadToAnalogY(dpad1) + (midValue - dpadToAnalogY(dpad1)) * scaledTilt1FactorLeftY;

            gamepad->state.rx = dpadToAnalogX(dpad2) + (midValue - dpadToAnalogX(dpad2)) * scaledTilt1FactorRightX;
            gamepad->state.ry = dpadToAnalogY(dpad2) + (midValue - dpadToAnalogY(dpad2)) * scaledTilt1FactorRightY;
        } else if (pinTilt2Pressed) {
            gamepad->state.lx = dpadToAnalogX(dpad1) + (midValue - dpadToAnalogX(dpad1)) * scaledTilt2FactorLeftX;
            gamepad->state.ly = dpadToAnalogY(dpad1) + (midValue - dpadToAnalogY(dpad1)) * scaledTilt2FactorLeftY;

            gamepad->state.rx = dpadToAnalogX(dpad2) + (midValue - dpadToAnalogX(dpad2)) * scaledTilt2FactorRightX;
            gamepad->state.ry = dpadToAnalogY(dpad2) + (midValue - dpadToAnalogY(dpad2)) * scaledTilt2FactorRightY;
        } else {
            gamepad->state.lx = dpadToAnalogX(dpad1);
            gamepad->state.ly = dpadToAnalogY(dpad1);

            gamepad->state.rx = dpadToAnalogX(dpad2);
            gamepad->state.ry = dpadToAnalogY(dpad2);
        }
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

