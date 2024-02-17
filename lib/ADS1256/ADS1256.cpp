// ADS1256 cpp file
/*
Name:		ADS1256.cpp
Created:	2022/07/14
Author:	Curious Scientist
Editor:	Notepad++
Comment: Visit https://curiousscientist.tech/blog/ADS1256-custom-library
Special thanks to Abraão Queiroz for spending time on the code and suggesting corrections for ESP32 microcontrollers!

2024-02-15 - Ported to GP2040-CE PicoPeripheral interface
*/

#include "ADS1256.h"
#include <cstdio>
#include <math.h>
#include "hardware/sync.h"
#include "pico.h"
#include "pico/stdlib.h"

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif
#ifndef bitSet
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#endif
#ifndef bitClear
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif
#ifndef bitWrite
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

// A function that waits for the DRDY status to change
void ADS1256::waitForDRDY() {
    while (gpio_get(_DRDY_pin));
}

// Constructor
ADS1256::ADS1256(PeripheralSPI *spi, const int DRDY_pin, const int RESET_pin, const int SYNC_pin, const int CS_pin, float VREF) {
    _SPI = spi;

    _DRDY_pin = DRDY_pin;
    gpio_init(_DRDY_pin);
    gpio_set_dir(_DRDY_pin, GPIO_IN);

    if (RESET_pin > -1) {
        _RESET_pin = RESET_pin;
        gpio_init(_RESET_pin);
        gpio_set_dir(_RESET_pin, GPIO_OUT);
    }

    if (SYNC_pin > -1) {
        _SYNC_pin = SYNC_pin;
        gpio_init(_SYNC_pin);
        gpio_set_dir(_SYNC_pin, GPIO_OUT);
    }

    _CS_pin = CS_pin;
    gpio_init(_CS_pin);
    gpio_set_dir(_CS_pin, GPIO_OUT);
    gpio_pull_up(_CS_pin);

    _VREF = VREF;
}

// Initialization
void ADS1256::init(uint8_t drate, uint8_t pga, bool useBuf) {
    // We do a manual chip reset on the ADS1256 - Datasheet Page 27/ RESET
    if (_RESET_pin > -1) {
        gpio_pull_down(_RESET_pin);
        sleep_ms(200);
        gpio_pull_up(_RESET_pin);
        sleep_ms(1000);
    }

    if (_SYNC_pin > -1) {
        gpio_pull_up(_SYNC_pin);
    }

    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
    _SPI->select(_CS_pin);

    setBuffer(useBuf);
    setAutoCal(1);
    setPGA(pga);
    setDRATE(drate);

    _SPI->transfer(ADS1256_CMD_SELFCAL); // Offset and self-gain calibration
    _SPI->deselect();
    _SPI->endTransaction();

    _isAcquisitionRunning = false; // MCU will be waiting to start a continuous acquisition
}

// Sending SDATAC to stop the continuous conversion
void ADS1256::stopConversion() {
    waitForDRDY();               // SDATAC should be called after DRDY goes LOW (p35. Figure 33)

    _SPI->select(_CS_pin);
    _SPI->transfer(ADS1256_CMD_SDATAC);     // Send SDATAC to the ADC
    _SPI->deselect();            // We finished the command sequence, so we switch it back to HIGH
    _SPI->endTransaction();

    _isAcquisitionRunning = false; // Reset to false, so the MCU will be able to start a new conversion
}

// Setting DRATE (sampling frequency)
void ADS1256::setDRATE(uint8_t drate) {
    writeRegister(ADS1256_REG_DRATE, drate);
    _DRATE = drate;
    sleep_ms(200);
}

// Setting MUX (input channel)
void ADS1256::setMUX(uint8_t mux) {
    writeRegister(ADS1256_REG_MUX, mux);
    _MUX = mux;
    sleep_ms(200);
}

// Setting PGA (input voltage range)
void ADS1256::setPGA(uint8_t pga) {
    _PGA = pga;
    _ADCON = readRegister(ADS1256_REG_ADCON); // Read the most recent value of the register
    _ADCON = (_ADCON & 0b11111000) | (_PGA & 0b00000111); // Clearing and then setting bits 2-0 based on pga
    writeRegister(ADS1256_REG_ADCON, _ADCON);
    sleep_ms(200);
}

