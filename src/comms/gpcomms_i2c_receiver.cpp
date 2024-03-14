#include "comms/gpcomms_i2c_receiver.h"
#include "storagemanager.h"
#include <pico/i2c_slave.h>

bool GPCommsI2CReceiverAddon::available() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_I2C_RECEIVER && PeripheralManager::getInstance().isI2CEnabled(options.hwBlock));
}

void GPCommsI2CReceiverAddon::setup() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	addr = I2C_DEFAULT_PERIPHERAL_ADDR;
	i2c = PeripheralManager::getInstance().getI2C(options.hwBlock);
	i2c->setAsPeripheral(handler, addr);
}

void GPCommsI2CReceiverAddon::handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	static uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
	static uint8_t receivedIndex = 0;

	switch (event) {
		case I2C_SLAVE_RECEIVE:
			buf[receivedIndex] = i2c_read_byte_raw(i2c);
			receivedIndex++;
			break;

		case I2C_SLAVE_FINISH:
			GPComms::handleBuffer(buf, receivedIndex);
			receivedIndex = 0;
			break;

		case I2C_SLAVE_REQUEST:
		default:
			break;
	}
}
