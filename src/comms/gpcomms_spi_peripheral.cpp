#include "comms/gpcomms_spi_peripheral.h"
#include "storagemanager.h"

static uint64_t micros = 0;

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
		// Grab command byte and shift buffer values
		uint8_t command = buf[0];
		for (int i = 0; i < size - 1; i++)
			buf[i] = buf[i + 1];

		switch (command) {
			case GPCMD_STATUS:
				handleGamepadStatus(reinterpret_cast<GPComms_Status *>(buf));
				break;
			case GPCMD_STATE:
				handleGamepadState(reinterpret_cast<GPComms_State *>(buf));
				break;
			case GPCMD_MESSAGE:
				handleGamepadMessage(reinterpret_cast<GPComms_Message *>(buf));
				break;
			case GPCMD_ACK:
				break;
			case GPCMD_UNKNOWN:
			default:
				break;
		}

		memset(buf, 0, size);
	}
}

/************************
 * SPI receive functions
 ************************/

void GPCommsSPIPeripheralAddon::handleGamepadStatus(GPComms_Status *gpStatus) {

}

void GPCommsSPIPeripheralAddon::handleGamepadState(GPComms_State *gpState) {

}

void GPCommsSPIPeripheralAddon::handleGamepadMessage(GPComms_Message *gpMessage) {

}
