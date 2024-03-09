#include "comms/gpcomms_i2c_controller.h"
#include "storagemanager.h"

bool GPCommsI2CControllerAddon::available() {
	const GPCommsI2CControllerOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CControllerOptions;
	return (options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock));
}

void GPCommsI2CControllerAddon::setup() {
	const GPCommsI2CControllerOptions& options = Storage::getInstance().getAddonOptions().gpCommsI2CControllerOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	addr = options.i2cAddress > 0 ? options.i2cAddress : I2C_DEFAULT_SLAVE_ADDR;
}

void GPCommsI2CControllerAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	sendState(gamepad);
}

/************************
 * I2C send functions
 ************************/

void GPCommsI2CControllerAddon::sendStatus(Gamepad *gamepad) {
	GamepadOptions options = gamepad->getOptions();
	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();

	GPComms_Status gpStatus = {
		.inputMode = options.inputMode,
		.turboRate = static_cast<int8_t>(addonOptions.turboOptions.shotCount),
		.macroEnabled = addonOptions.macroOptions.enabled,
		.dpadMode = options.dpadMode,
		.socdMode = options.socdMode,
	};

	buf[0] = GPCMD_STATUS;
	memcpy(&buf[1], &gpStatus, sizeof(GPComms_Status));
	i2c->write(addr, buf, sizeof(GPComms_Status) + 1, false);
	memset(buf, 0, GPCOMMS_I2C_BUFFER_SIZE);
}

void GPCommsI2CControllerAddon::sendState(Gamepad *gamepad) {
	GPComms_State gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(GPComms_State));
	i2c->write(addr, buf, sizeof(GPComms_State) + 1, false);
	memset(buf, 0, GPCOMMS_I2C_BUFFER_SIZE);
}

void GPCommsI2CControllerAddon::sendMessage(char *text, uint16_t length) {
	GPComms_Message gpMessage = {
		.length = length,
		.message = text,
	};

	buf[0] = GPCMD_MESSAGE;
	memcpy(&buf[1], &gpMessage.length, 2);
	memcpy(&buf[3], text, length);
	i2c->write(addr, buf, gpMessage.length + length + 1, false);
	memset(buf, 0, GPCOMMS_I2C_BUFFER_SIZE);
}
