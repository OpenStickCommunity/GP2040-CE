#include "addons/analogmod.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "math.h"

bool AnalogModInput::available() {
	return Storage::getInstance().getAddonOptions().analogmodOptions.enabled;
}

void AnalogModInput::setup() {
	const AnalogModOptions& options = Storage::getInstance().getAddonOptions().analogmodOptions;	
	analogmodSOCDMode = options.analogmodSOCDMode;

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
	pinAnalogModLeftAnalogDown = options.analogmodLeftAnalogDownPin;
	pinAnalogModLeftAnalogUp = options.analogmodLeftAnalogUpPin;
	pinAnalogModLeftAnalogLeft = options.analogmodLeftAnalogLeftPin;
	pinAnalogModLeftAnalogRight = options.analogmodLeftAnalogRightPin;
	pinAnalogModRightAnalogDown = options.analogmodRightAnalogDownPin;
	pinAnalogModRightAnalogUp = options.analogmodRightAnalogUpPin;
	pinAnalogModRightAnalogLeft = options.analogmodRightAnalogLeftPin;
	pinAnalogModRightAnalogRight = options.analogmodRightAnalogRightPin;
	pinRotate1 = options.rotate1Pin;
	rotate1FactorLeft = options.factorRotate1Left;
	rotate1FactorRight = options.factorRotate1Right;
	pinRotate2 = options.rotate2Pin;
	rotate2FactorLeft = options.factorRotate2Left;
	rotate2FactorRight = options.factorRotate2Right;


	// Setup AnalogMod Key
	uint8_t pinAnalogMod[12] = {
											pinTilt1,
											pinTilt2,
											pinAnalogModLeftAnalogDown,
											pinAnalogModLeftAnalogUp,
											pinAnalogModLeftAnalogLeft,
											pinAnalogModLeftAnalogRight,
											pinAnalogModRightAnalogDown,
											pinAnalogModRightAnalogUp,
											pinAnalogModRightAnalogLeft,
											pinAnalogModRightAnalogRight,
											pinRotate1,
											pinRotate2	};

	for (int i = 0; i < 12; i++) {
		if (pinAnalogMod[i] != (uint8_t)-1) {
			gpio_init(pinAnalogMod[i]);             // Initialize pin
			gpio_set_dir(pinAnalogMod[i], GPIO_IN); // Set as INPUT
			gpio_pull_up(pinAnalogMod[i]);          // Set as PULLUP
		}
	}

	dDebLeftState = 0;
	dDebRightState = 0;
	analogmodLeftState = 0;
	analogmodRightState = 0;

	lastGPUD = DIRECTION_NONE;
	lastGPLR = DIRECTION_NONE;

	lastAnalogModUD = DIRECTION_NONE;
	lastAnalogModLR = DIRECTION_NONE;

	uint32_t now = getMillis();
	for (int i = 0; i < 4; i++) {
		dpadTime[i] = now;
	}
}

