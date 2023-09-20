/* *****************************************************************************
 * Filename: Mpu6050.cpp
 *
 * Description: This is file contains the implementation of the Mpu6050 class.
 * All the information you need to understand how to communicate and setup the
 * chip can be found in the datasheet and register map from Invensense:
 * https://www.invensense.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
 * https://www.invensense.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
 *
 * Author: Thomas Havy
 *
 * License: MIT License - Copyright (c) 2017 Thomas Havy
 * A copy of the license can be found at:
 * https://github.com/Th-Havy/Simple-MPU6050-Arduino/blob/master/LICENSE
 *
 * Changes:
 * - Created: 07-Sept-2017
 *
 * ************************************************************************* */

/* Ported for GP2040 by HoloPengin 2023 */

#include "MPU6050.h"

// Selected Mpu6050 register addresses (found in the invensense datasheet)
#define MPU6050_REGISTER_SMPRT_DIV 0x19 // Sample rate divider
#define MPU6050_REGISTER_CONFIG 0x1A // DLPF config
#define MPU6050_REGISTER_GYRO_CONFIG 0x1B
#define MPU6050_REGISTER_ACCEL_CONFIG 0x1C
#define MPU6050_REGISTER_ACCEL_XOUT_H 0x3B // Accelerometer measurement
#define MPU6050_REGISTER_ACCEL_XOUT_L 0x3C
#define MPU6050_REGISTER_ACCEL_YOUT_H 0x3D
#define MPU6050_REGISTER_ACCEL_YOUT_L 0x3E
#define MPU6050_REGISTER_ACCEL_ZOUT_H 0x3F
#define MPU6050_REGISTER_ACCEL_ZOUT_L 0x40
#define MPU6050_REGISTER_TEMP_OUT_H 0x41 // Temperature measurement
#define MPU6050_REGISTER_TEMP_OUT_L 0x42
#define MPU6050_REGISTER_GYRO_XOUT_H 0x43 // Gyroscope measurement
#define MPU6050_REGISTER_GYRO_XOUT_L 0x44
#define MPU6050_REGISTER_GYRO_YOUT_H 0x45
#define MPU6050_REGISTER_GYRO_YOUT_L 0x46
#define MPU6050_REGISTER_GYRO_ZOUT_H 0x47
#define MPU6050_REGISTER_GYRO_ZOUT_L 0x48
#define MPU6050_REGISTER_PWR_MGMT_1 0x6B // Power management
#define MPU6050_REGISTER_WHO_AM_I 0x75 // Contains address of the device (0x68)

// Default I2C address of the MPU6050 (0x69 if AD0 pin set to HIGH)
#define MPU6050_DEFAULT_ADDRESS 0x68

// Constant to convert raw temperature to Celsius degrees
#define MPU6050_TEMP_LINEAR_COEF (1.0/340.00)
#define MPU6050_TEMP_OFFSET       36.53

// Constant to convert raw gyroscope to degree/s
#define MPU6050_GYRO_FACTOR_250 (1.0/131.0)
#define MPU6050_GYRO_FACTOR_500  (1.0/65.5)
#define MPU6050_GYRO_FACTOR_1000 (1.0/32.8)
#define MPU6050_GYRO_FACTOR_2000 (1.0/16.4)

// Constant to convert raw acceleration to m/s^2
#define GRAVITATIONAL_CONSTANT_G 9.81
#define MPU6050_ACCEL_FACTOR_2 (1.0 / 16384.0)
#define MPU6050_ACCEL_FACTOR_4 (1.0 / 8192.0)
#define MPU6050_ACCEL_FACTOR_8 (1.0 / 4096.0)
#define MPU6050_ACCEL_FACTOR_16 (1.0 / 2048.0)

MPU6050::MPU6050(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t speed, uint8_t addr) :
m_accelerometerRange(Max2g),
m_gyroscopeRange(Max250Dps)
{
    bbi2c.iSDA = sda;
    bbi2c.iSCL = scl;
    bbi2c.picoI2C = picoI2C;
    bbi2c.bWire = bWire;
    iSpeed = speed;
    address = addr;
}

