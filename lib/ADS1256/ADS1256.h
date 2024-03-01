// ADS1256 header file
/*
 Name:		ADS1256.h
 Created:	2022/07/14
 Author:	Curious Scientist
 Editor:	Notepad++
 Comment: Visit https://curiousscientist.tech/blog/ADS1256-custom-library
 Special thanks to Abraão Queiroz for spending time on the code and suggesting corrections for ESP32 microcontrollers!

 2024-02-15 - Ported to GP2040-CE PicoPeripheral interface
*/

#ifndef _ADS1256_h
#define _ADS1256_h

#include "peripheral_spi.h"

#define ADS1256_MAX_3V 3.3f
#define ADS1256_MAX_5V 5.0f
#define ADS1256_VREF_VOLTAGE 2.5f
#define ADS1256_CHANNEL_COUNT 8

#ifndef ADS1256_DEBUG
#define ADS1256_DEBUG false
#endif

/**************************************
 * SPI configuration
 **************************************/

#ifndef ADS1256_SPI_SPEED_HZ
#define ADS1256_SPI_SPEED_HZ 1000000 // 1MHz
#endif
#ifndef ADS1256_SPI_MODE
#define ADS1256_SPI_MODE SPIMode::SPI_MODE1
#endif
#ifndef ADS1256_SPI_BIT_ORDER
#define ADS1256_SPI_BIT_ORDER spi_order_t::SPI_MSB_FIRST
#endif

/**************************************
 * Differential inputs
 **************************************/

#define ADS1256_DIFF_0_1 0b00000001 // A0 + A1 as differential input
#define ADS1256_DIFF_2_3 0b00100011 // A2 + A3 as differential input
#define ADS1256_DIFF_4_5 0b01000101 // A4 + A5 as differential input
#define ADS1256_DIFF_6_7 0b01100111 // A6 + A7 as differential input

/**************************************
 * Single-ended inputs
 **************************************/

#define ADS1256_SING_0 0b00001111 // A0 + GND (common) as single-ended input
#define ADS1256_SING_1 0b00011111 // A1 + GND (common) as single-ended input
#define ADS1256_SING_2 0b00101111 // A2 + GND (common) as single-ended input
#define ADS1256_SING_3 0b00111111 // A3 + GND (common) as single-ended input
#define ADS1256_SING_4 0b01001111 // A4 + GND (common) as single-ended input
#define ADS1256_SING_5 0b01011111 // A5 + GND (common) as single-ended input
#define ADS1256_SING_6 0b01101111 // A6 + GND (common) as single-ended input
#define ADS1256_SING_7 0b01111111 // A7 + GND (common) as single-ended input

/**************************************
 * PGA settings (Input voltage range)
 **************************************/

#define ADS1256_PGA_1 0b00000000  // ± 5 V
#define ADS1256_PGA_2 0b00000001  // ± 2.5 V
#define ADS1256_PGA_4 0b00000010  // ± 1.25 V
#define ADS1256_PGA_8 0b00000011  // ± 625 mV
#define ADS1256_PGA_16 0b00000100 // ± 312.5 mV
#define ADS1256_PGA_32 0b00000101 //+ 156.25 mV
#define ADS1256_PGA_64 0b00000110 // ± 78.125 mV

/**************************************
 * Datarate (DEC)
 **************************************/

#define ADS1256_DRATE_30000SPS 0b11110000 // 240
#define ADS1256_DRATE_15000SPS 0b11100000 // 224
#define ADS1256_DRATE_7500SPS 0b11010000  // 208
#define ADS1256_DRATE_3750SPS 0b11000000  // 192
#define ADS1256_DRATE_2000SPS 0b10110000  // 176
#define ADS1256_DRATE_1000SPS 0b10100001  // 161
#define ADS1256_DRATE_500SPS 0b10010010   // 146
#define ADS1256_DRATE_100SPS 0b10000010   // 130
#define ADS1256_DRATE_60SPS 0b01110010    // 114
#define ADS1256_DRATE_50SPS 0b01100011    // 99
#define ADS1256_DRATE_30SPS 0b01010011    // 83
#define ADS1256_DRATE_25SPS 0b01000011    // 67
#define ADS1256_DRATE_15SPS 0b00110011    // 51
#define ADS1256_DRATE_10SPS 0b00100011    // 35
#define ADS1256_DRATE_5SPS 0b00010011     // 19
#define ADS1256_DRATE_2SPS 0b00000011     // 3

/**************************************
 * Status register
 **************************************/

#define ADS1256_BITORDER_MSB 0
#define ADS1256_BITORDER_LSB 1
#define ADS1256_ACAL_DISABLED 0
#define ADS1256_ACAL_ENABLED 1
#define ADS1256_BUFFER_DISABLED 0
#define ADS1256_BUFFER_ENABLED 1

/**************************************
 * Register addresses
 **************************************/

