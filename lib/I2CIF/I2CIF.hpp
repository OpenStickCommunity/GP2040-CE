//
// This is a modified version of the Bit Bang I2C Library
// Modified on: 9/2/2023
// Original Notice:
//
// Bit Bang I2C library
// Copyright (c) 2018 BitBank Software, Inc.
// Written by Larry Bank (bitbank@pobox.com)
// Project started 10/12/2018
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __BITBANG_I2C__
#define __BITBANG_I2C__

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include <cstdint>

#ifndef LOW
#define LOW 0
#define HIGH 1
#endif

struct I2C_IF {
  i2c_inst_t* picoI2C; // used pico I2C
  spi_inst_t* picoSPI; // used pico SPI
  uint8_t iSDA, iSCL;   // pin numbers (0xff = disabled)

  // Initialize the I2C BitBang library
  // Pass the pin numbers used for SDA and SCL
  // as well as the clock rate in Hz
  void init(uint32_t iClock) {
    if ((iSDA + 2 * i2c_hw_index(picoI2C))%4 != 0) return ;
    if ((iSCL + 3 + 2 * i2c_hw_index(picoI2C))%4 != 0) return ;

    i2c_init(picoI2C, iClock);
    gpio_set_function(iSDA, GPIO_FUNC_I2C);
    gpio_set_function(iSCL, GPIO_FUNC_I2C);
    gpio_pull_up(iSDA);
    gpio_pull_up(iSCL);
    return;
  }

  // Read N bytes
  int read(uint8_t iAddr, uint8_t *pData, int iLen) {
    return (i2c_read_blocking(picoI2C, iAddr, pData, iLen, false) >= 0);
  }

  // Read N bytes starting at a specific I2C internal register
  int readRegister(uint8_t iAddr, uint8_t u8Register, uint8_t *pData, int iLen) {
    int rc = i2c_write_blocking(picoI2C, iAddr, &u8Register, 1, true); // true to keep master control of bus 
    if (rc >= 0) {
        rc = i2c_read_blocking(picoI2C, iAddr, pData, iLen, false);
    }
    return (rc >= 0);
  }

  // Write I2C data
  // quits if a NACK is received and returns 0
  // otherwise returns the number of bytes written
  int write(uint8_t iAddr, uint8_t *pData, int iLen) { 
    return (i2c_write_blocking(picoI2C, iAddr, pData, iLen, true) >= 0) ? iLen : 0; 
  }

  // Scans for I2C devices on the bus
  // returns a bitmap of devices which are present (128 bits = 16 bytes, LSB first)
  //
  // Test if an address responds
  // returns 0 if no response, 1 if it responds
  uint8_t test(uint8_t addr) {
    uint8_t rxdata;
    return (i2c_read_blocking(picoI2C, addr, &rxdata, 1, false) >= 0);
  }

  // A set bit indicates that a device responded at that address
  void scan(uint8_t *pMap) {
    int i;
    for (i=0; i<16; i++) // clear the bitmap
      pMap[i] = 0;
    for (i=1; i<128; i++) // try every address
    {
      if (test(i))
      {
        pMap[i >> 3] |= (1 << (i & 7));
      }
    }
  }
};

#endif //__BITBANG_I2C__

