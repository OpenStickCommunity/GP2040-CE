#include "comms/gpcomms_spi_broadcast.h"
#include "storagemanager.h"

bool GPCommsSPIBroadcastAddon::available() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_SPI_BROADCAST && PeripheralManager::getInstance().isSPIEnabled(options.hwBlock));
}

void GPCommsSPIBroadcastAddon::setup() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	spi = PeripheralManager::getInstance().getSPI(options.hwBlock);
}

void GPCommsSPIBroadcastAddon::process() {
	if (spi->isWriteable()) {
		Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
		sendState(gamepad);
	}
}

/************************
 * SPI send functions
 ************************/

void GPCommsSPIBroadcastAddon::sendStatus(Gamepad *gamepad) {
	GamepadOptions options = gamepad->getOptions();
	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();

	GPComms_Status gpStatus = {
		.inputMode = options.inputMode,
		.turboRate = static_cast<int8_t>(addonOptions.turboOptions.shotCount),
		.macroEnabled = addonOptions.macroOptions.enabled,
		.dpadMode = options.dpadMode,
		.socdMode = options.socdMode,
	};

	buf[0] = GPCMD_STATUS;
	memcpy(&buf[1], &gpStatus, sizeof(GPComms_Status));
	spi->write(buf, sizeof(GPComms_Status) + 1);
	memset(buf, 0, GPCOMMS_BUFFER_SIZE);
}

void GPCommsSPIBroadcastAddon::sendState(Gamepad *gamepad) {
	GPComms_State gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(GPComms_State));
	spi->write(buf, sizeof(GPComms_State) + 1);
	memset(buf, 0, GPCOMMS_BUFFER_SIZE);
}

void GPCommsSPIBroadcastAddon::sendMessage(char *text, uint16_t length) {
	GPComms_Message gpMessage = {
		.length = length,
		.message = text,
	};

	buf[0] = GPCMD_MESSAGE;
	memcpy(&buf[1], &gpMessage.length, 2);
	memcpy(&buf[3], text, length);
	spi->write(buf, gpMessage.length + length + 1);
	memset(buf, 0, GPCOMMS_BUFFER_SIZE);
}
