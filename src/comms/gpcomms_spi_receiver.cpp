#include "comms/gpcomms_spi_receiver.h"
#include "storagemanager.h"

bool GPCommsSPIReceiverAddon::available() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_SPI_RECEIVE && PeripheralManager::getInstance().isSPIEnabled(options.hwBlock));
}

void GPCommsSPIReceiverAddon::setup() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	spi = PeripheralManager::getInstance().getSPI(options.hwBlock);
	spi->setSlave(true);
}

void GPCommsSPIReceiverAddon::process() {
	static uint8_t buf[GPCOMMS_BUFFER_SIZE];

	while (!spi->isReadable())
		tight_loop_contents();

	int size = spi->read(buf, GPCOMMS_BUFFER_SIZE);
	if (size > 0) {
		GPComms::handleBuffer(buf, size);
		memset(buf, 0, size);
	}

	GPComms::readGamepad();
}
