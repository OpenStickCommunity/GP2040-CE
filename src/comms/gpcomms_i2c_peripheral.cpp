#include "comms/gpcomms_i2c_peripheral.h"
#include "storagemanager.h"
#include <pico/i2c_slave.h>

bool GPCommsI2CPeripheralAddon::available() {
	const GPCommsI2CPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CPeripheralOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void GPCommsI2CPeripheralAddon::setup() {
	const GPCommsI2CPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CPeripheralOptions;
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_DEFAULT_SLAVE_ADDR;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	i2c->setSlave(handler, addr);
}

void GPCommsI2CPeripheralAddon::process() {
	GPComms::updateGamepad();
}

void GPCommsI2CPeripheralAddon::handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	static uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
	static int32_t receivedIndex = 0;

	switch (event) {
		case I2C_SLAVE_RECEIVE:
			buf[receivedIndex] = i2c_read_byte_raw(i2c);
			receivedIndex++;
			break;

		case I2C_SLAVE_FINISH:
			GPComms::handleBuffer(buf, receivedIndex);
			memcpy(buf, 0, receivedIndex);
			receivedIndex = 0;
			break;

		case I2C_SLAVE_REQUEST:
		default:
			break;
	}
}
