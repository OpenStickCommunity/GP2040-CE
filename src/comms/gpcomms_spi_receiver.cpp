#include "comms/gpcomms_spi_receiver.h"
#include "storagemanager.h"

bool GPCommsSPIReceiverAddon::available() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_SPI_RECEIVER && PeripheralManager::getInstance().isSPIEnabled(options.hwBlock));
}

void GPCommsSPIReceiverAddon::setup() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	spi = PeripheralManager::getInstance().getSPI(options.hwBlock);
	spi->setAsPeripheral(true);
	spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
	spi->endTransaction();
}

void GPCommsSPIReceiverAddon::process() {
	static uint8_t buf[GPCOMMS_BUFFER_SIZE];

	if (spi->isReadable()) {
		spi->beginTransaction(SPI_DEFAULT_SPEED, SPI_MSB_FIRST, SPI_MODE0);
		int size = spi->read(buf, GPCOMMS_BUFFER_SIZE);
		spi->endTransaction();
		if (size > 0) {
			GPComms::handleBuffer(buf, size);
		}
	}
}
