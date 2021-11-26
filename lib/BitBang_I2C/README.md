Bit Bang I2C library
--------------------
Copyright (c) 2018 BitBank Software, Inc.
Written by Larry Bank (bitbank@pobox.com)
Project started 10/12/2018

The purpose of this code is to provide a simple C library which can bit-bang
the I2C protocol on any 2 GPIO pins on any system. The I2C protocol doesn't
require any special functionality of the pins beyond standard GPIO features.
The reason I wrote it was for getting easy access to I2C devices on
various microcontrollers that don't necessarily have exposed I2C interfaces.
This has come in handy on a variety of projects including AVR, ESP32, and nRF5
micrcontrollers.

The pin access functions can be wrapper functions for the native versions (e.g. on the nRF5 SDK)
On AVR micros, the digitalWrite/digitalRead/pinMode functions are somewhat
slow because they check the pin numbers against tables and do other tasks.
This library includes logic to speed that up. By specifying pin numbers as the
port name + bit, the library will run considerably faster on AVR
microcontrollers. For example, On the Arduino Uno (ATmega328P), I/O pin 9 is
actually I/O Port B, bit 1. To use the direct pin method, you would specify
the pin number as `0xB1`. On the ATtiny85, this is the only pin numbering
supported so that the Wire library doesn't get linked in (to save FLASH space). 

This latest version allows you to use this library for both bit-bang I2C or
make use of the Wire library indirectly. Since each BBI2C object is independent,
you can have as many buses as you like operating on any combination of
bit-bang and hardware I2C.
 
Usage:
-----
Start by initializing a BBI2C structure with the desired pin numbers for SDA/SCL
along with the desired clock frequency. The bWire flag tells the library to use
hardware I2C when set to true. If using the hardware I2C (Wire library), the
pin numbers can be set to `0xff` to use the default I2C pins or to specific pins
on systems which support multiple I2C buses. Frequencies above 400Khz are 
possible, but not necessarily accurate. Luckily I2C devices don't really
care about the exact clock frequency, only that the signals are stable
within the given periods.

For Example:
```C++
BBI2C bbi2c;
bbi2c.bWire = 0; // use bit banging
bbi2c.iSDA = 10; // SDA on GPIO pin 10
bbi2c.iSCL = 11; // SCL on GPIO pin 11
I2CInit(&bbi2c, 100000); // SDA=pin 10, SCL=pin 11, 100K clock
```

Instead of exposing functions to start and stop I2C transactions, I decided
to make it simpler by providing composite functions that hide the details of
I2C protocol. For scanning the I2C bus for devices, I provide the I2CScan()
function which returns a bitmap (16 bytes x 8 bits) with a bit set for every
device it finds. Call it like this:

```C++
unsigned char ucMap[16];
I2CScan(&bbi2c, ucMap);
```

To detect if a single address is active, use `I2CTest(addr)`.

To identify the device, use `I2CDiscoverDevice(uint8_t iAddress)`.

For reading and writing data to the I2C device, use the following functions:

```C++
I2CRead(&bbi2c, uint8_t u8Address, uint8_t *pu8Data, int iLength);
I2CReadRegister(&bbi2c, uint8_t iAddr, uint8_t u8Register, uint8_t *pData, int iLen);
I2CWrite(&bbi2c, uint8_t iAddr, uint8_t *pData, int iLen);
```

There are currently 29 devices recognized by the discover function:
- SSD1306
- SH1106
- VL53L0X
- BMP180
- BMP280
- BME280
- BME680
- MPU6000
- MPU9250
- MCP9808
- LSM6DS3
- ADXL345
- ADS1115
- MAX44009
- MAG3110
- CCS811
- HTS221
- LPS25H
- LSM9DS1
- LM8330
- DS3231
- DS1307
- LIS3DH
- LIS3DSH
- INA219
- SHT3X
- HDC1080
- AXP192
- AXP202
- 24AAXXXE64

If you find this code useful, please consider sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)


