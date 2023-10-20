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
	analogmodLeftSOCDMode = options.analogmodLeftSOCDMode;
	analogmodRightSOCDMode = options.analogmodRightSOCDMode;

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

	leftLastAnalogModUD = DIRECTION_NONE;
	leftLastAnalogModLR = DIRECTION_NONE;

	rightLastAnalogModUD = DIRECTION_NONE;
	rightLastAnalogModLR = DIRECTION_NONE;

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
	SOCDAnalogModClean(analogmodLeftSOCDMode, analogmodRightSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	// Set AnalogMod Output
	OverrideGamepad(gamepad, analogmodLeftState, analogmodRightState);
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

	// (Tilt1+Tilt2 or Rotate1+Rotate2) = RS as DPad
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
		switch (analogmodLeftState) {
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
		
		switch (analogmodRightState) {
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
	
	// Not presssing any analog mod buttons
	else if (!pinTilt1Pressed && !pinTilt2Pressed && !pinRotate1Pressed && !pinRotate2Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1, input_mode);
		gamepad->state.ly = dpadToAnalogY(dpad1, input_mode);
		gamepad->state.rx = dpadToAnalogX(dpad2, input_mode);
		gamepad->state.ry = dpadToAnalogY(dpad2, input_mode);
	}
}


void AnalogModInput::SOCDAnalogModClean(SOCDMode leftSOCDMode, SOCDMode rightSOCDMode) {
	// Left Stick SOCD Cleaning
	switch (analogmodLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (leftSOCDMode == SOCD_MODE_UP_PRIORITY)
			{
				analogmodLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
				leftLastAnalogModUD = DIRECTION_UP; // We're in UP mode
			}
			else if (leftSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && leftLastAnalogModUD != DIRECTION_NONE)
				analogmodLeftState ^= (leftLastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
			else if (leftSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && leftLastAnalogModUD != DIRECTION_NONE)
				analogmodLeftState ^= (leftLastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else
				leftLastAnalogModUD = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_UP:
			leftLastAnalogModUD = DIRECTION_UP;
			break;

		case GAMEPAD_MASK_DOWN:
			leftLastAnalogModUD = DIRECTION_DOWN;
			break;

		default:
			leftLastAnalogModUD = DIRECTION_NONE;
			break;
	}
	
	switch (analogmodLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (leftSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && leftLastAnalogModLR != DIRECTION_NONE)
				analogmodLeftState ^= (leftLastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
			else if (leftSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && leftLastAnalogModLR != DIRECTION_NONE)
				analogmodLeftState ^= (leftLastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else
				leftLastAnalogModLR = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_LEFT:
			leftLastAnalogModLR = DIRECTION_LEFT;
			break;

		case GAMEPAD_MASK_RIGHT:
			leftLastAnalogModLR = DIRECTION_RIGHT;
			break;

		default:
			leftLastAnalogModLR = DIRECTION_NONE;
			break;
	}
	
	// Right Stick SOCD Cleaning
	switch (analogmodRightState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (rightSOCDMode == SOCD_MODE_UP_PRIORITY)
			{
				analogmodRightState ^= GAMEPAD_MASK_DOWN; // Remove Down
				rightLastAnalogModUD = DIRECTION_UP; // We're in UP mode
			}
			else if (rightSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && rightLastAnalogModUD != DIRECTION_NONE)
				analogmodRightState ^= (rightLastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
			else if (rightSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && rightLastAnalogModUD != DIRECTION_NONE)
				analogmodRightState ^= (rightLastAnalogModUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else
				rightLastAnalogModUD = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_UP:
			rightLastAnalogModUD = DIRECTION_UP;
			break;

		case GAMEPAD_MASK_DOWN:
			rightLastAnalogModUD = DIRECTION_DOWN;
			break;

		default:
			rightLastAnalogModUD = DIRECTION_NONE;
			break;
	}
	
	switch (analogmodRightState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (rightSOCDMode == SOCD_MODE_SECOND_INPUT_PRIORITY && rightLastAnalogModLR != DIRECTION_NONE)
				analogmodRightState ^= (rightLastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT;
			else if (rightSOCDMode == SOCD_MODE_FIRST_INPUT_PRIORITY && rightLastAnalogModLR != DIRECTION_NONE)
				analogmodRightState ^= (rightLastAnalogModLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else
				rightLastAnalogModLR = DIRECTION_NONE;
			break;

		case GAMEPAD_MASK_LEFT:
			rightLastAnalogModLR = DIRECTION_LEFT;
			break;

		case GAMEPAD_MASK_RIGHT:
			rightLastAnalogModLR = DIRECTION_RIGHT;
			break;

		default:
			rightLastAnalogModLR = DIRECTION_NONE;
			break;
	}
}
