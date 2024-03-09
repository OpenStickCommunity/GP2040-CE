#include "comms/gpcomms_i2c_broadcast.h"
#include "storagemanager.h"

bool GPCommsI2CBroadcastAddon::available() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
	return (options.mode == GP_COMMS_MODE_I2C_BROADCAST && PeripheralManager::getInstance().isI2CEnabled(options.hwBlock));
}

void GPCommsI2CBroadcastAddon::setup() {
	const GPCommsOptions &options = Storage::getInstance().getAddonOptions().gpCommsOptions;
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
	const AddonOptions &addonOptions = Storage::getInstance().getAddonOptions();
	uint16_t size = GPComms::fillBufferStatus(buf, gamepad, addonOptions);
	i2c->write(addr, buf, size, false);
}

void GPCommsI2CBroadcastAddon::sendState(Gamepad *gamepad) {
	uint16_t size = GPComms::fillBufferState(buf, gamepad);
	i2c->write(addr, buf, size, false);
}

void GPCommsI2CBroadcastAddon::sendMessage(char *text, uint16_t length) {
	uint16_t size = GPComms::fillBufferMessage(buf, text, length);
	i2c->write(addr, buf, size, false);
}
