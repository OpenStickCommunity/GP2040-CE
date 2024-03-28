#include "comms/gpcomms_spi_broadcast.h"
#include "storagemanager.h"
#include "peripheral_spi.h"

bool GPCommsSPIBroadcastAddon::available() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_SPI_BROADCAST && PeripheralManager::getInstance().isSPIEnabled(options.hwBlock));
}

void GPCommsSPIBroadcastAddon::setup() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	spi = PeripheralManager::getInstance().getSPI(options.hwBlock);
	spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
	spi->endTransaction();
}

void GPCommsSPIBroadcastAddon::process() {
	static uint64_t nextRunTime = 0;

	uint64_t now = getMicro();
	if (spi->isWriteable() && now - nextRunTime > 0) {
		Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
		sendState(gamepad);
		nextRunTime = now + 1000;
	}
}

/************************
 * SPI send functions
 ************************/

void GPCommsSPIBroadcastAddon::sendStatus(Gamepad *gamepad) {
	const AddonOptions &addonOptions = Storage::getInstance().getAddonOptions();
	uint16_t size = GPComms::fillBufferStatus(buf, gamepad, addonOptions);
	spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
	spi->select();
	spi->write(buf, size);
	spi->deselect();
	spi->endTransaction();
}

void GPCommsSPIBroadcastAddon::sendState(Gamepad *gamepad) {
	uint16_t size = GPComms::fillBufferState(buf, gamepad);
	spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
	spi->select();
	spi->write(buf, size);
	spi->deselect();
	spi->endTransaction();
}

void GPCommsSPIBroadcastAddon::sendMessage(char *text, uint16_t length) {
	uint16_t size = GPComms::fillBufferMessage(buf, text, length);
	spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
	spi->select();
	spi->write(buf, size);
	spi->deselect();
	spi->endTransaction();
}
