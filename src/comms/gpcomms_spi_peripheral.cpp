#include "comms/gpcomms_spi_peripheral.h"
#include "storagemanager.h"

bool GPCommsSPIPeripheralAddon::available() {
	const GPCommsSPIPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsSPIPeripheralOptions;
	return (options.enabled && PeripheralManager::getInstance().isSPIEnabled(options.spiBlock));
}

void GPCommsSPIPeripheralAddon::setup() {
	const GPCommsSPIPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsSPIPeripheralOptions;
	spi = PeripheralManager::getInstance().getSPI(options.spiBlock);
	spi->setSlave(true);
}

void GPCommsSPIPeripheralAddon::process() {
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
