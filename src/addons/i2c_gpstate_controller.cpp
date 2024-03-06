#include "i2c_gpstate_controller.h"
#include "storagemanager.h"
#include "i2c_gpcommand.h"

bool I2CGPStateControllerAddon::available() {
	const I2CGPStateControllerOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateControllerOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void I2CGPStateControllerAddon::setup() {
	const I2CGPStateControllerOptions& options = Storage::getInstance().getAddonOptions().i2cGPStateControllerOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_DEFAULT_SLAVE_ADDR;
}

void I2CGPStateControllerAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	sendGamepadState(gamepad);
}

/************************
 * I2C send functions
 ************************/

void I2CGPStateControllerAddon::sendGamepadStatus(Gamepad *gamepad) {
	GamepadOptions options = gamepad->getOptions();
	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();

	I2C_GPStatus gpStatus = {
		.inputMode = options.inputMode,
		.turboRate = static_cast<int8_t>(addonOptions.turboOptions.shotCount),
		.macroEnabled = addonOptions.macroOptions.enabled,
		.dpadMode = options.dpadMode,
		.socdMode = options.socdMode,
	};

	buf[0] = GPCMD_STATUS;
	memcpy(&buf[1], &gpStatus, sizeof(I2C_GPStatus));
	i2c->write(addr, buf, sizeof(I2C_GPStatus) + 1, false);
	memset(buf, 0, I2C_GPSTATE_BUFFER_SIZE);
}

void I2CGPStateControllerAddon::sendGamepadState(Gamepad *gamepad) {
	I2C_GPState gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(I2C_GPState));
	i2c->write(addr, buf, sizeof(I2C_GPState) + 1, false);
	memset(buf, 0, I2C_GPSTATE_BUFFER_SIZE);
}

void I2CGPStateControllerAddon::sendGamepadMessage(Gamepad *gamepad, char *text, uint16_t length) {
	I2C_GPMessage gpMessage = {
		.length = length,
		.message = text,
	};

	buf[0] = GPCMD_MESSAGE;
	memcpy(&buf[1], &gpMessage.length, 2);
	memcpy(&buf[3], text, length);
	i2c->write(addr, buf, gpMessage.length + length + 1, false);
	memset(buf, 0, I2C_GPSTATE_BUFFER_SIZE);
}
