#ifndef _I2CMPU6050_H
#define _I2CMPU6050_H

#include <MPU6050.h>

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef I2C_MPU6050_ENABLED
#define I2C_MPU6050_ENABLED 0
#endif

#ifndef I2C_MPU6050_SPEED
#define I2C_MPU6050_SPEED 400000
#endif

#ifndef I2C_MPU6050_ADDRESS
#define I2C_MPU6050_ADDRESS 0x68
#endif

#ifndef I2C_MPU6050_SDA_PIN
#define I2C_MPU6050_SDA_PIN -1
#endif

#ifndef I2C_MPU6050_SCL_PIN
#define I2C_MPU6050_SCL_PIN -1
#endif

#ifndef I2C_MPU6050_BLOCK
#define I2C_MPU6050_BLOCK i2c0
#endif

#ifndef I2C_MPU6050_SPEED
#define I2C_MPU6050_SPEED 400000
#endif

#ifndef I2C_MPU6050_ADDRESS
#define I2C_MPU6050_ADDRESS 0x68
#endif

// IMU Module Name
#define I2CMPU6050Name "I2CMPU6050"


class I2CMPU6050Input : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // IMU Setup
    virtual void preprocess() {}
    virtual void process();     // IMU Process
    virtual std::string name() { return I2CMPU6050Name; }
private:
    MPU6050 * imu;
};

#endif // _I2CMPU6050_H_