// Reading PGA from the ADCON register
uint8_t ADS1256::getPGA() {
    uint8_t pgaValue = 0;
    pgaValue = readRegister(ADS1256_REG_ADCON) & 0b00000111; // Reading the ADS1256_REG_ADCON and keeping the first three bits.
    return (pgaValue);
}

// Setting CLKOUT
void ADS1256::setCLKOUT(uint8_t clkout) {
    _ADCON = readRegister(ADS1256_REG_ADCON); // Read the most recent value of the register

    // Values: 0, 1, 2, 3

    if (clkout == 0) {
        // 00
        bitWrite(_ADCON, 6, 0);
        bitWrite(_ADCON, 5, 0);
    }
    else if (clkout == 1) {
        // 01 (default)
        bitWrite(_ADCON, 6, 0);
        bitWrite(_ADCON, 5, 1);
    }
    else if (clkout == 2) {
        // 10
        bitWrite(_ADCON, 6, 1);
        bitWrite(_ADCON, 5, 0);
    }
    else if (clkout == 3) {
        // 11
        bitWrite(_ADCON, 6, 1);
        bitWrite(_ADCON, 5, 1);
    }

    writeRegister(ADS1256_REG_ADCON, _ADCON);
    sleep_ms(100);
}

// Setting SDCS
void ADS1256::setSDCS(uint8_t sdcs) {
    _ADCON = readRegister(ADS1256_REG_ADCON); // Read the most recent value of the register

    // Values: 0, 1, 2, 3

    if (sdcs == 0) {
        // 00 (default)
        bitWrite(_ADCON, 4, 0);
        bitWrite(_ADCON, 3, 0);
    }
    else if (sdcs == 1) {
        // 01
        bitWrite(_ADCON, 4, 0);
        bitWrite(_ADCON, 3, 1);
    }
    else if (sdcs == 2) {
        // 10
        bitWrite(_ADCON, 4, 1);
        bitWrite(_ADCON, 3, 0);
    }
    else if (sdcs == 3) {
        // 11
        bitWrite(_ADCON, 4, 1);
        bitWrite(_ADCON, 3, 1);
    }

    writeRegister(ADS1256_REG_ADCON, _ADCON);
    sleep_ms(100);
}

// Setting byte order (MSB/LSB)
void ADS1256::setByteOrder(uint8_t byteOrder) {
    _STATUS = readRegister(ADS1256_REG_STATUS); // Read the most recent value of the register

    if (byteOrder == 0) {
        // Byte order is MSB (default)
        bitWrite(_STATUS, 3, 0);
    }
    else if (byteOrder == 1) {
        // Byte order is LSB
        bitWrite(_STATUS, 3, 1);
    }

    writeRegister(ADS1256_REG_STATUS, _STATUS);
    sleep_ms(100);
}

// Getting byte order (MSB/LSB)
void ADS1256::getByteOrder() {
    uint8_t statusValue = readRegister(ADS1256_REG_STATUS); // Read the whole STATUS register

#if ADS1256_DEBUG==true
    // Read bit 1 and print the corresponding message
    if (bitRead(statusValue, 3) == 0) {
        // Byte order is MSB (default)
        printf("Byte order is MSB (default)");
    }
    else {
        // Byte order is LSB
        printf("Byte order is LSB");
    }
#else
    (void)statusValue;
#endif
}

// Setting ACAL (Automatic SYSCAL)
void ADS1256::setAutoCal(uint8_t acal) {
    _STATUS = readRegister(ADS1256_REG_STATUS); // Read the most recent value of the register

    if (acal == 0) {
        // Auto-calibration is disabled (default)
        bitWrite(_STATUS, 2, 0);
    }
    else if (acal == 1) {
        // Auto-calibration is enabled
        bitWrite(_STATUS, 2, 1);
    }

    writeRegister(ADS1256_REG_STATUS, _STATUS);
    sleep_ms(100);
}

