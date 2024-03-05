#include "i2c_gpstate.h"
#include "storagemanager.h"

bool I2CGPStateAddon::available() {
	const I2CGPStateOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void I2CGPStateAddon::setup() {
	const I2CGPStateOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_GPSTATE_ADDRESS;
}

void I2CGPStateAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	GPState gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	i2c->write(addr, (uint8_t *)&gpState, sizeof(GPState), false);
}
