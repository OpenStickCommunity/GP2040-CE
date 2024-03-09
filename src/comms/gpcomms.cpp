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

uint8_t GPComms::fillBufferStatus(uint8_t *buf, Gamepad *gamepad, const AddonOptions &addonOptions) {
	GamepadOptions options = gamepad->getOptions();

	GPComms_Status gpStatus = {
		.inputMode = options.inputMode,
		.turboRate = static_cast<int8_t>(addonOptions.turboOptions.shotCount),
		.macroEnabled = addonOptions.macroOptions.enabled,
		.dpadMode = options.dpadMode,
		.socdMode = options.socdMode,
	};

	buf[0] = GPCMD_STATUS;
	memcpy(&buf[1], &gpStatus, sizeof(GPComms_Status));
	return 1 + sizeof(GPComms_Status);
}

uint8_t GPComms::fillBufferState(uint8_t *buf, Gamepad *gamepad) {
	GPComms_State gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(GPComms_State));
	return 1 + sizeof(GPComms_State);
}

uint8_t GPComms::fillBufferMessage(uint8_t *buf, char *text, uint16_t textLength) {
	GPComms_Message gpMessage = {
		.length = textLength,
		.message = text,
	};

	buf[0] = GPCMD_MESSAGE;
	memcpy(&buf[1], &gpMessage.length, 2);
	memcpy(&buf[3], text, textLength);
	return 1 + sizeof(uint16_t) + textLength;
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
