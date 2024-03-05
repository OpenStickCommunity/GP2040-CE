#include "i2c_gpstate_controller.h"
#include "storagemanager.h"

bool I2CGPStateControllerAddon::available() {
	const I2CGPStateControllerOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateControllerOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void I2CGPStateControllerAddon::setup() {
	const I2CGPStateControllerOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateControllerOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_SLAVE_ID;
}

void I2CGPStateControllerAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	GPState gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	i2c->write(addr, (uint8_t *)&gpState, sizeof(GPState), false);
}