// Getting ACAL (Automatic SYSCAL)
void ADS1256::getAutoCal() {
    uint8_t statusValue = readRegister(ADS1256_REG_STATUS); // Read the whole STATUS register

#if ADS1256_DEBUG==true
    // Read bit 1 and print the corresponding message
    if (bitRead(statusValue, 2) == 0) {
        // Auto-calibration is disabled (default)
        printf("ACAL is disabled!");
    }
    else {
        // Auto-calibration is enabled
        printf("ACAL is enabled!");
    }
#else
    (void)statusValue;
#endif
}

// Setting input buffer (Input impedance)
void ADS1256::setBuffer(uint8_t bufen) {
    _STATUS = readRegister(ADS1256_REG_STATUS); // Read the most recent value of the register

    if (bufen == 0) {
        // Analog input buffer is disabled (default)
        //_STATUS |= B00000000;
        bitWrite(_STATUS, 1, 0);
    }
    else if (bufen == 1) {
        // Analog input buffer is enabled (recommended)
        //_STATUS |= B00000010;
        bitWrite(_STATUS, 1, 1);
    }

    writeRegister(ADS1256_REG_STATUS, _STATUS);
    sleep_ms(100);
}

// Getting input buffer (Input impedance)
void ADS1256::getBuffer() {
    uint8_t statusValue = readRegister(ADS1256_REG_STATUS); // Read the whole STATUS register

#if ADS1256_DEBUG==true
    // Read bit 1 and print the corresponding message
    if (bitRead(statusValue, 1) == 0) {
        // Analog input buffer is disabled (default)
        printf("Buffer is disabled!");
    }
    else {
        // Analog input buffer is enabled (recommended)
        printf("Buffer is enabled!");
    }
#else
    (void)statusValue;
#endif
}

// Setting GPIO
void ADS1256::setGPIO(uint8_t dir0, uint8_t dir1, uint8_t dir2, uint8_t dir3) {
    _GPIO = readRegister(ADS1256_REG_IO); // Read the most recent value of the register

    // Default: 11100000 - DEC: 224 - Ref: p32 I/O section
    // Sets D3-D0 as input or output

    // Bit7: DIR3
    bitWrite(_GPIO, 7, dir3 == 1 ? 1 : 0); // D3 is input (default)
    //-----------------------------------------------------
    // Bit6: DIR2
    bitWrite(_GPIO, 6, dir2 == 1 ? 1 : 0); // D2 is input (default)
    //-----------------------------------------------------
    // Bit5: DIR1
    bitWrite(_GPIO, 5, dir1 == 1 ? 1 : 0); // D1 is input (default)
    //-----------------------------------------------------
    // Bit4: DIR0
    bitWrite(_GPIO, 4, dir0 == 1 ? 1 : 0); // D0 is input
    //-----------------------------------------------------

    writeRegister(ADS1256_REG_IO, _GPIO);
    sleep_ms(100);
}

// Writing GPIO
void ADS1256::writeGPIO(uint8_t dir0, uint8_t dir1, uint8_t dir2, uint8_t dir3) {
    _GPIO = readRegister(ADS1256_REG_IO);

    // Sets D3-D0 output values
    // It is important that first one must use setGPIO, then writeGPIO

    // Bit3: DIR3
    bitWrite(_GPIO, 3, dir3 == 1 ? 1 : 0);
    //-----------------------------------------------------
    // Bit2: DIR2
    bitWrite(_GPIO, 2, dir2 == 1 ? 1 : 0);
    //-----------------------------------------------------
    // Bit1: DIR1
    bitWrite(_GPIO, 1, dir1 == 1 ? 1 : 0);
    //-----------------------------------------------------
    // Bit0: DIR0
    bitWrite(_GPIO, 0, dir0 == 1 ? 1 : 0);
    //-----------------------------------------------------

    writeRegister(ADS1256_REG_IO, _GPIO);
    sleep_ms(100);
}