#define ADS1256_REG_STATUS 0x00
#define ADS1256_REG_MUX 0x01
#define ADS1256_REG_ADCON 0x02
#define ADS1256_REG_DRATE 0x03
#define ADS1256_REG_IO 0x04
#define ADS1256_REG_OFC0 0x05
#define ADS1256_REG_OFC1 0x06
#define ADS1256_REG_OFC2 0x07
#define ADS1256_REG_FSC0 0x08
#define ADS1256_REG_FSC1 0x09
#define ADS1256_REG_FSC2 0x0A

/**************************************
 * Command definitions
 **************************************/

#define ADS1256_CMD_RDATA 0b00000001
#define ADS1256_CMD_RDATAC 0b00000011
#define ADS1256_CMD_SDATAC 0b00001111
#define ADS1256_CMD_RREG 0b00010000
#define ADS1256_CMD_WREG 0b01010000
#define ADS1256_CMD_SELFCAL 0b11110000
#define ADS1256_CMD_SELFOCAL 0b11110001
#define ADS1256_CMD_SELFGCAL 0b11110010
#define ADS1256_CMD_SYSOCAL 0b11110011
#define ADS1256_CMD_SYSGCAL 0b11110100
#define ADS1256_CMD_SYNC 0b11111100
#define ADS1256_CMD_STANDBY 0b11111101
#define ADS1256_CMD_RESET 0b11111110
#define ADS1256_CMD_WAKEUP 0b11111111

class ADS1256 {
public:
    // Constructor
    ADS1256(PeripheralSPI *spi, const int DRDY_pin, const int RESET_pin, const int SYNC_pin, const int CS_pin, float VREF);

    // Initializing function
    void init(uint8_t drate, uint8_t pga, bool useBuf);

    // Read a register
    uint8_t readRegister(uint8_t registerAddress);

    // Write a register
    void writeRegister(uint8_t registerAddress, uint8_t registerValueToWrite);

    // Individual methods
    void setDRATE(uint8_t drate);
    void setPGA(uint8_t pga);
    uint8_t getPGA();
    void setMUX(uint8_t mux);
    void setByteOrder(uint8_t byteOrder);
    void getByteOrder();
    void setBuffer(uint8_t bufen);
    void getBuffer();
    void setAutoCal(uint8_t acal);
    void getAutoCal();
    void setGPIO(uint8_t dir0, uint8_t dir1, uint8_t dir2, uint8_t dir3);
    void writeGPIO(uint8_t dir0value, uint8_t dir1value, uint8_t dir2value, uint8_t dir3value);
    uint8_t readGPIO(uint8_t gpioPin);
    void setCLKOUT(uint8_t clkout);
    void setSDCS(uint8_t sdcs);
    void sendDirectCommand(uint8_t directCommand);

    // Get a single conversion
    uint32_t readSingle();

    // Single input continuous reading
    uint32_t readSingleContinuous();

    // Reads a single channel value
    uint32_t readChannel(uint8_t channel);

    // Cycling through the single-ended inputs
    uint32_t cycleSingle(); // Ax + COM

    // Cycling through the differential inputs
    uint32_t cycleDifferential(); // Ax + Ay

    // Converts the reading into a voltage value
    float convertToVoltage(int32_t rawData);

    // Stop AD
    void stopConversion();

private:
    void waitForDRDY();

    PeripheralSPI *_SPI;

    float _VREF; // Value of the reference voltage
    // Pins
    int _DRDY_pin;  // Pin assigned for DRDY
    int _CS_pin;    // Pin assigned for CS
    int _RESET_pin; // Pin assigned for RESET (optional)
    int _SYNC_pin;  // Pin assigned for SYNC/PDWN

    // SPI format
    int _SPISpeed = ADS1256_SPI_SPEED_HZ;
    SPIMode _SPIMode = ADS1256_SPI_MODE;
    spi_order_t _SPIBitOrder = ADS1256_SPI_BIT_ORDER;

    // Register-related variables
    uint8_t _registerAddress;      // Value holding the address of the register we want to manipulate
    uint8_t _registerValueToWrite; // Value to be written on a selected register
    uint8_t _registerValuetoRead;  // Value read from the selected register

    // Register values
    uint8_t _DRATE;     // Value of the DRATE register
    uint8_t _ADCON;     // Value of the ADCON register
    uint8_t _MUX;       // Value of the MUX register
    uint8_t _PGA;       // Value of the PGA (within ADCON)
    uint8_t _GPIO;      // Value of the GPIO register
    uint8_t _STATUS;    // Value of the status register
    uint8_t _GPIOvalue; // GPIO value
    uint8_t _ByteOrder; // Byte order

    uint8_t _outputBuffer[3];   // 3-byte (24-bit) buffer for the fast acquisition - Single-channel, continuous
    // uint32_t _outputValue;      // Combined value of the _outputBuffer[3]
    bool _isAcquisitionRunning; // bool that keeps track of the acquisition (running or not)
    uint8_t _cycle;             // Tracks the cycles as the MUX is cycling through the input channels
};

#endif