bool MPU6050::init(Mpu6050AccelerometerRange accelRange,
                   Mpu6050GyroscopeRange gyroRange,
                   Mpu6050DLPFBandwidth bandwidth,
                   uint8_t SampleRateDivider)
{
    // Pull sensor out of sleep mode
    I2CInit(&bbi2c, iSpeed);
    wakeUp();

    // Test connection between the Arduino and the sensor
    if (!isConnected())
    {
        return false;
    }

    // Reduce output rate of the sensor
    setSampleRateDivider(SampleRateDivider);

    setAccelerometerRange(accelRange);
    setGyroscopeRange(gyroRange);
    setDLPFBandwidth(bandwidth);

    return true;
}

Mpu6050Data MPU6050::readData()
{
    Mpu6050Data data;

    data.acceleration = readAcceleration();
    data.gyroscope = readGyroscope();
    data.temperature = readTemperature();

    return data;
}

void MPU6050::readRawAcceleration(int16_t &rawAccelX, int16_t &rawAccelY, int16_t &rawAccelZ)
{
    uc[0] = MPU6050_REGISTER_ACCEL_XOUT_H; // accelX register (followed by y and z)
    I2CWrite(&bbi2c, address, uc, 1);
    I2CRead(&bbi2c, address, uc, 6);

    rawAccelX = uc[0]<<8 | uc[1];
    rawAccelY = uc[2]<<8 | uc[3];
    rawAccelZ = uc[4]<<8 | uc[5];
}

Vector3f MPU6050::readAcceleration()
{
    int16_t rawAccelX, rawAccelY, rawAccelZ;
    readRawAcceleration(rawAccelX, rawAccelY, rawAccelZ);

    // Convert each integer value to physical units
    Vector3f accel = Vector3f();
    accel.x = rawAccelerationToG(rawAccelX);
    accel.y = rawAccelerationToG(rawAccelY);
    accel.z = rawAccelerationToG(rawAccelZ);

    return accel;
}

void MPU6050::readRawGyroscope(int16_t &rawGyroX, int16_t &rawGyroY, int16_t &rawGyroZ)
{
    uc[0] = MPU6050_REGISTER_GYRO_XOUT_H; // Gyro X register (followed by y and z)
    I2CWrite(&bbi2c, address, uc, 1);
    I2CRead(&bbi2c, address, uc, 6);
    
    rawGyroX = uc[0]<<8 | uc[1];
    rawGyroY = uc[2]<<8 | uc[3];
    rawGyroZ = uc[4]<<8 | uc[5];
}

Vector3f MPU6050::readGyroscope()
{
    int16_t rawGyroX, rawGyroY, rawGyroZ;
    readRawGyroscope(rawGyroX, rawGyroY, rawGyroZ);

    // Convert each integer value to physical units
    Vector3f gyro = Vector3f();
    gyro.x = rawGyroscopeToDps(rawGyroX);
    gyro.y = rawGyroscopeToDps(rawGyroY);
    gyro.z = rawGyroscopeToDps(rawGyroZ);

    return gyro;
}

float MPU6050::readTemperature()
{
    int16_t rawTemperature = read16(MPU6050_REGISTER_TEMP_OUT_H);

    return rawTemperatureToCelsius(rawTemperature);
}

void MPU6050::setAccelerometerRange(Mpu6050AccelerometerRange range)
{
    uint8_t accelRange = static_cast<int>(range) << 3;

    uint8_t accelRangeRegister = read8(MPU6050_REGISTER_ACCEL_CONFIG);

    // Change only the range in the register
    accelRangeRegister = (accelRangeRegister & 0b11100111) | accelRange;

    write8(MPU6050_REGISTER_ACCEL_CONFIG, accelRangeRegister);

    m_accelerometerRange = range;
}

void MPU6050::setGyroscopeRange(Mpu6050GyroscopeRange range)
{
    uint8_t gyroRange = static_cast<int>(range) << 3;

    uint8_t gyroRangeRegister = read8(MPU6050_REGISTER_GYRO_CONFIG);

    // Change only the range in the register
    gyroRangeRegister = (gyroRangeRegister & 0b11100111) | gyroRange;

    write8(MPU6050_REGISTER_GYRO_CONFIG, gyroRangeRegister);

    m_gyroscopeRange = range;
}

void MPU6050::setDLPFBandwidth(Mpu6050DLPFBandwidth bandwidth)
{
    uint8_t band = static_cast<uint8_t>(bandwidth);

    uint8_t registerDLPF = read8(MPU6050_REGISTER_CONFIG);

    registerDLPF = (registerDLPF & 0b11111000) | band; // change only bandwidth

    write8(MPU6050_REGISTER_CONFIG, registerDLPF);
}

