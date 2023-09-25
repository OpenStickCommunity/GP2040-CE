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
    // TODO: Remove Vector3f, plain floats instead
    Vector3f angular = imu->readGyroscope(); // deg/sec
    Vector3f accel = imu->readAcceleration(); // G
    

    // Flip X and Z if mounted on back of PCB (rotate around Y axis 180deg)
    bool upsidedown = false;
    if (upsidedown) {
        accel.x = -accel.x;
        angular.x = -angular.x;
        accel.z = -accel.z;
        angular.z = -angular.z;
    }
    
    // TODO: Make orientation an enum
    // TODO: Make configurable in UI
    
    // Convert MPU6050 axes to PS4 axes, depending on orientation
    // PS4: x right, y up, z towards user.
    // Default MPU6050 orientation: x right, y away from user, z up
    Gamepad* gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.accelY = accel.z;
    gamepad->state.gyroY = angular.z;
    uint8_t orientation = 0;
    switch (orientation) {
        case 0: // 0 deg
            gamepad->state.accelX = accel.x;
            gamepad->state.gyroX = angular.x;
            gamepad->state.accelZ = -accel.y;
            gamepad->state.gyroZ = -angular.y;
            break;
        case 1: // 90 deg counterclockwise
            gamepad->state.accelX = -accel.y;
            gamepad->state.gyroX = -angular.y;
            gamepad->state.accelZ = -accel.x;
            gamepad->state.gyroZ = -angular.x;
            break;
        case 2: // 180 deg
            gamepad->state.accelX = -accel.x;
            gamepad->state.gyroX = -angular.x;
            gamepad->state.accelZ = accel.y;
            gamepad->state.gyroZ = angular.y;
            break;
        case 3: // 270 deg counterclockwise
            gamepad->state.accelX = accel.y;
            gamepad->state.gyroX = angular.y;
            gamepad->state.accelZ = accel.x;
            gamepad->state.gyroZ = angular.x;
            break;
    }

    return;
}

