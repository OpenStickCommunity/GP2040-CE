#include "comms/gpcomms.h"

static Mask_t GPComms::gpioState = 0;

void GPComms::updateGamepad() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	memcpy(&gamepad->state, GPComms::getGamepadState(), sizeof(GamepadState));
}

void GPComms::handleBuffer(uint8_t *buf, int size) {
	// Grab command byte and shift buffer values
	uint8_t command = buf[0];
	for (int i = 0; i < size - 1; i++)
		buf[i] = buf[i + 1];

	switch (command) {
		case GPCMD_STATUS:
			GPComms::handleStatus(reinterpret_cast<GPComms_Status *>(buf));
			break;

		case GPCMD_STATE:
			GPComms::handleState(reinterpret_cast<GPComms_State *>(buf));
			break;

		case GPCMD_MESSAGE:
			GPComms::handleMessage(reinterpret_cast<GPComms_Message *>(buf));
			break;

		case GPCMD_ACK:
			break;

		case GPCMD_UNKNOWN:
		default:
			break;
	}
}

void GPComms::handleStatus(GPComms_Status *gpStatus) {
	(void)0;
}

void GPComms::handleState(GPComms_State *gpState) {
	gamepadState = gpState->gamepadState;
	gpioState = gpState->gpioState;
}

void GPComms::handleMessage(GPComms_Message *gpMessage) {
	(void)0;
}
