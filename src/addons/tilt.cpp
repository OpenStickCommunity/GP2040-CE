#include "addons/tilt.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "math.h"

bool TiltInput::available() {
	return Storage::getInstance().getAddonOptions().tiltOptions.enabled;
}

void TiltInput::setup() {
	const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;
	tiltLeftSOCDMode = options.tiltLeftSOCDMode;
	tiltRightSOCDMode = options.tiltRightSOCDMode;

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
	pinRotate1 = options.rotate1Pin;
	pinRotate2 = options.rotate2Pin;
	rotate1FactorLeft = options.factorRotate1Left;
	rotate2FactorLeft = options.factorRotate2Left;
	rotate3FactorLeft = options.factorRotate3Left;
	rotate4FactorLeft = options.factorRotate4Left;
	rotate5FactorLeft = options.factorRotate5Left;
	rotate6FactorLeft = options.factorRotate6Left;
	rotate1FactorRight = options.factorRotate1Right;
	rotate2FactorRight = options.factorRotate2Right;
	rotate3FactorRight = options.factorRotate3Right;
	rotate4FactorRight = options.factorRotate4Right;
	rotate5FactorRight = options.factorRotate5Right;
	rotate6FactorRight = options.factorRotate6Right;


	// Setup Tilt Key
	uint8_t pinTilt[12] = {
											pinTilt1,
											pinTilt2,
											pinTiltLeftAnalogDown,
											pinTiltLeftAnalogUp,
											pinTiltLeftAnalogLeft,
											pinTiltLeftAnalogRight,
											pinTiltRightAnalogDown,
											pinTiltRightAnalogUp,
											pinTiltRightAnalogLeft,
											pinTiltRightAnalogRight,
											pinRotate1,
											pinRotate2	};

	for (int i = 0; i < 12; i++) {
		if (pinTilt[i] != (uint8_t)-1) {
			gpio_init(pinTilt[i]);             // Initialize pin
			gpio_set_dir(pinTilt[i], GPIO_IN); // Set as INPUT
			gpio_pull_up(pinTilt[i]);          // Set as PULLUP
		}
	}

	dDebLeftState = 0;
	dDebRightState = 0;
	tiltLeftState = 0;
	tiltRightState = 0;

	leftLastTiltUD = DIRECTION_NONE;
	leftLastTiltLR = DIRECTION_NONE;

	rightLastTiltUD = DIRECTION_NONE;
	rightLastTiltLR = DIRECTION_NONE;

	uint32_t now = getMillis();
	for (int i = 0; i < 4; i++) {
		dpadTime[i] = now;
	}
}

