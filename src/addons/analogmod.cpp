#include "addons/analogmod.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

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


	// Setup AnalogMod Key
	uint8_t pinAnalogMod[10] = {
											pinTilt1,
											pinTilt2,
											pinAnalogModLeftAnalogDown,
											pinAnalogModLeftAnalogUp,
											pinAnalogModLeftAnalogLeft,
											pinAnalogModLeftAnalogRight,
											pinAnalogModRightAnalogDown,
											pinAnalogModRightAnalogUp,
											pinAnalogModRightAnalogLeft,
											pinAnalogModRightAnalogRight };

	for (int i = 0; i < 10; i++) {
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

	double scaledTilt1FactorLeftX  = tilt1FactorLeftX  / 100.0;
	double scaledTilt1FactorLeftY  = tilt1FactorLeftY  / 100.0;
	double scaledTilt1FactorRightX = tilt1FactorRightX / 100.0;
	double scaledTilt1FactorRightY = tilt1FactorRightY / 100.0;
	double scaledTilt2FactorLeftX  = tilt2FactorLeftX  / 100.0;
	double scaledTilt2FactorLeftY  = tilt2FactorLeftY  / 100.0;
	double scaledTilt2FactorRightX = tilt2FactorRightX / 100.0;
	double scaledTilt2FactorRightY = tilt2FactorRightY / 100.0;


    if (pinTilt1Pressed) {
        gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * scaledTilt1FactorLeftX;
        gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * scaledTilt1FactorLeftY;
    }
    else if (pinTilt2Pressed) {
        gamepad->state.lx = dpadToAnalogX(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogX(dpad1)) * scaledTilt2FactorLeftX;
        gamepad->state.ly = dpadToAnalogY(dpad1) + (GAMEPAD_JOYSTICK_MID - dpadToAnalogY(dpad1)) * scaledTilt2FactorLeftY;
    }
	else {
		gamepad->state.lx = dpadToAnalogX(dpad1);
		gamepad->state.ly = dpadToAnalogY(dpad1);
	}

	if (pinTilt1Pressed && pinTilt2Pressed) {
		gamepad->state.dpad = dpad2; //Hold tilt1 + tilt2 turn on D-Pad
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
}
