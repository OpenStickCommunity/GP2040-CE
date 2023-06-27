#include "addons/tilt.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

// These constants define the adjustment factors for gamepad analog inputs under different tilt states.
// Defines the behavior of the left analog stick when the Tilt1 and Tilt2 buttons are pressed.
// The main purpose of the left analog stick is to move the character.
// Pressing it simultaneously with Tilt 1 will make the character walk.
// Pressing it simultaneously with Tilt 2 will make the character walk more slowly.
const double TILT1_FACTOR_LEFT_X = 0.35; // Adjustment factor for left analog stick X direction
const double TILT1_FACTOR_LEFT_Y = 0.45; // Adjustment factor for left analog stick Y direction
const double TILT2_FACTOR_LEFT_X = 0.65; // Adjustment factor for left analog stick X direction
const double TILT2_FACTOR_LEFT_Y = 0.35; // Adjustment factor for left analog stick Y direction

// The Right analog stick has 8 directions, which can be handled by pressing up, down, left, right, and simultaneously.
// This function adds to that the ability to tilt it at an angle closer to horizontal than diagonal.
const double TILT1_FACTOR_RIGHT_X = 0.3; // Adjustment factor for right analog stick X direction
const double TILT1_FACTOR_RIGHT_Y = 1.7; // Adjustment factor for right analog stick Y direction
const double TILT2_FACTOR_RIGHT_X = 0.3; // Adjustment factor for right analog stick X direction
const double TILT2_FACTOR_RIGHT_Y = 0.3; // Adjustment factor for right analog stick Y direction

bool TiltInput::available() {
	return Storage::getInstance().getAddonOptions().tiltOptions.enabled;
}

void TiltInput::setup() {
	const TiltOptions& options = Storage::getInstance().getAddonOptions().tiltOptions;	
	tiltSOCDMode = options.tiltSOCDMode;

	pinTilt1 = options.tilt1Pin;
	pinTilt2 = options.tilt2Pin;
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

	dDebLeftState = 0;
	dDebRightState = 0;
	tiltLeftState = 0;
	tiltRightState = 0;

	lastGPUD = DIRECTION_NONE;
	lastGPLR = DIRECTION_NONE;

	lastTiltUD = DIRECTION_NONE;
	lastTiltLR = DIRECTION_NONE;

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

	// Debounce our directional pins
	debounce();

	// Convert gamepad from process() output to uint8 value
	uint8_t gamepadState = gamepad->state.dpad;

	gamepad->state.dpad = gamepadState;
}

void TiltInput::process()
{
	const AddonOptions& options = Storage::getInstance().getAddonOptions();
	SOCDTiltClean(tiltSOCDMode);

	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	uint8_t tiltLeftOut = tiltLeftState;
	uint8_t tiltRightOut = tiltRightState;

	// Set Tilt Output
	OverrideGamepad(gamepad, tiltLeftOut, tiltRightOut);
}

//The character's movement changes depending on the degree to which the stick is tilted.
//I added the functionality to allow the all - button controller to perform the operations that can be performed by the sticks.
//Two buttons, tilt1 and tilt2, have been implemented.
//While pressing these buttons, pressing the left or right analog stick will cause the character to walk or walk more slowly.
//Since this is an auxiliary function for appeals and such,
//pressing Tilt1 and Tilt2 at the same time will cause the light analog stick to correspond to each of the DPad methods.
void TiltInput::OverrideGamepad(Gamepad* gamepad, uint8_t dpad1, uint8_t dpad2) {
	bool pinTilt1Pressed = pinTilt1 != (uint8_t)-1 && !gpio_get(pinTilt1);
	bool pinTilt2Pressed = pinTilt2 != (uint8_t)-1 && !gpio_get(pinTilt2);

	if (pinTilt1Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * TILT1_FACTOR_LEFT_X;
		gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * TILT1_FACTOR_LEFT_Y;
	}
	else if (pinTilt2Pressed) {
		gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * TILT2_FACTOR_LEFT_X;
		gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * TILT2_FACTOR_LEFT_Y;
	}
	else {
		gamepad->state.lx = dpadToAnalogX(dpad1);
		gamepad->state.ly = dpadToAnalogY(dpad1);
	}

	if (pinTilt1Pressed && pinTilt2Pressed) {
		gamepad->state.dpad = dpad2; //Hold tilt1 + tilt2 turn on D-Pad
	}
	else if (pinTilt1Pressed) {
		if (dpad2 & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
			gamepad->state.rx = dpadToAnalogX(dpad2) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad2)) * TILT1_FACTOR_RIGHT_X;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID * TILT1_FACTOR_RIGHT_Y;
		}
		else {
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
		}
	}
	else if (pinTilt2Pressed) {
		if (dpad2 & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
			gamepad->state.rx = dpadToAnalogX(dpad2) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad2)) * TILT2_FACTOR_RIGHT_X;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID * TILT2_FACTOR_RIGHT_Y;
		}
		else {
			gamepad->state.rx = dpadToAnalogX(dpad2);
			gamepad->state.ry = dpadToAnalogY(dpad2);
		}
	}
	else {
		gamepad->state.rx = dpadToAnalogX(dpad2);
		gamepad->state.ry = dpadToAnalogY(dpad2);
	}
}


void TiltInput::SOCDTiltClean(SOCDMode socdMode) {
	// Tilt SOCD Last-Win Clean
	switch (tiltLeftState & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) {
	case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN): // If last state was Up or Down, exclude it from our gamepad
		if (socdMode == SOCD_MODE_UP_PRIORITY) {
			tiltLeftState ^= GAMEPAD_MASK_DOWN; // Remove Down
			lastTiltUD = DIRECTION_UP; // We're in UP mode
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY && lastTiltUD != DIRECTION_NONE) {
			tiltLeftState ^= (lastTiltUD == DIRECTION_UP) ? GAMEPAD_MASK_UP : GAMEPAD_MASK_DOWN;
		}
		else {
			tiltLeftState ^= (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN); // Remove UP and Down in Neutral
			lastTiltUD = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_UP:
		lastTiltUD = DIRECTION_UP;
		break;
	case GAMEPAD_MASK_DOWN:
		lastTiltUD = DIRECTION_DOWN;
		break;
	default:
		lastTiltUD = DIRECTION_NONE;
		break;
	}
	switch (tiltLeftState & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) {
	case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
		if (socdMode == SOCD_MODE_UP_PRIORITY || socdMode == SOCD_MODE_NEUTRAL) {
			tiltLeftState ^= (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT); // Remove L + R to Neutral
			lastTiltLR = DIRECTION_NONE;
		}
		else if (socdMode == SOCD_MODE_SECOND_INPUT_PRIORITY) {
			if (lastTiltLR != DIRECTION_NONE)
				tiltLeftState ^= (lastTiltLR == DIRECTION_LEFT) ? GAMEPAD_MASK_LEFT : GAMEPAD_MASK_RIGHT; // Last Win
			else
				lastTiltLR = DIRECTION_NONE;
		}
		break;
	case GAMEPAD_MASK_LEFT:
		lastTiltLR = DIRECTION_LEFT;
		break;
	case GAMEPAD_MASK_RIGHT:
		lastTiltLR = DIRECTION_RIGHT;
		break;
	default:
		lastTiltLR = DIRECTION_NONE;
		break;
	}
}
