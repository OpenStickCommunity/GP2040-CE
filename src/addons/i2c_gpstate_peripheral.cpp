#include "i2c_gpstate_peripheral.h"
#include "i2c_gpcommand.h"
#include "storagemanager.h"
#include <pico/i2c_slave.h>

static uint8_t buf[256] = {0};
static uint64_t micros = 0;

bool I2CGPStatePeripheralAddon::available() {
	const I2CGPStatePeripheralOptions& options = Storage::getInstance().getAddonOptions().i2cGPStatePeripheralOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void I2CGPStatePeripheralAddon::setup() {
	const I2CGPStatePeripheralOptions& options = Storage::getInstance().getAddonOptions().i2cGPStatePeripheralOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_SLAVE_ID;
}

void I2CGPStatePeripheralAddon::process() {
	// Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	// GPState gpState = {
	// 	.gamepadState = gamepad->state,
	// 	.gpioState = gamepad->debouncedGpio,
	// };

	// i2c->write(addr, (uint8_t *)&gpState, sizeof(GPState), false);
}

/************************
 * I2C receive functions
 ************************/

static void handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	static int32_t receivedIndex = 0;

	switch (event) {
		case I2C_SLAVE_RECEIVE: // master has written some data
			buf[receivedIndex++] = i2c_read_byte_raw(i2c);
			break;
		case I2C_SLAVE_REQUEST: // master is requesting data load from memory
			// TODO: Do something useful?!?
			break;
		case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
			{
				uint64_t diff = to_us_since_boot(get_absolute_time()) - micros;
				printf("MICROS: %llu, BYTES: %i, DATA: %X\n", diff, receivedIndex, *buf);
				I2C_GPCommand cmd = (I2C_GPCommand)buf[0];
				switch (cmd) {
					case GPCMD_STATUS:
						break;
					case GPCMD_STATE:
						break;
					case GPCMD_MESSAGE:
						break;
					case GPCMD_ACK:
						break;
					case GPCMD_UNKNOWN:
					default:
						break;
				}
				receivedIndex = 0;
				memcpy(buf, 0, 256);
			}
			break;
		default:
			break;
	}
}

void I2CGPStatePeripheralAddon::handleGamepadStatus(I2C_GPStatus gpStatus) {

}

void I2CGPStatePeripheralAddon::handleGamepadState(I2C_GPState gpState) {

}

void I2CGPStatePeripheralAddon::handleGamepadMessage(I2C_GPMessage gpMessage) {

}
