// ADS1115 Library
// category=Signal Input/Output


// Repurposed Code from GP2040-CE ADS1219 Library
// Original Author: OM222O <techelectroyt@gmail.com>
// Ported to BitBang by:
//   Luke Arntson <arntsonl@gmail.com>
// Repurposed by:
//   Mark Valencia <mav.aldrin@gmail.com>

#ifndef _ADS1115_H_
#define _ADS1115_H_

#include "peripheral_i2c.h"
#include <cstdint>

// NOTE These are applicable for the ADS111X series of ICs
// (ADS1113,ADS1114,ADS1115) and also the ADS101X series (ADS1013,ADS1014,ADS1015)
// Might make this into a more general ADS1X1X library in the future
#define ADS111X_CONFIG_OS_OFFSET 15
#define ADS111X_CONFIG_MUX_OFFSET 12
#define ADS111X_CONFIG_PGA_OFFSET 9
#define ADS111X_CONFIG_MODE_OFFSET 8
#define ADS111X_CONFIG_DR_OFFSET 5
#define ADS111X_CONFIG_COMP_MODE_OFFSET 4
#define ADS111X_CONFIG_COMP_POL_OFFSET 3
#define ADS111X_CONFIG_COMP_LAT_OFFSET 2
#define ADS111X_CONFIG_COMP_QUE_OFFSET 0

// TODO
#define ADS111X_CONVERSION_REGISTER_ADDRESS 0b00000000
#define ADS111X_CONFIG_REGISTER_ADDRESS     0b00000001
#define ADS111X_LOTHRESH_REGISTER_ADDRESS   0b00000010
#define ADS111X_HITHRESH_REGISTER_ADDRESS   0b00000011

#define ADS111X_CONFIG_DEFAULT 0x8583
// TODO
#define ADS111X_I2C_ADDR_GND                0x48
#define ADS111X_I2C_ADDR_VDD                0x49
#define ADS111X_I2C_ADDR_SDA                0x4A
#define ADS111X_I2C_ADDR_SCLK               0x4B

// NOTE change these to hexadecimal later
#define ADS111X_OS_MASK 0b0111111111111111
#define ADS111X_OS_OFF  0b0000000000000000
#define ADS111X_OS_ON   0b1000000000000000

// bit:14-12                    0b0111000000000000
#define ADS111X_MUX_MASK      0b1000111111111111
#define ADS111X_MUX_DIFF_0_1  0b0000000000000000
#define ADS111X_MUX_DIFF_0_3  0b0001000000000000
#define ADS111X_MUX_DIFF_1_3  0b0010000000000000
#define ADS111X_MUX_DIFF_2_3  0b0011000000000000
#define ADS111X_MUX_SINGLE_0  0b0100000000000000
#define ADS111X_MUX_SINGLE_1  0b0101000000000000
#define ADS111X_MUX_SINGLE_2  0b0110000000000000
#define ADS111X_MUX_SINGLE_3  0b0111000000000000

// bit:11-9                     0b0000111000000000
#define ADS111X_GAIN_MASK       0b1111000111111111
#define ADS111X_GAIN_TWOTHIRDS  0b0000000000000000
#define ADS111X_GAIN_ONE        0b0000001000000000
#define ADS111X_GAIN_TWO        0b0000010000000000
#define ADS111X_GAIN_FOUR       0b0000011000000000
#define ADS111X_GAIN_EIGHT      0b0000100000000000
#define ADS111X_GAIN_SIXTEEN    0b0000101000000000
#define ADS111X_GAIN_DEFAULT    0b0000010000000000

// bit:8                          0b0000000100000000
#define ADS111X_MODE_MASK         0b1111111011111111
#define ADS111X_MODE_CONTINUOUS   0b0000000000000000
#define ADS111X_MODE_SINGLE_SHOT  0b0000000100000000

// bit:7-5                      0b0000000011100000
#define ADS111X_DATA_RATE_MASK  0b1111111100011111
#define ADS1115_DATA_RATE_8     0b0000000000000000
#define ADS1115_DATA_RATE_16    0b0000000000100000
#define ADS1115_DATA_RATE_32    0b0000000001000000
#define ADS1115_DATA_RATE_64    0b0000000001100000
#define ADS1115_DATA_RATE_128   0b0000000010000000
#define ADS1115_DATA_RATE_250   0b0000000010100000
#define ADS1115_DATA_RATE_475   0b0000000011000000
#define ADS1115_DATA_RATE_860   0b0000000011100000

// bit:4                                0b0000000000010000
#define ADS111X_COMP_MODE_MASK        0b1111111111101111
#define ADS111X_COMP_MODE_TRADITIONAL 0b0000000000000000
#define ADS111X_COMP_MODE_WINDOW      0b0000000000010000

