#include "comms/gpcomms.h"
#include "gamepad.h"
#include "storagemanager.h"

GamepadState GPComms::gamepadState;
Mask_t GPComms::gpioState = 0;

void GPComms::readGamepad() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->state = *GPComms::getGamepadState();
	gamepad->debouncedGpio = GPComms::getGpioState();
}

void GPComms::handleBuffer(uint8_t *buf, int size) {
	uint8_t command = buf[0];
	uint8_t *payload = &buf[1];

	switch (command) {
		case GPCMD_STATE:
			GPComms::handleState(payload);
			break;

		case GPCMD_STATUS:
			GPComms::handleStatus(payload);
			break;

		case GPCMD_MESSAGE:
			GPComms::handleMessage(payload);
			break;

		case GPCMD_ACK:
			break;

		case GPCMD_UNKNOWN:
		default:
			break;
	}
}

void GPComms::handleStatus(uint8_t *payload) {
	(void)0;
}

void GPComms::handleState(uint8_t *payload) {
	static GPComms_State gpState;
	memcpy(&gpState, payload, sizeof(GPComms_State));
	gamepadState = gpState.gamepadState;
	gpioState = gpState.gpioState;
}

void GPComms::handleMessage(uint8_t *payload) {
	(void)0;
}