void MPU6050::setSampleRateDivider(uint8_t divider)
{
    write8(MPU6050_REGISTER_SMPRT_DIV, divider);
}

Mpu6050AccelerometerRange MPU6050::getAccelerometerRange()
{
    return m_accelerometerRange;
}

Mpu6050GyroscopeRange MPU6050::getGyroscopeRange()
{
    return m_gyroscopeRange;
}

Mpu6050DLPFBandwidth MPU6050::getDLPFBandwidth()
{
    uint8_t registerDLPF = read8(MPU6050_REGISTER_CONFIG);
    registerDLPF &= 0b00000111; // Keep only the value of DLPF_CFG

    return static_cast<Mpu6050DLPFBandwidth>(registerDLPF);
}

uint8_t MPU6050::getSampleRateDivider()
{
    return read8(MPU6050_REGISTER_SMPRT_DIV);
}

void MPU6050::reset()
{
    write8(MPU6050_REGISTER_PWR_MGMT_1, 0b10000000);
}

bool MPU6050::isConnected()
{
    // The content of WHO_AM_I is always 0x68, so if the wiring is right and
    // the I2C communication works fine this function should return true
    return read8(MPU6050_REGISTER_WHO_AM_I) == MPU6050_DEFAULT_ADDRESS;
}

void MPU6050::sleepMode()
{
    write8(MPU6050_REGISTER_PWR_MGMT_1, 0b01000000);
}

void MPU6050::wakeUp()
{
    write8(MPU6050_REGISTER_PWR_MGMT_1, 0b00000000);
}

void MPU6050::set_ad0(bool ad0)
{
    address = MPU6050_DEFAULT_ADDRESS + ad0;
}

bool MPU6050::get_ad0()
{
    if (address == MPU6050_DEFAULT_ADDRESS)
    {
        return false;
    }
    else
    {
        return true;
    }
}

float MPU6050::rawTemperatureToCelsius(int16_t rawTemperature)
{
    return rawTemperature * MPU6050_TEMP_LINEAR_COEF + MPU6050_TEMP_OFFSET;
}

float MPU6050::rawGyroscopeToDps(int16_t rawGyroscope)
{
    switch (m_gyroscopeRange)
    {
        case Max250Dps:
            return rawGyroscope * MPU6050_GYRO_FACTOR_250;
            break;
        case Max500Dps:
            return rawGyroscope * MPU6050_GYRO_FACTOR_500;
            break;
        case Max1000Dps:
            return rawGyroscope * MPU6050_GYRO_FACTOR_1000;
            break;
        case Max2000Dps:
            return rawGyroscope * MPU6050_GYRO_FACTOR_2000;
            break;
        default:
            break;
    }
}

float MPU6050::rawAccelerationToG(int16_t rawAcceleration)
{
    switch (m_accelerometerRange)
    {
        case Max2g:
            return rawAcceleration * MPU6050_ACCEL_FACTOR_2;
            break;
        case Max4g:
            return rawAcceleration * MPU6050_ACCEL_FACTOR_4;
            break;
        case Max8g:
            return rawAcceleration * MPU6050_ACCEL_FACTOR_8;
            break;
        case Max16g:
            return rawAcceleration * MPU6050_ACCEL_FACTOR_16;
            break;
        default:
            break;
    }
}

// Read one register
uint8_t MPU6050::read8(uint8_t registerAddr)
{
    uc[0] = registerAddr;
    I2CWrite(&bbi2c, address, uc, 1);
    I2CRead(&bbi2c, address, uc, 1);
    return uc[0];
}

// read a value contained in two consecutive registers
int16_t MPU6050::read16(uint8_t registerAddr)
{
    uc[0] = registerAddr;
    I2CWrite(&bbi2c, address, uc, 1);
    I2CRead(&bbi2c, address, uc, 2);
    int16_t val = uc[0]<<8 | uc[1];
    return val;
}

// write one register 
void MPU6050::write8(uint8_t registerAddr, uint8_t value)
{
    uc[0] = registerAddr;
    uc[1] = value;
    I2CWrite(&bbi2c, address, uc, 2);
}