// Reading GPIO
uint8_t ADS1256::readGPIO(uint8_t gpioPin) {
    uint8_t GPIO_bit3, GPIO_bit2, GPIO_bit1, GPIO_bit0, GPIO_return = 0;

    _GPIO = readRegister(ADS1256_REG_IO); // Read the GPIO register

    // Save each bit values in a variable
    GPIO_bit3 = bitRead(_GPIO, 3);
    GPIO_bit2 = bitRead(_GPIO, 2);
    GPIO_bit1 = bitRead(_GPIO, 1);
    GPIO_bit0 = bitRead(_GPIO, 0);

    sleep_ms(100);

    switch (gpioPin) { // Selecting which value should be returned
        case 0: GPIO_return = GPIO_bit0; break;
        case 1: GPIO_return = GPIO_bit1; break;
        case 2: GPIO_return = GPIO_bit2; break;
        case 3: GPIO_return = GPIO_bit3; break;
    }

    return GPIO_return;
}

void ADS1256::sendDirectCommand(uint8_t directCommand) {
    // Direct commands can be found in the datasheet Page 34, Table 24.
    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);

    _SPI->select(_CS_pin); // REF: P34: "CS must stay low during the entire command sequence"
    sleep_us(5);
    _SPI->transfer(directCommand); // Send Command
    sleep_us(5);
    _SPI->deselect(); // REF: P34: "CS must stay low during the entire command sequence"

    _SPI->endTransaction();
}

// Converting the 24-bit data into a voltage value
float ADS1256::convertToVoltage(int32_t rawData) {
    if (rawData >> 23 == 1) // if the 24th digit (sign) is 1, the number is negative
    {
        rawData = rawData - 16777216; // conversion for the negative sign
        //"mirroring" around zero
    }

    float voltage = ((2 * _VREF) / 8388607) * rawData / (pow(2, _PGA)); // 8388607 = 2^{23} - 1
    // REF: p23, Table 16.

    return (voltage);
}

void ADS1256::writeRegister(uint8_t registerAddress, uint8_t registerValueToWrite) {
    waitForDRDY();

    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
    _SPI->select(_CS_pin); // CS must stay LOW during the entire sequence [Ref: P34, T24]
    sleep_us(5); // see t6 in the datasheet
    _SPI->transfer(0x50 | registerAddress); // 0x50 = 01010000 = WREG
    _SPI->transfer(0x00); // 2nd (empty) command byte
    _SPI->transfer(registerValueToWrite); // pass the value to the register
    _SPI->deselect();
    _SPI->endTransaction();

    sleep_ms(100);
}

// Reading a register
uint8_t ADS1256::readRegister(uint8_t registerAddress) {
    waitForDRDY();

    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
    // SPI_MODE1 = output edge: rising, data capture: falling; clock polarity: 0, clock phase: 1.
    _SPI->select(_CS_pin); // CS must stay LOW during the entire sequence [Ref: P34, T24]
    _SPI->transfer(0x10 | registerAddress); // 0x10 = 0001000 = RREG - OR together the two numbers (command + address)
    _SPI->transfer(0x00); // 2nd (empty) command byte
    sleep_us(5); // see t6 in the datasheet
    _registerValuetoRead = _SPI->transfer(0xFF); // read out the register value
    _SPI->deselect();
    _SPI->endTransaction();

    sleep_ms(100);
    return _registerValuetoRead;
}

// Reading a single value ONCE using the RDATA command
uint32_t ADS1256::readSingle() {
    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
    _SPI->select(_CS_pin); // REF: P34: "CS must stay low during the entire command sequence"
    waitForDRDY();
    _SPI->transfer(ADS1256_CMD_RDATA); // Issue RDATA (0000 0001) command
    sleep_us(7);     // Wait t6 time (~6.51 us) REF: P34, FIG:30.

    _outputBuffer[0] = _SPI->transfer(0); // MSB
    _outputBuffer[1] = _SPI->transfer(0); // Mid-byte
    _outputBuffer[2] = _SPI->transfer(0); // LSB

    // Shifting and combining the above three items into a single, 24-bit number
    uint32_t _outputValue = ((uint32_t)_outputBuffer[0] << 16) | ((uint32_t)_outputBuffer[1] << 8) | (_outputBuffer[2]);

    _SPI->deselect(); // We finished the command sequence, so we set CS to HIGH
    _SPI->endTransaction();

    return (_outputValue);
}

