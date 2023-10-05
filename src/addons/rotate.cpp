#include "addons/rotate.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "math.h"

bool RotateInput::available() {
	return Storage::getInstance().getAddonOptions().rotateOptions.enabled;
}

void RotateInput::setup() {
	const RotateOptions& options = Storage::getInstance().getAddonOptions().rotateOptions;	
	rotateSOCDMode = options.rotateSOCDMode;

	pinRotate1 = options.rotate1Pin;
	rotate1FactorLeftX = options.factorRotate1LeftX;
	rotate1FactorLeftY = options.factorRotate1LeftY;
	rotate1FactorRightX = options.factorRotate1RightX;
	rotate1FactorRightY = options.factorRotate1RightY;
	pinRotate2 = options.rotate2Pin;
	rotate2FactorLeftX = options.factorRotate2LeftX;
	rotate2FactorLeftY = options.factorRotate2LeftY;
	rotate2FactorRightX = options.factorRotate2RightX;
	rotate2FactorRightY = options.factorRotate2RightY;
	pinRotateLeftAnalogDown = options.rotateLeftAnalogDownPin;
	pinRotateLeftAnalogUp = options.rotateLeftAnalogUpPin;
	pinRotateLeftAnalogLeft = options.rotateLeftAnalogLeftPin;
	pinRotateLeftAnalogRight = options.rotateLeftAnalogRightPin;
	pinRotateRightAnalogDown = options.rotateRightAnalogDownPin;
	pinRotateRightAnalogUp = options.rotateRightAnalogUpPin;
	pinRotateRightAnalogLeft = options.rotateRightAnalogLeftPin;
	pinRotateRightAnalogRight = options.rotateRightAnalogRightPin;


	// Setup Rotate Key
	uint8_t pinRotate[10] = {
											pinRotate1,
											pinRotate2,
											pinRotateLeftAnalogDown,
											pinRotateLeftAnalogUp,
											pinRotateLeftAnalogLeft,
											pinRotateLeftAnalogRight,
											pinRotateRightAnalogDown,
											pinRotateRightAnalogUp,
											pinRotateRightAnalogLeft,
											pinRotateRightAnalogRight };

	for (int i = 0; i < 10; i++) {
		if (pinRotate[i] != (uint8_t)-1) {
			gpio_init(pinRotate[i]);             // Initialize pin
			gpio_set_dir(pinRotate[i], GPIO_IN); // Set as INPUT
			gpio_pull_up(pinRotate[i]);          // Set as PULLUP
		}
	}

	dDebLeftState = 0;
	dDebRightState = 0;
	rotateLeftState = 0;
	rotateRightState = 0;

	lastGPUD = DIRECTION_NONE;
	lastGPLR = DIRECTION_NONE;

	lastRotateUD = DIRECTION_NONE;
	lastRotateLR = DIRECTION_NONE;

	uint32_t now = getMillis();
	for (int i = 0; i < 4; i++) {
		dpadTime[i] = now;
	}
}

