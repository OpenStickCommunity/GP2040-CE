#include "i2c_gpstate_peripheral.h"
#include "storagemanager.h"

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
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	GPState gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	i2c->write(addr, (uint8_t *)&gpState, sizeof(GPState), false);
}
