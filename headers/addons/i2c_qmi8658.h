#ifndef _I2C_QMI8658_H
#define _I2C_QMI8658_H

extern "C" {
#include "QMI8658.h"
}

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "peripheralmanager.h"

#ifndef I2C_QMI8658_ENABLED
#define I2C_QMI8658_ENABLED 0
#endif

// Analog Module Name
#define I2C_QMI8658_Name "I2C_QMI8658"

class I2CQMI8658Input : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual void postprocess(bool sent) {}
	virtual void reinit() {}
	virtual std::string name() { return I2C_QMI8658_Name; }
private:
	PeripheralI2C *i2c;
	uint32_t uIntervalMS;
	uint32_t nextTimer;
	int16_t acc[3];
	int16_t gyro[3];
};

#endif  // _I2C_QMI8658_H_