// Reads the recently selected input channel using RDATAC
uint32_t ADS1256::readSingleContinuous() {
    if (_isAcquisitionRunning == false) {
        _isAcquisitionRunning = true;
        _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
        _SPI->select(_CS_pin); // REF: P34: "CS must stay low during the entire command sequence"
        waitForDRDY();
        _SPI->transfer(ADS1256_CMD_RDATAC); // Issue RDATAC (0000 0011)
        sleep_us(7);     // Wait t6 time (~6.51 us) REF: P34, FIG:30.
    }
    else {
        waitForDRDY();
    }

    _outputBuffer[0] = _SPI->transfer(0); // MSB
    _outputBuffer[1] = _SPI->transfer(0); // Mid-byte
    _outputBuffer[2] = _SPI->transfer(0); // LSB

    uint32_t _outputValue = ((uint32_t)_outputBuffer[0] << 16) | ((uint32_t)_outputBuffer[1] << 8) | (_outputBuffer[2]);

    return _outputValue;
}

uint32_t ADS1256::readChannel(uint8_t channel) {
    _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
    _SPI->select(_CS_pin); // CS must stay LOW during the entire sequence [Ref: P34, T24]

    uint32_t _outputValue = 0;

    if (channel < 8) {

        // Step 1. - Updating MUX
        switch (channel) {
            // Channels are written manually
            case 0:                    // Channel 2
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_0); // AIN1+AINCOM
                break;

            case 1:                    // Channel 3
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_1); // AIN2+AINCOM
                break;

            case 2:                    // Channel 4
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_2); // AIN3+AINCOM
                break;

            case 3:                    // Channel 5
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_3); // AIN4+AINCOM
                break;

            case 4:                    // Channel 6
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_4); // AIN5+AINCOM
                break;

            case 5:                    // Channel 7
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_5); // AIN6+AINCOM
                break;

            case 6:                    // Channel 8
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_6); // AIN7+AINCOM
                break;

            case 7:                    // Channel 1
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_7); // AIN0+AINCOM
                break;
        }

        // Step 2.
        _SPI->transfer(ADS1256_CMD_SYNC); // SYNC
        sleep_us(4);             // t11 delay 24*tau = 3.125 us //delay should be larger, so we delay by 4 us
        _SPI->transfer(ADS1256_CMD_WAKEUP); // WAKEUP

        waitForDRDY();

        // Step 3.
        // Issue RDATA (0000 0001) command
        _SPI->transfer(ADS1256_CMD_RDATA);
        sleep_us(7); // Wait t6 time (~6.51 us) REF: P34, FIG:30.

        _outputBuffer[0] = _SPI->transfer(0); // MSB
        _outputBuffer[1] = _SPI->transfer(0); // Mid-byte
        _outputBuffer[2] = _SPI->transfer(0); // LSB

        _SPI->deselect();

        _outputValue = ((uint32_t)_outputBuffer[0] << 16) | ((uint32_t)_outputBuffer[1] << 8) | (_outputBuffer[2]);
    }

    return _outputValue;
}

