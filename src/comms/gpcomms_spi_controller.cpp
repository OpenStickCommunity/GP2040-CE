#include "comms/gpcomms_spi_controller.h"
#include "storagemanager.h"

bool GPCommsSPIControllerAddon::available() {
	const GPCommsSPIControllerOptions& options = Storage::getInstance().getAddonOptions().gpCommsSPIControllerOptions;
	return (options.enabled && PeripheralManager::getInstance().isSPIEnabled(options.spiBlock));
}

void GPCommsSPIControllerAddon::setup() {
	const GPCommsSPIControllerOptions& options = Storage::getInstance().getAddonOptions().gpCommsSPIControllerOptions;
	spi = PeripheralManager::getInstance().getSPI(options.spiBlock);
}

void GPCommsSPIControllerAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	while (!spi->isWriteable())
		tight_loop_contents();

	sendGamepadState(gamepad);
}

/************************
 * SPI send functions
 ************************/

void GPCommsSPIControllerAddon::sendGamepadStatus(Gamepad *gamepad) {
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

void GPCommsSPIControllerAddon::sendGamepadState(Gamepad *gamepad) {
	GPComms_State gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(GPComms_State));
	spi->write(buf, sizeof(GPComms_State) + 1);
	memset(buf, 0, GPCOMMS_BUFFER_SIZE);
}

void GPCommsSPIControllerAddon::sendGamepadMessage(Gamepad *gamepad, char *text, uint16_t length) {
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
