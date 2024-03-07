#include "comms/gpcomms_i2c_peripheral.h"
#include "storagemanager.h"
#include <pico/i2c_slave.h>

static uint64_t micros = 0;

bool GPCommsI2CPeripheralAddon::available() {
	const GPCommsI2CPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CPeripheralOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void GPCommsI2CPeripheralAddon::setup() {
	const GPCommsI2CPeripheralOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CPeripheralOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_DEFAULT_SLAVE_ADDR;
}

void GPCommsI2CPeripheralAddon::process() {
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

void GPCommsI2CPeripheralAddon::handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
	static uint8_t buf[256] = {0};
	static int32_t receivedIndex = 0;

	switch (event) {
		case I2C_SLAVE_RECEIVE: // master has written some data
			buf[receivedIndex] = i2c_read_byte_raw(i2c);
			receivedIndex++;
			break;
		case I2C_SLAVE_REQUEST: // master is requesting data load from memory
			// TODO: Do something useful?!?
			break;
		case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
			{
				// uint64_t diff = to_us_since_boot(get_absolute_time()) - micros;
				// printf("MICROS: %llu, BYTES: %i, DATA: %X\n", diff, receivedIndex, *buf);

				// Grab command byte and shift buffer values
				uint8_t command = buf[0];
				for (int i = 0; i < receivedIndex; i++)
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
				receivedIndex = 0;
				memcpy(buf, 0, 256);
			}
			break;
		default:
			break;
	}
}

void GPCommsI2CPeripheralAddon::handleGamepadStatus(GPComms_Status *gpStatus) {

}

void GPCommsI2CPeripheralAddon::handleGamepadState(GPComms_State *gpState) {

}

void GPCommsI2CPeripheralAddon::handleGamepadMessage(GPComms_Message *gpMessage) {

}
