#include "comms/gpcomms_i2c_broadcast.h"
#include "storagemanager.h"

bool GPCommsI2CBroadcastAddon::available() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_I2C_BROADCAST && PeripheralManager::getInstance().isI2CEnabled(options.hwBlock));
}

void GPCommsI2CBroadcastAddon::setup() {
	const GPCommsOptions& options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	i2c = PeripheralManager::getInstance().getI2C(options.hwBlock);
	addr = I2C_DEFAULT_SLAVE_ADDR;
}

void GPCommsI2CBroadcastAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	sendState(gamepad);
}

/************************
 * I2C send functions
 ************************/

void GPCommsI2CBroadcastAddon::sendStatus(Gamepad *gamepad) {
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

void GPCommsI2CBroadcastAddon::sendState(Gamepad *gamepad) {
	GPComms_State gpState = {
		.gamepadState = gamepad->state,
		.gpioState = gamepad->debouncedGpio,
	};

	buf[0] = GPCMD_STATE;
	memcpy(&buf[1], &gpState, sizeof(GPComms_State));
	i2c->write(addr, buf, sizeof(GPComms_State) + 1, false);
	memset(buf, 0, GPCOMMS_I2C_BUFFER_SIZE);
}

void GPCommsI2CBroadcastAddon::sendMessage(char *text, uint16_t length) {
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