uint32_t ADS1256::cycleSingle() {
    if(_isAcquisitionRunning == false) {
        _isAcquisitionRunning = true;
        _cycle = 0;
        _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
        _SPI->select(_CS_pin); // CS must stay LOW during the entire sequence [Ref: P34, T24]
        _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
        _SPI->transfer(0x00);
        _SPI->transfer(ADS1256_SING_0); // AIN0+AINCOM
        _SPI->deselect();
        sleep_us(50);
        _SPI->select(_CS_pin);
    }
    else {}

    uint32_t _outputValue = 0;
    if (_cycle < 8) {
        waitForDRDY();
        // Step 1. - Updating MUX
        switch (_cycle) {
            // Channels are written manually
            case 0:                    // Channel 2
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_1); // AIN1+AINCOM
                break;

            case 1:                    // Channel 3
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_2); // AIN2+AINCOM
                break;

            case 2:                    // Channel 4
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_3); // AIN3+AINCOM
                break;

            case 3:                    // Channel 5
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_4); // AIN4+AINCOM
                break;

            case 4:                    // Channel 6
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_5); // AIN5+AINCOM
                break;

            case 5:                    // Channel 7
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_6); // AIN6+AINCOM
                break;

            case 6:                    // Channel 8
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_7); // AIN7+AINCOM
                break;

            case 7:                    // Channel 1
                _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
                _SPI->transfer(0x00);
                _SPI->transfer(ADS1256_SING_0); // AIN0+AINCOM
                break;
        }

        // Step 2.
        _SPI->transfer(ADS1256_CMD_SYNC); // SYNC
        // sleep_us(4);             // t11 delay 24*tau = 3.125 us //delay should be larger, so we delay by 4 us
        _SPI->transfer(ADS1256_CMD_WAKEUP); // WAKEUP

        // Step 3.
        // Issue RDATA (0000 0001) command
        _SPI->transfer(ADS1256_CMD_RDATA);
        sleep_us(7); // Wait t6 time (~6.51 us) REF: P34, FIG:30.

        _outputBuffer[0] = _SPI->transfer(0); // MSB
        _outputBuffer[1] = _SPI->transfer(0); // Mid-byte
        _outputBuffer[2] = _SPI->transfer(0); // LSB

        _outputValue = ((uint32_t)_outputBuffer[0] << 16) | ((uint32_t)_outputBuffer[1] << 8) | (_outputBuffer[2]);

        _cycle++; // Increase cycle - This will move to the next MUX input channel
        if (_cycle == 8) {
            _cycle = 0; // Reset to 0 - Restart conversion from the 1st input channel
        }
    }

    return _outputValue;
}

uint32_t ADS1256::cycleDifferential() {
    if (_isAcquisitionRunning == false) {
        _isAcquisitionRunning = true;
        _cycle = 0;
        _SPI->beginTransaction(_SPISpeed, _SPIBitOrder, _SPIMode);
        _SPI->select(_CS_pin); // CS must stay LOW during the entire sequence [Ref: P34, T24]
        _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
        _SPI->transfer(0x00);
        _SPI->transfer(ADS1256_DIFF_0_1); //AIN0+AINCOM
        _SPI->deselect();
        sleep_us(50);
        _SPI->select(_CS_pin);
    }

    uint32_t _outputValue = 0;
    if (_cycle < 4) {
        // DRDY has to go low
        waitForDRDY();

        // Step 1. - Updating MUX
        switch (_cycle) {
            case 0: //Channel 2
            _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
            _SPI->transfer(0x00);
            _SPI->transfer(ADS1256_DIFF_2_3);  //AIN2+AIN3
            break;

            case 1: //Channel 3
            _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
            _SPI->transfer(0x00);
            _SPI->transfer(ADS1256_DIFF_4_5); //AIN4+AIN5
            break;

            case 2: //Channel 4
            _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
            _SPI->transfer(0x00);
            _SPI->transfer(ADS1256_DIFF_6_7); //AIN6+AIN7
            break;

            case 3: //Channel 1
            _SPI->transfer(0x50 | 1); // 0x50 = WREG //1 = MUX
            _SPI->transfer(0x00);
            _SPI->transfer(ADS1256_DIFF_0_1); //AIN0+AIN1
            break;
        }

        _SPI->transfer(ADS1256_CMD_SYNC); // SYNC
        sleep_us(4);     // t11 delay 24*tau = 3.125 us //delay should be larger, so we delay by 4 us
        _SPI->transfer(ADS1256_CMD_WAKEUP); // WAKEUP

        // Step 3.
        _SPI->transfer(ADS1256_CMD_RDATA); // Issue RDATA (0000 0001) command
        sleep_us(7);     // Wait t6 time (~6.51 us) REF: P34, FIG:30.

        _outputBuffer[0] = _SPI->transfer(0); // MSB
        _outputBuffer[1] = _SPI->transfer(0); // Mid-byte
        _outputBuffer[2] = _SPI->transfer(0); // LSB

        _outputValue = ((uint32_t)_outputBuffer[0] << 16) | ((uint32_t)_outputBuffer[1] << 8) | (_outputBuffer[2]);

        _cycle++;
        if (_cycle == 4) {
            _cycle = 0;
            // After the 4th cycle, we reset to zero so the next iteration reads the 1st MUX again
        }
    }

    return _outputValue;
}
