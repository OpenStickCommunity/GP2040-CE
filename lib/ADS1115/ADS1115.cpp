
#include "ADS1115.h"

#include <cstdint>
#include <cstring>

ADS1115::ADS1115(PeripheralI2C *i2cController, uint8_t addr) {
  i2c = i2cController;
  address = addr;
  config = ADS111X_CONFIG_DEFAULT;
  singleShot = true;
}

void ADS1115::begin() { reset(); }
// NOTE This requires general call function. Not used
void ADS1115::reset() {}

// 7.5.1.2 I
// 2C General Call
// The ADS111x respond to the I2C general call address (0000000b) if the eighth
// bit is 0b. The devices acknowledge the general call address and respond to
// commands in the second byte. If the second byte is 00000110b (06h), the
// ADS111x reset the internal registers and enter a power-down state.
//

// Operational status or single-shot conversion start
// This bit determines the operational status of the device. OS can only be
// written when in power-down state and has no effect when a conversion is
// ongoing. When writing: 0b : No effect 1b : Start a single conversion (when in
// power-down state) When reading: 0b : Device is currently performing a
// conversion. 1b : Device is not currently performing a conversion

// Start data conversion in single-shot mode. Unused otherwise
void ADS1115::start() {
  // TODO check if device is in single-shot mode, else
  // config &= ADS111X_OS_MASK;
  // config |= ADS111X_OS_ON;
  // setConfig(config);
  setOperationalStatus(ads1115OperationalStatus_t::ON);
}

void ADS1115::powerDown() {
  // config &= ADS111X_OS_MASK;
  // config |= ADS111X_OS_OFF;
  // setConfig(config);
  setOperationalStatus(ads1115OperationalStatus_t::OFF);
}

void ADS1115::setConfig() {
  uc[0] = ADS111X_CONFIG_REGISTER_ADDRESS;

  uc[1] = (config >> 8) & 0xFF;
  uc[2] = config & 0xFF;

  i2c->write(address, uc, 3);
}


// Might be useful for configuring the LO_THRESH and HI_THRESH registers if they need to be used
void ADS1115::writeRegister(uint16_t reg, uint16_t data) {
  uc[0] = reg;
  if (reg == ADS111X_CONFIG_REGISTER_ADDRESS) config = data;
  uc[1] = (data >> 8) & 0xFF;
  uc[2] = data & 0xFF;
  i2c->write(address, uc, 3);
}


void ADS1115::resetConfig() { 
  config = ADS111X_CONFIG_DEFAULT;
  setConfig(); 
}

uint16_t ADS1115::readRegister(ads1115AddressRegister_t reg) {
  // uc[0] = ADS111X_CONFIG_REGISTER_ADDRESS;
  // i2c->write(address, uc, len);
  i2c->readRegister(address, (uint8_t)reg, uc, 1);

  uint16_t data = uc[0];
  return data;
}

// for reading config or conversion data
uint16_t ADS1115::readConfigRegister() { // reg must be 0 or 1
  // uc[0] = ADS111X_CONFIG_REGISTER_ADDRESS;
  // i2c->write(address, uc, len);
  i2c->readRegister(address, (uint8_t)ADS111X_CONFIG_REGISTER_ADDRESS, uc, 2);

  uint16_t data16 = (uc[0] << 8) | (uc[1]);
  return data16;
}


uint16_t ADS1115::readConversionResult() {
  // uc[0] = ADS111X_CONVERSION_REGISTER_ADDRESS; // Point to Conversion Register
  //                                              // Addresss
  // i2c->write(address, uc, 1);
  // i2c->read(address, uc, 2);
  // or just use the one below
  i2c->readRegister(address, ADS111X_CONVERSION_REGISTER_ADDRESS, uc, 2);
  // NOTE this is in two's complement format.
  uint16_t data16 = (uc[0] << 8) | (uc[1]);
  return data16;
}

uint16_t ADS1115::readSingleEnded(int channel) {
  config &= ADS111X_MUX_MASK;
  switch (channel) {
  case (0):
    config |= ADS111X_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS111X_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS111X_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS111X_MUX_SINGLE_3;
    break;
  default:
    break;
  }
  setConfig();
  return readConversionResult();
}

uint16_t ADS1115::readDifferential_0_1() {
  config &= ADS111X_MUX_MASK;
  config |= ADS111X_MUX_DIFF_0_1;
  setConfig();
  return readConversionResult();
}

uint16_t ADS1115::readDifferential_0_3() {
  config &= ADS111X_MUX_MASK;
  config |= ADS111X_MUX_DIFF_0_3;
  setConfig();
  return readConversionResult();
}

uint16_t ADS1115::readDifferential_1_3() {
  config &= ADS111X_MUX_MASK;
  config |= ADS111X_MUX_DIFF_1_3;
  setConfig();
  return readConversionResult();
}

void ADS1115::setOperationalStatus(ads1115OperationalStatus_t os) {
  config &= ADS111X_OS_MASK;
  config |= (uint16_t)os;
  setConfig();
}

void ADS1115::setGain(ads1115Gain_t gain) {
  config &= ADS111X_GAIN_MASK;
  config |= (int)gain;
  setConfig();
}

void ADS1115::setDataRate(int rate) {
  config &= ADS111X_DATA_RATE_MASK;
  switch (rate) {
  case (8):
    config |= ADS1115_DATA_RATE_8;
    break;
  case (16):
    config |= ADS1115_DATA_RATE_16;
    break;
  case (32):
    config |= ADS1115_DATA_RATE_32;
    break;
  case (64):
    config |= ADS1115_DATA_RATE_64;
    break;
  case (128):
    config |= ADS1115_DATA_RATE_128;
    break;
  case (250):
    config |= ADS1115_DATA_RATE_250;
    break;
  case (475):
    config |= ADS1115_DATA_RATE_475;
    break;
  case (860):
    config |= ADS1115_DATA_RATE_860;
    break;
  default:
    config |= ADS1115_DATA_RATE_128;
    break;
  }
  setConfig();
}

void ADS1115::setConversionMode(ads1115Mode_t mode) {
  config &= (uint16_t)ADS111X_MODE_MASK;
  config |= (uint16_t)mode;
  setConfig();
  if (mode == (ads1115Mode_t)ADS111X_MODE_CONTINUOUS) {
    singleShot = false;
  } else {
    singleShot = true;
  }
}

void ADS1115::setChannel(int channel) {
  config &= ADS111X_MUX_MASK;
  switch (channel) {
  case (0):
    config |= ADS111X_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS111X_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS111X_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS111X_MUX_SINGLE_3;
    break;
  default:
    break;
  }
  setConfig();
}
