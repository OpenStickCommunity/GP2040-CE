/* *****************************************************************************
 * Filename: Mpu6050.h
 *
 * Description: This is file contains the definition of the Mpu6050 class, that
 * is used to configure and read data from the eponymous sensor.
 * Each instance of this class represents a different sensor that has an I2C
 * address 0x68 or 0x69 if the AD0 pin is connected to logic 1 (3.3V).
 * The Mpu6050Data struct is also defined here and has 3 member variables for
 * each type of measurement (accel,gyro,temp).
 * A set of useful enumerations is defined for configuring the sensor in a more
 * readable way.
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

/* TODO:
    - On-demand gyro cal
    - Automatic cal (if no motion for 5 seconds, calibrate gyro. Figure out thresholds)
    - Add support for alternative orientations
*/

#ifndef MPU6050_H
#define MPU6050_H

#include <BitBang_I2C.h>
#include "Vector3f.h"

struct Mpu6050Data
{
    public:
        Vector3f acceleration; // G
        Vector3f gyroscope; // degree/s
        float temperature; // Celsius degrees
};

// Used to select the gyroscope measurement range.
// Always select the smallest possible range for precision.
enum Mpu6050GyroscopeRange
{
    Max250Dps, // +/- 250 degree per second of rotational speed
    Max500Dps,
    Max1000Dps,
    Max2000Dps
};

// Used to select the accelerometer measurement range.
// Always select the smallest possible range for precision.
enum Mpu6050AccelerometerRange
{
    Max2g, // +/- 2g
    Max4g,
    Max8g,
    Max16g
};

// Used to configure the internal Digital Low Pass Filter (DLPF)
// The DLPF will average the measurement which is useful for slow varying
// motion, as it removes high frequency components (such as vibrations)
enum Mpu6050DLPFBandwidth
{
    Max260Hz, // 260 measurements per second
    Max184Hz,
    Max94Hz,
    Max44Hz,
    Max21Hz,
    Max10Hz,
    Max5Hz
};

class MPU6050
{
    public:
        // Specify AD0 pin level and I2C bus when instantiating a device
        MPU6050(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t speed, uint8_t addr);

        // Initialize the device before reading data
        bool init(Mpu6050AccelerometerRange accelRange = Max16g,
                  Mpu6050GyroscopeRange gyroRange = Max2000Dps,
                  Mpu6050DLPFBandwidth bandwidth = Max260Hz,
                  uint8_t SampleRateDivider = 7);

        void calibrateGyro();

        // Methods to read measurements
        Mpu6050Data readData();
        void readRawAcceleration(int16_t &rawAccelX, int16_t &rawAccelY, int16_t &rawAccelZ);
        void readRawGyroscope(int16_t &rawGyroX, int16_t &rawGyroY, int16_t &rawGyroZ);
        Vector3f readAcceleration();
        Vector3f readGyroscope();
        float readTemperature();

        // Configuration of the sensors
        void setAccelerometerRange(Mpu6050AccelerometerRange range);
        void setGyroscopeRange(Mpu6050GyroscopeRange range);
        void setDLPFBandwidth(Mpu6050DLPFBandwidth bandwidth);
        void setSampleRateDivider(uint8_t divider); // sample rate = 8kHz / (1 + divider)

        Mpu6050AccelerometerRange getAccelerometerRange();
        Mpu6050GyroscopeRange getGyroscopeRange();
        Mpu6050DLPFBandwidth getDLPFBandwidth();
        uint8_t getSampleRateDivider();

        // Resets the registers to their default value (init() should be called
        // after a reset to use again the device)
        void reset();

        // Return true if the connection is established with the MPU6050
        bool isConnected();

        // Set sensor in sleep mode (power saving mode, no data measurements)
        void sleepMode();
        int wakeUp();

        void set_ad0(bool ad0);
        bool get_ad0();


    private:
        uint8_t address;
	    int32_t iSpeed;
        Mpu6050AccelerometerRange m_accelerometerRange;
        Mpu6050GyroscopeRange m_gyroscopeRange;

        BBI2C bbi2c;

        int16_t gyroOffsetX = 0;
        int16_t gyroOffsetY = 0;
        int16_t gyroOffsetZ = 0;

        // Raw data to real-world units
        float rawTemperatureToCelsius(int16_t rawTemperature);
        float rawGyroscopeToDps(int16_t rawGyroscope); // degree/s
        float rawAccelerationToG(int16_t rawAcceleration); // m/s^2

        // For convenience:
        uint8_t read8(uint8_t registerAddr);
        int16_t read16(uint8_t registerAddr);
        int write8(uint8_t registerAddr, uint8_t value);

        // Buffer for I2C communication
	    unsigned char uc[16];
};

#endif // MPU6050_H
