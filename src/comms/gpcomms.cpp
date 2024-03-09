#include "comms/gpcomms.h"
#include "gamepad.h"
#include "storagemanager.h"

GamepadState GPComms::gamepadState;
Mask_t GPComms::gpioState = 0;

void GPComms::updateGamepad() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->state = *GPComms::getGamepadState();
}

void GPComms::handleBuffer(uint8_t *buf, int size) {
	uint8_t command = buf[0];
	uint8_t *payload = &buf[1];

	switch (command) {
		case GPCMD_STATUS:
			GPComms::handleStatus(reinterpret_cast<GPComms_Status *>(payload));
			break;

		case GPCMD_STATE:
			GPComms::handleState(reinterpret_cast<GPComms_State *>(payload));
			break;

		case GPCMD_MESSAGE:
			GPComms::handleMessage(reinterpret_cast<GPComms_Message *>(payload));
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