void TiltInput::debounce()
{
	uint32_t now = getMillis();
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	for (int i = 0; i < 4; i++)
	{
		if ((dDebLeftState & dpadMasks[i]) != (tiltLeftState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebLeftState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	tiltLeftState = dDebLeftState;

	for (int i = 0; i < 4; i++)
	{
		if ((dDebRightState & dpadMasks[i]) != (tiltRightState & dpadMasks[i]) && (now - dpadTime[i]) > gamepad->debounceMS)
		{
			dDebRightState ^= dpadMasks[i];
			dpadTime[i] = now;
		}
	}
	tiltRightState = dDebRightState;
}

void TiltInput::preprocess()
{
	bool sliderLeft;
	bool sliderRight;
	
	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	sliderLeft = gamepad->getOptions().dpadMode == DPAD_MODE_LEFT_ANALOG;
	sliderRight = gamepad->getOptions().dpadMode == DPAD_MODE_RIGHT_ANALOG;

	// Need to invert since we're using pullups
	tiltLeftState = 0;
	if ((pinTiltLeftAnalogUp == (uint8_t)-1) && (pinTiltLeftAnalogDown == (uint8_t)-1) && (pinTiltLeftAnalogLeft == (uint8_t)-1) && (pinTiltLeftAnalogRight == (uint8_t)-1) && sliderLeft) {
		tiltLeftState |= gamepad->state.dpad;
	}
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
	if ((pinTiltRightAnalogUp == (uint8_t)-1) && (pinTiltRightAnalogDown == (uint8_t)-1) && (pinTiltRightAnalogLeft == (uint8_t)-1) && (pinTiltRightAnalogRight == (uint8_t)-1) && sliderRight) {
		tiltRightState |= gamepad->state.dpad;
	}
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

	// Debounce our directional pins
	debounce();

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void TiltInput::process()
{
	SOCDTiltClean(tiltLeftSOCDMode, tiltRightSOCDMode);

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
	bool pinRotate1Pressed = (pinRotate1 != (uint8_t)-1) ? !gpio_get(pinRotate1) : false;
	bool pinRotate2Pressed = (pinRotate2 != (uint8_t)-1) ? !gpio_get(pinRotate2) : false;

	// Scales input from 0-100% of maximum input
	double scaledTilt1FactorLeftX  = 1.0 - (tilt1FactorLeftX  / 100.0);
	double scaledTilt1FactorLeftY  = 1.0 - (tilt1FactorLeftY  / 100.0);
	double scaledTilt1FactorRightX = 1.0 - (tilt1FactorRightX / 100.0);
	double scaledTilt1FactorRightY = 1.0 - (tilt1FactorRightY / 100.0);
	double scaledTilt2FactorLeftX  = 1.0 - (tilt2FactorLeftX  / 100.0);
	double scaledTilt2FactorLeftY  = 1.0 - (tilt2FactorLeftY  / 100.0);
	double scaledTilt2FactorRightX = 1.0 - (tilt2FactorRightX / 100.0);
	double scaledTilt2FactorRightY = 1.0 - (tilt2FactorRightY / 100.0);
	
	double rotate1degreeLeft = rotate1FactorLeft;
	double rotate2degreeLeft = rotate2FactorLeft;
	double rotate3degreeLeft = rotate3FactorLeft;
	double rotate4degreeLeft = rotate4FactorLeft;
	double rotate5degreeLeft = rotate5FactorLeft;
	double rotate6degreeLeft = rotate6FactorLeft;
	double rotate1degreeRight = rotate1FactorRight;
	double rotate2degreeRight = rotate2FactorRight;
	double rotate3degreeRight = rotate3FactorRight;
	double rotate4degreeRight = rotate4FactorRight;
	double rotate5degreeRight = rotate5FactorRight;
	double rotate6degreeRight = rotate6FactorRight;

	uint8_t input_mode = gamepad->getOptions().inputMode;
	uint16_t joystickMid = GetJoystickMidValue(input_mode);

	// (Tilt1+Tilt2 or Rotate1+Rotate2) = LS or RS as DPad
	if ((pinTilt1Pressed && pinTilt2Pressed && !pinRotate1Pressed && !pinRotate2Pressed) || (!pinTilt1Pressed && !pinTilt2Pressed && pinRotate1Pressed && pinRotate2Pressed)) {
		gamepad->state.dpad |= dpad1|dpad2;
		gamepad->state.lx = joystickMid;
		gamepad->state.ly = joystickMid;
		gamepad->state.rx = joystickMid;
		gamepad->state.ry = joystickMid;
    }

	// Tilt 1
	else if ( pinTilt1Pressed && !pinTilt2Pressed && !pinRotate1Pressed && !pinRotate2Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1, input_mode) + (joystickMid - dpadToAnalogX(dpad1, input_mode)) * scaledTilt1FactorLeftX;
		gamepad->state.ly = dpadToAnalogY(dpad1, input_mode) + (joystickMid - dpadToAnalogY(dpad1, input_mode)) * scaledTilt1FactorLeftY;
		gamepad->state.rx = dpadToAnalogX(dpad2, input_mode) + (joystickMid - dpadToAnalogX(dpad2, input_mode)) * scaledTilt1FactorRightX;
		gamepad->state.ry = dpadToAnalogY(dpad2, input_mode) + (joystickMid - dpadToAnalogY(dpad2, input_mode)) * scaledTilt1FactorRightY;
    }

	// Tilt 2
	else if (!pinTilt1Pressed && pinTilt2Pressed && !pinRotate1Pressed && !pinRotate2Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1, input_mode) + (joystickMid - dpadToAnalogX(dpad1, input_mode)) * scaledTilt2FactorLeftX;
		gamepad->state.ly = dpadToAnalogY(dpad1, input_mode) + (joystickMid - dpadToAnalogY(dpad1, input_mode)) * scaledTilt2FactorLeftY;
		gamepad->state.rx = dpadToAnalogX(dpad2, input_mode) + (joystickMid - dpadToAnalogX(dpad2, input_mode)) * scaledTilt2FactorRightX;
		gamepad->state.ry = dpadToAnalogY(dpad2, input_mode) + (joystickMid - dpadToAnalogY(dpad2, input_mode)) * scaledTilt2FactorRightY;
    }

	// Rotate1
	else if (!pinTilt1Pressed && !pinTilt2Pressed && pinRotate1Pressed && !pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate1degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate1degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate1degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate1degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Rotate2
    else if (!pinTilt1Pressed && !pinTilt2Pressed && !pinRotate1Pressed && pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate2degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate2degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate2degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate2degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Rotate3
    else if (pinTilt1Pressed && !pinTilt2Pressed && pinRotate1Pressed && !pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate3degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate3degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate3degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate3degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Rotate4
    else if (!pinTilt1Pressed && pinTilt2Pressed && pinRotate1Pressed && !pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate4degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate4degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate4degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate4degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Rotate5
    else if (pinTilt1Pressed && !pinTilt2Pressed && !pinRotate1Pressed && pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate5degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate5degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate5degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate5degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Rotate6
    else if (!pinTilt1Pressed && pinTilt2Pressed && !pinRotate1Pressed && pinRotate2Pressed) {
		switch (tiltLeftState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.lx = joystickMid * (1 + sin(rotate6degreeLeft / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ly = joystickMid * (1 - cos(rotate6degreeLeft / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
			gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
			break;
		}

		switch (tiltRightState) {
		case (GAMEPAD_MASK_UP):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*0/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*0/2) );
			break;
		case (GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*1/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*1/2) );
			break;
		case (GAMEPAD_MASK_DOWN):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*2/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*2/2) );
			break;
		case (GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*3/2) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*3/2) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*1/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*1/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*3/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*3/4) );
			break;
		case (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*5/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*5/4) );
			break;
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT):
			gamepad->state.rx = joystickMid * (1 + sin(rotate6degreeRight / 180 * M_PI + M_PI*7/4) );
			gamepad->state.ry = joystickMid * (1 - cos(rotate6degreeRight / 180 * M_PI + M_PI*7/4) );
			break;
		default:
			gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
			gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
			break;
		}
    }

	// Not presssing any tilt or rotate buttons
	else if (!pinTilt1Pressed && !pinTilt2Pressed && !pinRotate1Pressed && !pinRotate2Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
		gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
		gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
		gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
	}
}


void TiltInput::SOCDTiltClean(SOCDMode leftSOCDMode, SOCDMode rightSOCDMode) {
	// Left Stick SOCD Cleaning
	switch (tiltLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (leftSOCDMode == SOCD_MODE_UP_PRIORITY)
			{
				tiltLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
				leftLastTiltUD = DIRECTION_UP; // We're in UP mode
			}
			else if (leftSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && leftLastTiltUD != DIRECTION_NONE)
				tiltLeftState ^= (leftLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
			else if (leftSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && leftLastTiltUD != DIRECTION_NONE)
				tiltLeftState ^= (leftLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else
				leftLastTiltUD = DIRECTION_NONE;
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

	switch (tiltLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (leftSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && leftLastTiltLR != DIRECTION_NONE)
				tiltLeftState ^= (leftLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
			else if (leftSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && leftLastTiltLR != DIRECTION_NONE)
				tiltLeftState ^= (leftLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else
				leftLastTiltLR = DIRECTION_NONE;
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
	switch (tiltRightState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (rightSOCDMode == SOCD_MODE_UP_PRIORITY)
			{
				tiltRightState ^= GAMEPAD_MASK_DOWN; // Remove Down
				rightLastTiltUD = DIRECTION_UP; // We're in UP mode
			}
			else if (rightSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && rightLastTiltUD != DIRECTION_NONE)
				tiltRightState ^= (rightLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
			else if (rightSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && rightLastTiltUD != DIRECTION_NONE)
				tiltRightState ^= (rightLastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else
				rightLastTiltUD = DIRECTION_NONE;
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

	switch (tiltRightState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (rightSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && rightLastTiltLR != DIRECTION_NONE)
				tiltRightState ^= (rightLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
			else if (rightSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && rightLastTiltLR != DIRECTION_NONE)
				tiltRightState ^= (rightLastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else
				rightLastTiltLR = DIRECTION_NONE;
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