#include "ADS1219.h"

#include <cstring>

ADS1219::ADS1219(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t speed, uint8_t addr) {
  bbi2c.iSDA = sda;
  bbi2c.iSCL = scl;
  bbi2c.picoI2C = picoI2C;
  bbi2c.bWire = bWire;
  iSpeed = speed;
  address = addr;
  config = 0x00;
  singleShot = true;
}

void ADS1219::begin() {
  I2CInit(&bbi2c, iSpeed); // on Linux, SDA = bus number, SCL = device address
  reset();
}

// 8.5.3.3 START/SYNC (0000 100x)
//  In single-shot conversion mode, the START/SYNC command is used to start a single conversion, or (when sent
//  during an ongoing conversion) to reset the digital filter and then restart a single new conversion. When the
//  device is set to continuous conversion mode, the START/SYNC command must be issued one time to start
//  converting continuously. Sending the START/SYNC command when converting in continuous conversion mode
//  resets the digital filter and restarts continuous conversions.
void ADS1219::start(){
  uc[0] = 0x08;
  I2CWrite(&bbi2c, address, uc, 1);
}

// 8.5.3.4 POWERDOWN (0000 001x)
//  The POWERDOWN command places the device into power-down mode. This command shuts down all internal
//  analog components, but holds all register values. In case the POWERDOWN command is issued when a
//  conversion is ongoing, the conversion completes before the ADS1219 enters power-down mode. As soon as a
//  START/SYNC command is issued, all analog components return to their previous states.
void ADS1219::powerDown(){
  uc[0] = 0x02;
  I2CWrite(&bbi2c, address, uc, 1);
}


// 8.5.3.6 RREG (0010 0rxx)
//  The RREG command reads the value of the register at address r. Reading a register must be performed as
//  shown in Figure 37 by using two I2C communication frames. The first frame is an I2C write operation where the
//  R/W bit at the end of the address byte is 0 to indicate a write. In this frame, the host sends the RREG command
//  including the register address to the ADS1219. The second frame is an I2C read operation where the R/W bit at
//  the end of the address byte is 1 to indicate a read. The ADS1219 reports the contents of the requested register
//  in this second I2C frame.
uint8_t ADS1219::readRegister(adsRegister_t reg){ // reg must be 0 or 1
  uc[0] = 0x20 | (reg<<2); // this is a guess
  I2CWrite(&bbi2c, address, uc, 1);
  I2CRead(&bbi2c, address, uc, 1);
  return uc[0];
}

// 8.5.3.7 WREG (0100 00xx dddd dddd)
//  The WREG command writes dddd dddd to the configuration register. Figure 38 shows the sequence for writing
//  the configuration register. The R/W bit at the end of the address byte is 0 to indicate a write. The WREG
//  command forces the digital filter to reset and any ongoing ADC conversion to restart.
void ADS1219::writeRegister(uint8_t data){
  uc[0] = CONFIG_REGISTER_ADDRESS;
  uc[1] = data;
  I2CWrite(&bbi2c, address, uc, 2);
}

// 8.5.3.5 RDATA (0001 xxxx)
//  The RDATA command loads the output shift register with the most recent conversion result. Reading conversion
//  data must be performed as shown in Figure 36 by using two I2C communication frames. The first frame is an I2C
//  write operation where the R/W bit at the end of the address byte is 0 to indicate a write. In this frame, the host
//  sends the RDATA command to the ADS1219. The second frame is an I2C read operation where the R/W bit at
//  the end of the address byte is 1 to indicate a read. The ADS1219 reports the latest ADC conversion data in this
//  second I2C frame. If a conversion finishes in the middle of the RDATA command byte, the state of the DRDY pin
//  at the end of the read operation signals whether the old or the new result is loaded. If the old result is loaded,
//  DRDY stays low, indicating that the new result is not read out. The new conversion result loads when DRDY is
//  high.
uint32_t ADS1219::readConversionResult(){
  uc[0] = 0x10; // Read from 24-bit conversion
  I2CWrite(&bbi2c, address, uc, 1);
  I2CRead(&bbi2c, address, uc, 3);
  uint32_t data32 = (uc[0] << 16) | (uc[1] << 8) | (uc[2]);
  if (data32 >= 0x800000)
			data32 = data32-0x1000000;
  return data32; // 24-bit ADC result signage hack
}

// 8.5.3.2 RESET (0000 011x)
//  This command resets the device to the default states. No delay time is required after the RESET command is
//  latched before starting to communicate with the device as long as the timing requirements (see the I2C Timing
//  Requirements table) for the (repeated) START and STOP conditions are met.
void ADS1219::reset(){
  uc[0] = 0x6;
  I2CWrite(&bbi2c, address, uc, 1);
}

void ADS1219::resetConfig(){
	writeRegister(0x00);
}

long ADS1219::readSingleEnded(int channel){
	config &= MUX_MASK;
	switch (channel){
    case (0):
      config |= MUX_SINGLE_0;
      break;
    case (1):
      config |= MUX_SINGLE_1;
      break;
    case (2):
      config |= MUX_SINGLE_2;
      break;
    case (3):
      config |= MUX_SINGLE_3;
      break;
	default:
	  break;
  }
  writeRegister(config);
  return readConversionResult();
}

long ADS1219::readDifferential_0_1(){
  config &= MUX_MASK;
  config |= MUX_DIFF_0_1;
  writeRegister(config);
  return readConversionResult();
}

long ADS1219::readDifferential_2_3(){
  config &= MUX_MASK;
  config |= MUX_DIFF_2_3;
  writeRegister(config);
  return readConversionResult();
}

long ADS1219::readDifferential_1_2(){
  config &= MUX_MASK;
  config |= MUX_DIFF_1_2;
  writeRegister(config);
  return readConversionResult();
}

long ADS1219::readShorted(){
  config &= MUX_MASK;
  config |= MUX_SHORTED;
  writeRegister(config);
  return readConversionResult();
}

void ADS1219::setGain(adsGain_t gain){
  config &= GAIN_MASK;
  config |= gain;
  writeRegister(config);
}

void ADS1219::setDataRate(int rate){
	config &= DATA_RATE_MASK;
	switch (rate){
    case (20):
      config |= DATA_RATE_20;
      break;
    case (90):
      config |= DATA_RATE_90;
      break;
    case (330):
      config |= DATA_RATE_330;
      break;
    case (1000):
      config |= DATA_RATE_1000;
      break;
	default:
	  break;
  }
  writeRegister(config);
}

void ADS1219::setConversionMode(adsMode_t mode){
  config &= MODE_MASK;
  config |= mode;
  writeRegister(config);
  if (mode == CONTINUOUS){
	  singleShot = false;
  } else {
	  singleShot = true;
  }
}

void ADS1219::setVoltageReference(adsRef_t vref){
  config &= VREF_MASK;
  config |= vref;
  writeRegister(config);
}

void ADS1219::setChannel(int channel){
  config &= MUX_MASK;
	switch (channel){
    case (0):
      config |= MUX_SINGLE_0;
      break;
    case (1):
      config |= MUX_SINGLE_1;
      break;
    case (2):
      config |= MUX_SINGLE_2;
      break;
    case (3):
      config |= MUX_SINGLE_3;
      break;
	  default:
	    break;
  }
  writeRegister(config);
}
