#include "addons/i2cmpu6050.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"
#include "enums.pb.h"
#include <math.h>

bool I2CMPU6050Input::available() {
    const MPU6050Options& option = Storage::getInstance().getAddonOptions().mpu6050Options;
    return (option.enabled && isValidPin(option.i2cSDAPin));
}

void I2CMPU6050Input::setup() {
    const MPU6050Options& options = Storage::getInstance().getAddonOptions().mpu6050Options;
    int i2cSDAPin = options.i2cSDAPin;
    
    imu = new MPU6050(1, i2cSDAPin, 15, i2c1, 40000, I2C_MPU6050_ADDRESS);
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