// bit:3                                    0b0000000000001000
#define ADS111X_COMP_POLARITY_MASK        0b1111111111110111
#define ADS111X_COMP_POLARITY_ACTIVE_LOW  0b0000000000000000
#define ADS111X_COMP_POLARITY_ACTIVE_HIGH 0b0000000000001000

// bit:2                          0b0000000000000100
#define ADS111X_COMP_LATCH_MASK   0b1111111111111011
#define ADS111X_COMP_LATCH_TRUE   0b0000000000000000
#define ADS111X_COMP_LATCH_FALSE  0b0000000000000100

// bit:1-0                        0b0000000000000011
#define ADS111X_COMP_QUE_MASK     0b1111111111111100
#define ADS111X_COMP_QUE_ONE      0b0000000000000000
#define ADS111X_COMP_QUE_TWO      0b0000000000000001
#define ADS111X_COMP_QUE_FOUR     0b0000000000000010
#define ADS111X_COMP_QUE_DISABLE  0b0000000000000011

#define ADS111X_LO_THRESH_DEFAULT 0x8000
#define ADS111X_HI_THRESH         0x7FFF


enum class ads1115AddressRegister_t : uint8_t {
  CONVERSION = ADS111X_CONVERSION_REGISTER_ADDRESS,
  CONFIG = ADS111X_CONFIG_REGISTER_ADDRESS,
  LO_THRESH = ADS111X_LOTHRESH_REGISTER_ADDRESS,
  HI_THRESH = ADS111X_HITHRESH_REGISTER_ADDRESS,
};
enum class ads1115OperationalStatus_t : uint16_t {
  OFF = ADS111X_OS_OFF,
  ON = ADS111X_OS_ON,
};

enum class ads1115Gain_t : uint16_t {
  DEFAULT = ADS111X_GAIN_DEFAULT,
  FOUR = ADS111X_GAIN_FOUR,
  TWO = ADS111X_GAIN_TWO,
  ONE = ADS111X_GAIN_ONE
};

enum class ads1115MUXDiff_t : uint16_t {
  DIFF_0_1 = ADS111X_MUX_DIFF_0_1,
  DIFF_0_3 = ADS111X_MUX_DIFF_0_3,
  DIFF_1_3 = ADS111X_MUX_DIFF_1_3,
  DIFF_2_3 = ADS111X_MUX_DIFF_2_3
};

enum class ads1115Channel_t : uint16_t {
  SINGLE_0 = ADS111X_MUX_SINGLE_0,
  SINGLE_1 = ADS111X_MUX_SINGLE_1,
  SINGLE_2 = ADS111X_MUX_SINGLE_2,
  SINGLE_3 = ADS111X_MUX_SINGLE_3
};


enum class ads1115DataRate_t : uint16_t {
  sps_8 = ADS1115_DATA_RATE_8,
  sps_16 = ADS1115_DATA_RATE_16,
  sps_32 = ADS1115_DATA_RATE_32,
  sps_64 = ADS1115_DATA_RATE_64,
  sps_128 = ADS1115_DATA_RATE_128,
  sps_250 = ADS1115_DATA_RATE_250,
  sps_475 = ADS1115_DATA_RATE_475,
  sps_860 = ADS1115_DATA_RATE_860,
};

enum class ads1115Mode_t : uint16_t {
  SINGLE_SHOT = ADS111X_MODE_SINGLE_SHOT,
  CONTINUOUS = ADS111X_MODE_CONTINUOUS
};

class ADS1115 {
protected:
  void setConfig(uint16_t config);
  uint8_t address;
  PeripheralI2C *i2c;

public:
  // Constructor
  ADS1115() {}
  ADS1115(PeripheralI2C *i2cController, uint8_t addr = ADS111X_I2C_ADDR_GND);

  // Methods
  void begin();
  void reset();
  void resetConfig();

  uint16_t readSingleEnded(int channel);
  uint16_t readDifferential_0_1();
  uint16_t readDifferential_0_3();
  uint16_t readDifferential_1_3();
  uint16_t readDifferential_2_3();

  void setGain(ads1115Gain_t gain);
  void setDataRate(int rate);
  void setConversionMode(ads1115Mode_t mode);
  void setOperationalStatus(ads1115OperationalStatus_t os);

  void setChannel(int channel);
  void powerDown();
  void start();
  uint16_t readRegister(ads1115AddressRegister_t reg);
  uint16_t readConfigRegister();
  uint16_t readConversionResult();

  void setI2C(PeripheralI2C *i2cController) { this->i2c = i2cController; }
  void setAddress(uint8_t addr) { this->address = addr; }

private:
  void writeRegister(uint16_t reg, uint16_t data);

  uint16_t config = ADS111X_CONFIG_DEFAULT;
  int data_ready;
  bool singleShot = true;
  uint8_t uc[8]; // data buffer(?)
};

#endif