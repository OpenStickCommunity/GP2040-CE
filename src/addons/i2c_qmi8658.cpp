#include "addons/i2c_qmi8658.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

bool I2CQMI8658Input::available() {
	const QMI8658Options& options = Storage::getInstance().getAddonOptions().qmi8658Options;
	if (options.enabled) {
		PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice({QMI8658_DEVICE_ADDR_L, QMI8658_DEVICE_ADDR_H});
		if (result.address > -1) {
			i2c = PeripheralManager::getInstance().getI2C(result.block);
			return true;
		}
	}
	return false;
}

static PeripheralI2C *QMI8658Input_I2C;
static int16_t QMI8658Input_I2C_Write(uint8_t address, uint8_t *data, uint16_t len, bool isBlock) {
	return QMI8658Input_I2C->write(address, data, len, isBlock);
}
static int16_t QMI8658Input_I2C_ReadRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
	return QMI8658Input_I2C->readRegister(address, reg, data, len);
}

void I2CQMI8658Input::setup() {
	uIntervalMS = 1;
	nextTimer = getMillis();

	QMI8658Input_I2C = i2c;
	QMI8658_I2C_Write = &QMI8658Input_I2C_Write;
	QMI8658_I2C_ReadRegister = &QMI8658Input_I2C_ReadRegister;
	if (QMI8658_init()) {
		Gamepad * gamepad = Storage::getInstance().GetGamepad();
		gamepad->auxState.sensors.accelerometer.enabled = true;
		gamepad->auxState.sensors.gyroscope.enabled = true;
	}
}

void I2CQMI8658Input::process() {
	if (nextTimer < getMillis()) {
		unsigned int tim_count = 0;
		QMI8658_read_xyz_raw(acc, gyro, &tim_count);
		nextTimer = getMillis() + uIntervalMS;
	}

	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	gamepad->auxState.sensors.accelerometer.x = acc[0];
	gamepad->auxState.sensors.accelerometer.y = acc[1];
	gamepad->auxState.sensors.accelerometer.z = acc[2];
	gamepad->auxState.sensors.accelerometer.active = true;

	gamepad->auxState.sensors.gyroscope.x = gyro[0];
	gamepad->auxState.sensors.gyroscope.y = gyro[1];
	gamepad->auxState.sensors.gyroscope.z = gyro[2];
	gamepad->auxState.sensors.gyroscope.active = true;
}