void RotateInput::debounce()
{
	uint32_t now = getMillis();
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	for (int i = 0; i < 4; i++)
	{
		if ((dDebLeftState & dpadMasks[i]) != (rotateLeftState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebLeftState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	rotateLeftState = dDebLeftState;

	for (int i = 0; i < 4; i++)
	{
		if ((dDebRightState & dpadMasks[i]) != (rotateRightState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebRightState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	rotateRightState = dDebRightState;
}

void RotateInput::preprocess()
{
	bool sliderLeft;
	bool sliderRight;
	
	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	sliderLeft = gamepad->getOptions().dpadMode == DPAD_MODE_LEFT_ANALOG;
	sliderRight = gamepad->getOptions().dpadMode == DPAD_MODE_RIGHT_ANALOG;

	// Need to invert since we're using pullups
	rotateLeftState = 0;
	if ((pinRotateLeftAnalogUp == (uint8_t)-1) && (pinRotateLeftAnalogDown == (uint8_t)-1) && (pinRotateLeftAnalogLeft == (uint8_t)-1) && (pinRotateLeftAnalogRight == (uint8_t)-1) && sliderLeft) {
		rotateLeftState |= gamepad->state.dpad;
	}
	if (pinRotateLeftAnalogUp != (uint8_t)-1) {
		rotateLeftState |= (!gpio_get(pinRotateLeftAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinRotateLeftAnalogDown != (uint8_t)-1) {
		rotateLeftState |= (!gpio_get(pinRotateLeftAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinRotateLeftAnalogLeft != (uint8_t)-1) {
		rotateLeftState |= (!gpio_get(pinRotateLeftAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinRotateLeftAnalogRight != (uint8_t)-1) {
		rotateLeftState |= (!gpio_get(pinRotateLeftAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	rotateRightState = 0;
	if ((pinRotateRightAnalogUp == (uint8_t)-1) && (pinRotateRightAnalogDown == (uint8_t)-1) && (pinRotateRightAnalogLeft == (uint8_t)-1) && (pinRotateRightAnalogRight == (uint8_t)-1) && sliderRight) {
		rotateRightState |= gamepad->state.dpad;
	}
	if (pinRotateRightAnalogUp != (uint8_t)-1) {
		rotateRightState |= (!gpio_get(pinRotateRightAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinRotateRightAnalogDown != (uint8_t)-1) {
		rotateRightState |= (!gpio_get(pinRotateRightAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinRotateRightAnalogLeft != (uint8_t)-1) {
		rotateRightState |= (!gpio_get(pinRotateRightAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinRotateRightAnalogRight != (uint8_t)-1) {
		rotateRightState |= (!gpio_get(pinRotateRightAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	// Debounce our directional pins
	debounce();

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void RotateInput::process()
{
	SOCDRotateClean(rotateSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	uint8_t rotateLeftOut = rotateLeftState;
	uint8_t rotateRightOut = rotateRightState;

	// Set Rotate Output
	OverrideGamepad(gamepad, rotateLeftOut, rotateRightOut);
}

//The character's movement changes depending on the degree to which the stick is rotateed.
//I added the functionality to allow the all - button controller to perform the operations that can be performed by the sticks.
//Two buttons, rotate1 and rotate2, have been implemented.
//While pressing these buttons, pressing the left or right analog stick will cause the character to walk or walk more slowly.
//Since this is an auxiliary function for appeals and such,
//pressing Rotate1 and Rotate2 at the same time will cause the light analog stick to correspond to each of the DPad methods.
void RotateInput::OverrideGamepad(Gamepad* gamepad, uint8_t dpad1, uint8_t dpad2) {
	bool pinRotate1Pressed = (pinRotate1 != (uint8_t)-1) ? !gpio_get(pinRotate1) : false;
	bool pinRotate2Pressed = (pinRotate2 != (uint8_t)-1) ? !gpio_get(pinRotate2) : false;

	double scaledRotate1FactorLeftX  = rotate1FactorLeftX  / 100.0;
	double scaledRotate1FactorLeftY  = rotate1FactorLeftY  / 100.0;
	double scaledRotate1FactorRightX = rotate1FactorRightX / 100.0;
	double scaledRotate1FactorRightY = rotate1FactorRightY / 100.0;
	double scaledRotate2FactorLeftX  = rotate2FactorLeftX  / 100.0;
	double scaledRotate2FactorLeftY  = rotate2FactorLeftY  / 100.0;
	double scaledRotate2FactorRightX = rotate2FactorRightX / 100.0;
	double scaledRotate2FactorRightY = rotate2FactorRightY / 100.0;
	
	uint16_t stickradius = GAMEPAD_JOYSTICK_MAX - GAMEPAD_JOYSTICK_MID;
	double rotate1degreeLeft = rotate1FactorLeftX;
	double rotate1degreeRight = rotate1FactorRightX;
	double rotate2degreeLeft = rotate2FactorLeftX;
	double rotate2degreeRight = rotate2FactorRightX;
	
	double rotate1LeftSIN = sin(rotate1degreeLeft / 180 * M_PI) * stickradius;
	double rotate1LeftCOS = cos(rotate1degreeLeft / 180 * M_PI) * stickradius;
	double rotate1LeftDiagSIN = sin(rotate1degreeLeft / 180 * M_PI + M_PI/4) * stickradius;
	double rotate1LeftDiagCOS = cos(rotate1degreeLeft / 180 * M_PI + M_PI/4) * stickradius;
	double rotate1RightSIN = sin(rotate1degreeRight / 180 * M_PI) * stickradius;
	double rotate1RightCOS = cos(rotate1degreeRight / 180 * M_PI) * stickradius;
	double rotate1RightDiagSIN = sin(rotate1degreeRight / 180 * M_PI + M_PI/4) * stickradius;
	double rotate1RightDiagCOS = cos(rotate1degreeRight / 180 * M_PI + M_PI/4) * stickradius;
	double rotate2LeftSIN = sin(rotate2degreeLeft / 180 * M_PI) * stickradius;
	double rotate2LeftCOS = cos(rotate2degreeLeft / 180 * M_PI) * stickradius;
	double rotate2LeftDiagSIN = sin(rotate2degreeLeft / 180 * M_PI - M_PI/4) * stickradius;
	double rotate2LeftDiagCOS = cos(rotate2degreeLeft / 180 * M_PI - M_PI/4) * stickradius;
	double rotate2RightSIN = sin(rotate2degreeRight / 180 * M_PI) * stickradius;
	double rotate2RightCOS = cos(rotate2degreeRight / 180 * M_PI) * stickradius;
	double rotate2RightDiagSIN = sin(rotate2degreeRight / 180 * M_PI - M_PI/4) * stickradius;
	double rotate2RightDiagCOS = cos(rotate2degreeRight / 180 * M_PI - M_PI/4) * stickradius;

    if (pinRotate1Pressed) {
		switch (rotateLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate1LeftSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate1LeftCOS;
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate1LeftCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate1LeftSIN;
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate1LeftSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate1LeftCOS;
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate1LeftCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate1LeftSIN;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate1LeftDiagSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate1LeftDiagCOS;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate1LeftDiagCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate1LeftDiagSIN;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate1LeftDiagSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate1LeftDiagCOS;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate1LeftDiagCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate1LeftDiagSIN;
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1);
			gamepad->state.ly = dpadToAnalogY(dpad1);
			break;
		}
    }
    else if (pinRotate2Pressed) {
		switch (rotateLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate2LeftSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate2LeftCOS;
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate2LeftCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate2LeftSIN;
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate2LeftSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate2LeftCOS;
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate2LeftCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate2LeftSIN;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate2LeftDiagSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate2LeftDiagCOS;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate2LeftDiagCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID - rotate2LeftDiagSIN;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID + rotate2LeftDiagSIN;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate2LeftDiagCOS;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID - rotate2LeftDiagCOS;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID + rotate2LeftDiagSIN;
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1);
			gamepad->state.ly = dpadToAnalogY(dpad1);
			break;
		}
    }
	else {
		gamepad->state.lx = dpadToAnalogX(dpad1);
		gamepad->state.ly = dpadToAnalogY(dpad1);
	}

	if (pinRotate1Pressed && pinRotate2Pressed) {
		gamepad->state.dpad = dpad2; //Hold rotate1 + rotate2 turn on D-Pad
		gamepad->state.rx = GAMEPAD_JOYSTICK_MID;
		gamepad->state.ry = GAMEPAD_JOYSTICK_MID;
		
	}
	else if (pinRotate1Pressed) {
		switch (rotateRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate1RightSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate1RightCOS;
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate1RightCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate1RightSIN;
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate1RightSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate1RightCOS;
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate1RightCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate1RightSIN;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate1RightDiagSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate1RightDiagCOS;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate1RightDiagCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate1RightDiagSIN;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate1RightDiagSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate1RightDiagCOS;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate1RightDiagCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate1RightDiagSIN;
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
			break;
		}
	}
	else if (pinRotate2Pressed) {
		switch (rotateRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate2RightSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate2RightCOS;
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate2RightCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate2RightSIN;
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate2RightSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate2RightCOS;
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate2RightCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate2RightSIN;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate2RightDiagSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate2RightDiagCOS;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate2RightDiagCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID - rotate2RightDiagSIN;
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID + rotate2RightDiagSIN;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate2RightDiagCOS;
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID - rotate2RightDiagCOS;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID + rotate2RightDiagSIN;
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
			break;
		}
	}
	else {
		gamepad->state.rx = dpadToAnalogX(dpad2);
		gamepad->state.ry = dpadToAnalogY(dpad2);
	}
}


void RotateInput::SOCDRotateClean(SOCDMode socdMode) {
	// Rotate SOCD Last-Win Clean
	switch (rotateLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			rotateLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
			lastRotateUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastRotateUD != DIRECTION_NONE) {
			rotateLeftState ^= (lastRotateUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			rotateLeftState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			lastRotateUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		lastRotateUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		lastRotateUD = DIRECTION_DOWN;
		break;
	default:
		lastRotateUD = DIRECTION_NONE;
		break;
	}
	switch (rotateLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			rotateLeftState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			lastRotateLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (lastRotateLR != DIRECTION_NONE)
				rotateLeftState ^= (lastRotateLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				lastRotateLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		lastRotateLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		lastRotateLR = DIRECTION_RIGHT;
		break;
	default:
		lastRotateLR = DIRECTION_NONE;
		break;
	}
	
	// Rotate SOCD Last-Win Clean
	switch (rotateRightState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			rotateRightState ^= GAMEPAD_MASK_DOWN; // Remove Down
			lastRotateUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastRotateUD != DIRECTION_NONE) {
			rotateRightState ^= (lastRotateUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			rotateRightState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			lastRotateUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		lastRotateUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		lastRotateUD = DIRECTION_DOWN;
		break;
	default:
		lastRotateUD = DIRECTION_NONE;
		break;
	}
	switch (rotateRightState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			rotateRightState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			lastRotateLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (lastRotateLR != DIRECTION_NONE)
				rotateRightState ^= (lastRotateLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				lastRotateLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		lastRotateLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		lastRotateLR = DIRECTION_RIGHT;
		break;
	default:
		lastRotateLR = DIRECTION_NONE;
		break;
	}
}
