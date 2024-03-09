#include "comms/gpcomms_spi_broadcast.h"
#include "storagemanager.h"

bool GPCommsSPIBroadcastAddon::available() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_SPI_BROADCAST && PeripheralManager::getInstance().isSPIEnabled(options.hwBlock));
}

void GPCommsSPIBroadcastAddon::setup() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
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
	const AddonOptions &addonOptions = Storage::getInstance().getAddonOptions();
	uint16_t size = GPComms::fillBufferStatus(buf, gamepad, addonOptions);
	spi->write(buf, size);
}

void GPCommsSPIBroadcastAddon::sendState(Gamepad *gamepad) {
	uint16_t size = GPComms::fillBufferState(buf, gamepad);
	spi->write(buf, size);
}

void GPCommsSPIBroadcastAddon::sendMessage(char *text, uint16_t length) {
	uint16_t size = GPComms::fillBufferMessage(buf, text, length);
	spi->write(buf, size);
}
