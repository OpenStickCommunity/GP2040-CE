// ADS1219 Library
// category=Signal Input/Output
// url=https://github.com/OM222O/ADS1219

// Original Author: OM222O <techelectroyt@gmail.com> 

// Ported to BitBang by:
//   Luke Arntson <arntsonl@gmail.com>

#ifndef _ADS1219_H_
#define _ADS1219_H_

#include "peripheral_i2c.h"

#define CONFIG_REGISTER_ADDRESS 0x40
#define STATUS_REGISTER_ADDRESS 0x24

#define MUX_MASK 				0x1F
#define MUX_DIFF_0_1			0x00
#define MUX_DIFF_2_3			0x20
#define MUX_DIFF_1_2			0x40
#define MUX_SINGLE_0			0x60
#define MUX_SINGLE_1			0x80
#define MUX_SINGLE_2			0xA0
#define MUX_SINGLE_3			0xC0
#define MUX_SHORTED				0xE0

#define GAIN_MASK 				0xEF
#define GAIN_ONE				0x00
#define GAIN_FOUR				0x10

#define DATA_RATE_MASK			0xF3
#define DATA_RATE_20			0x00
#define DATA_RATE_90			0x04
#define DATA_RATE_330			0x08
#define DATA_RATE_1000			0x0c

#define MODE_MASK				0xFD
#define MODE_SINGLE_SHOT		0x00
#define MODE_CONTINUOUS			0x02

#define REGISTER_CONFIG         0x00
#define REGISTER_STATUS         0x01
#define REGISTER_STATUS_DRDY    0x40

#define VREF_MASK				0xFE
#define VREF_INTERNAL			0x00
#define VREF_EXTERNAL			0x01

typedef enum{
  ONE	= GAIN_ONE,
  FOUR	= GAIN_FOUR
}adsGain_t;

typedef enum{
  SINGLE_SHOT	= MODE_SINGLE_SHOT,
  CONTINUOUS	= MODE_CONTINUOUS
}adsMode_t;

typedef enum{
  REF_INTERNAL	= VREF_INTERNAL,
  REF_EXTERNAL	= VREF_EXTERNAL
}adsRef_t;

typedef enum{
  CONFIG = REGISTER_CONFIG,
  STATUS = REGISTER_STATUS
}adsRegister_t;

typedef enum{
  CHANNEL_0 = MUX_SINGLE_0,
  CHANNEL_1 = MUX_SINGLE_1,
  CHANNEL_2 = MUX_SINGLE_2,
  CHANNEL_3 = MUX_SINGLE_3
}adsChannel_t;

class ADS1219  {
  protected:
	uint8_t address;
	PeripheralI2C* i2c;
  public:
    // Constructor 
	ADS1219() {}
	ADS1219(PeripheralI2C *i2cController, uint8_t addr = 0x40);

    // Methods
    void begin();
	void reset();
	void resetConfig();
	long readSingleEnded(int channel);
	long readDifferential_0_1();
	long readDifferential_2_3();
	long readDifferential_1_2();
	long readShorted();
	void setGain(adsGain_t gain);
	void setDataRate(int rate);
	void setConversionMode(adsMode_t mode);
	void setVoltageReference(adsRef_t vref);
	void setChannel(int channel);
	void powerDown();
	uint8_t readRegister(adsRegister_t reg);
	void start();
	uint32_t readConversionResult();

	void setI2C(PeripheralI2C *i2cController) { this->i2c = i2cController; }
	void setAddress(uint8_t addr) { this->address = addr; }

  private:
	void writeRegister(uint8_t data);
	
	uint8_t config = 0x00;
	bool singleShot = true;
	int data_ready;
	unsigned char uc[128];
};

#endif