void AnalogModInput::debounce()
{
	uint32_t now = getMillis();
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	for (int i = 0; i < 4; i++)
	{
		if ((dDebLeftState & dpadMasks[i]) != (analogmodLeftState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebLeftState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	analogmodLeftState = dDebLeftState;

	for (int i = 0; i < 4; i++)
	{
		if ((dDebRightState & dpadMasks[i]) != (analogmodRightState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebRightState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	analogmodRightState = dDebRightState;
}

void AnalogModInput::preprocess()
{
	bool sliderLeft;
	bool sliderRight;
	
	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	sliderLeft = gamepad->getOptions().dpadMode == DPAD_MODE_LEFT_ANALOG;
	sliderRight = gamepad->getOptions().dpadMode == DPAD_MODE_RIGHT_ANALOG;

	// Need to invert since we're using pullups
	analogmodLeftState = 0;
	if ((pinAnalogModLeftAnalogUp == (uint8_t)-1) && (pinAnalogModLeftAnalogDown == (uint8_t)-1) && (pinAnalogModLeftAnalogLeft == (uint8_t)-1) && (pinAnalogModLeftAnalogRight == (uint8_t)-1) && sliderLeft) {
		analogmodLeftState |= gamepad->state.dpad;
	}
	if (pinAnalogModLeftAnalogUp != (uint8_t)-1) {
		analogmodLeftState |= (!gpio_get(pinAnalogModLeftAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinAnalogModLeftAnalogDown != (uint8_t)-1) {
		analogmodLeftState |= (!gpio_get(pinAnalogModLeftAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinAnalogModLeftAnalogLeft != (uint8_t)-1) {
		analogmodLeftState |= (!gpio_get(pinAnalogModLeftAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinAnalogModLeftAnalogRight != (uint8_t)-1) {
		analogmodLeftState |= (!gpio_get(pinAnalogModLeftAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	analogmodRightState = 0;
	if ((pinAnalogModRightAnalogUp == (uint8_t)-1) && (pinAnalogModRightAnalogDown == (uint8_t)-1) && (pinAnalogModRightAnalogLeft == (uint8_t)-1) && (pinAnalogModRightAnalogRight == (uint8_t)-1) && sliderRight) {
		analogmodRightState |= gamepad->state.dpad;
	}
	if (pinAnalogModRightAnalogUp != (uint8_t)-1) {
		analogmodRightState |= (!gpio_get(pinAnalogModRightAnalogUp) ? gamepad->mapDpadUp->buttonMask : 0);
	}
	if (pinAnalogModRightAnalogDown != (uint8_t)-1) {
		analogmodRightState |= (!gpio_get(pinAnalogModRightAnalogDown) ? gamepad->mapDpadDown->buttonMask : 0);
	}
	if (pinAnalogModRightAnalogLeft != (uint8_t)-1) {
		analogmodRightState |= (!gpio_get(pinAnalogModRightAnalogLeft) ? gamepad->mapDpadLeft->buttonMask : 0);
	}
	if (pinAnalogModRightAnalogRight != (uint8_t)-1) {
		analogmodRightState |= (!gpio_get(pinAnalogModRightAnalogRight) ? gamepad->mapDpadRight->buttonMask : 0);
	}

	// Debounce our directional pins
	debounce();

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void AnalogModInput::process()
{
	SOCDAnalogModClean(analogmodSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	uint8_t analogmodLeftOut = analogmodLeftState;
	uint8_t analogmodRightOut = analogmodRightState;

	// Set AnalogMod Output
	OverrideGamepad(gamepad, analogmodLeftOut, analogmodRightOut);
}

//The character's movement changes depending on the degree to which the stick is tilted.
//Addon will allow the all - button controller to perform the operations that can be performed by the sticks.
//Two buttons, tilt1 and tilt2, have been implemented.
//While pressing these buttons, pressing the left or right analog stick will cause the character to walk or walk more slowly.
//Since this is an auxiliary function for appeals and such,
//pressing Tilt1 and Tilt2 at the same time will cause the light analog stick to correspond to each of the DPad methods.

void AnalogModInput::OverrideGamepad(Gamepad* gamepad, uint8_t dpad1, uint8_t dpad2) {
	bool pinTilt1Pressed = (pinTilt1 != (uint8_t)-1) ? !gpio_get(pinTilt1) : false;
	bool pinTilt2Pressed = (pinTilt2 != (uint8_t)-1) ? !gpio_get(pinTilt2) : false;
	bool pinRotate1Pressed = (pinRotate1 != (uint8_t)-1) ? !gpio_get(pinRotate1) : false;
	bool pinRotate2Pressed = (pinRotate2 != (uint8_t)-1) ? !gpio_get(pinRotate2) : false;

	double scaledTilt1FactorLeftX  = tilt1FactorLeftX  / 100.0;
	double scaledTilt1FactorLeftY  = tilt1FactorLeftY  / 100.0;
	double scaledTilt1FactorRightX = tilt1FactorRightX / 100.0;
	double scaledTilt1FactorRightY = tilt1FactorRightY / 100.0;
	double scaledTilt2FactorLeftX  = tilt2FactorLeftX  / 100.0;
	double scaledTilt2FactorLeftY  = tilt2FactorLeftY  / 100.0;
	double scaledTilt2FactorRightX = tilt2FactorRightX / 100.0;
	double scaledTilt2FactorRightY = tilt2FactorRightY / 100.0;
	
	uint16_t stickradius = GAMEPAD_JOYSTICK_MAX - GAMEPAD_JOYSTICK_MID;
	double rotate1degreeLeft = rotate1FactorLeft;
	double rotate1degreeRight = rotate1FactorRight;
	double rotate2degreeLeft = rotate2FactorLeft;
	double rotate2degreeRight = rotate2FactorRight;
	
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

	// Tilt 1 has priority over Tilt 2
    if (pinTilt1Pressed) {
        gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * scaledTilt1FactorLeftX;
        gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * scaledTilt1FactorLeftY;
    }
    else if (pinTilt2Pressed) {
        gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * scaledTilt2FactorLeftX;
        gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * scaledTilt2FactorLeftY;
    }
	// Tilt has priority over Rotate
	// Rotate 1 has priority over Rotate 2
    else if (pinRotate1Pressed) {
		switch (analogmodLeftState) {
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
		switch (analogmodLeftState) {
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
	
	
	// Tilt 1 has priority over Tilt 2
	// Hold Tilt1 + Tilt2 turn on D-Pad
	if (pinTilt1Pressed && pinTilt2Pressed) {
		gamepad->state.dpad = dpad2;
		gamepad->state.rx = GAMEPAD_JOYSTICK_MID;
		gamepad->state.ry = GAMEPAD_JOYSTICK_MID;
	}
	else if (pinTilt1Pressed) {
		if (dpad2 & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
			gamepad->state.rx = dpadToAnalogX(dpad2) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad2)) * scaledTilt1FactorRightX;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID * scaledTilt1FactorRightY;
		}
		else {
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
		}
	}
	else if (pinTilt2Pressed) {
		if (dpad2 & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
			gamepad->state.rx = dpadToAnalogX(dpad2) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad2)) * scaledTilt2FactorRightX;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID * scaledTilt2FactorRightY;
		}
		else {
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
		}
	}
	// Tilt has priority over Rotate
	// Rotate 1 has priority over Rotate 2
	// Hold Rotate1 + Rotate2 turn on D-Pad
	else if (pinRotate1Pressed && pinRotate2Pressed) {
		gamepad->state.dpad = dpad2;
		gamepad->state.rx = GAMEPAD_JOYSTICK_MID;
		gamepad->state.ry = GAMEPAD_JOYSTICK_MID;
		
	}
	else if (pinRotate1Pressed) {
		switch (analogmodRightState) {
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
		switch (analogmodRightState) {
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


void AnalogModInput::SOCDAnalogModClean(SOCDMode socdMode) {
	// AnalogMod SOCD Last-Win Clean
	switch (analogmodLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			analogmodLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
			lastAnalogModUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastAnalogModUD != DIRECTION_NONE) {
			analogmodLeftState ^= (lastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			analogmodLeftState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			lastAnalogModUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		lastAnalogModUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		lastAnalogModUD = DIRECTION_DOWN;
		break;
	default:
		lastAnalogModUD = DIRECTION_NONE;
		break;
	}
	switch (analogmodLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			analogmodLeftState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			lastAnalogModLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (lastAnalogModLR != DIRECTION_NONE)
				analogmodLeftState ^= (lastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				lastAnalogModLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		lastAnalogModLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		lastAnalogModLR = DIRECTION_RIGHT;
		break;
	default:
		lastAnalogModLR = DIRECTION_NONE;
		break;
	}
	
	// AnalogMod SOCD Last-Win Clean
	switch (analogmodRightState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			analogmodRightState ^= GAMEPAD_MASK_DOWN; // Remove Down
			lastAnalogModUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastAnalogModUD != DIRECTION_NONE) {
			analogmodRightState ^= (lastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			analogmodRightState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			lastAnalogModUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		lastAnalogModUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		lastAnalogModUD = DIRECTION_DOWN;
		break;
	default:
		lastAnalogModUD = DIRECTION_NONE;
		break;
	}
	switch (analogmodRightState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			analogmodRightState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			lastAnalogModLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (lastAnalogModLR != DIRECTION_NONE)
				analogmodRightState ^= (lastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				lastAnalogModLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		lastAnalogModLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		lastAnalogModLR = DIRECTION_RIGHT;
		break;
	default:
		lastAnalogModLR = DIRECTION_NONE;
		break;
	}
}
