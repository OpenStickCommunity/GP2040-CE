#include "addons/i2cmpu6050.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "enums.pb.h"
#include <math.h>

bool I2CMPU6050Input::available() {
    const MPU6050Options& options = Storage::getInstance().getAddonOptions().mpu6050Options;
    if (!options.enabled) {
        return false;
    }
    // Double check pin ranges
    if (!isValidPin(options.i2cSDAPin) || !isValidPin(options.i2cSCLPin)){
        return false;
    }

    // Make sure SDA/SCL are on the assigned i2c block
    if ((options.i2cSDAPin + 2 * i2c_hw_index(options.i2cBlock == 0 ? i2c0 : i2c1))%4 != 0) {
        return false;
    }

    if ((options.i2cSCLPin + 3 + 2 * i2c_hw_index(options.i2cBlock == 0 ? i2c0 : i2c1))%4 != 0) {
        return false;
    }

    // Make sure speed is in range
    if (options.i2cSpeed < 100000 || options.i2cSpeed > 400000) {
        return false;
    }

    // Make sure address is valid for MPU6050 (default is 0x68, pull AD0 high for 0x69)
    if (options.i2cAddress != 0x68 && options.i2cAddress != 0x69) {
        return false;
    }
    
    return true;
}

void I2CMPU6050Input::setup() {
    const MPU6050Options& options = Storage::getInstance().getAddonOptions().mpu6050Options;
    
    imu = new MPU6050(1,
        options.i2cSDAPin,
        options.i2cSCLPin,
        options.i2cBlock == 0 ? i2c0 : i2c1,
        options.i2cSpeed,
        options.i2cAddress);
    imu->init();
}

void I2CMPU6050Input::process() {
    Vector3f angular = imu->readGyroscope(); // deg/sec
    Vector3f accel = imu->readAcceleration(); // G
    
    Gamepad* gamepad = Storage::getInstance().GetGamepad();

    // Convert from MPU6050 orientation to PS4
    // PS4 report orientation: x right, y up, z backward
    // MPU6050 orientation (top pcb, dot facing NW): x right, y forward, z up
    gamepad->state.accelX = accel.x;
    gamepad->state.gyroX = angular.x;

    gamepad->state.accelY = accel.z;
    gamepad->state.gyroY = angular.z;
    
    gamepad->state.accelZ = -accel.y;
    gamepad->state.gyroZ = -angular.y;

    return;
}

