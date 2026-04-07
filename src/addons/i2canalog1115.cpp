#include "addons/i2canalog1115.h"
#include "ADS1115.h"
#include "config.pb.h"
#include "gamepad.h"
#include "helper.h"
#include "storagemanager.h"
#include <cstdint>
#include <map>

// 655535 / (((2 << 15) - 1) * 3.3V / 4.096V) // This is the only way to remap
// the output data since the ads1115 can only use its internal reference voltage
#define ADS1115_3_3V_REMAP_FACTOR 2.482462122f

bool I2CAnalog1115Input::available() {
  const AnalogADS1115Options &options =
      Storage::getInstance().getAddonOptions().analogADS1115Options;
  if (options.enabled) {
    ads = new ADS1115Device();
    PeripheralI2CScanResult result =
        PeripheralManager::getInstance().scanForI2CDevice(
            ads->getDeviceAddresses());
    if (result.address > -1) {
      ads->setAddress(result.address);
      ads->setI2C(PeripheralManager::getInstance().getI2C(result.block));
      return true;
    } else {
      delete ads;
    }
  }
  return false;
}

void I2CAnalog1115Input::setup() {
  const AnalogADS1115Options &options =
      Storage::getInstance().getAddonOptions().analogADS1115Options;

  memset(&pins, 0, sizeof(pins));
  Gamepad * gamepad = Storage::getInstance().GetGamepad();
  channelHop = 0;

  uIntervalMS = 10;
  nextTimer = getMillis();

  // Init our ADS1115 library
  ads->resetConfig();
  ads->setChannel(0);                                // Start on Channel 0
  ads->setConversionMode(ads1115Mode_t::CONTINUOUS); // Read analog continuously
  ads->setGain(ads1115Gain_t::ONE);                  // Set gain to 1
  ads->setDataRate(860);
  ads->start(); // start command (no effect when in continuous mode)
                //
}

void I2CAnalog1115Input::process() {
  if (nextTimer < getMillis()) {
    float result;
    uint16_t readValue;
    // NOTE Conversion-ready alert is outputted on ALERT/RDY pin. Will ignore
    // for now. Might add option later 
    // if ( ads->readRegister16(STATUS) &
    // REGISTER_STATUS_DRDY ) {

    // I can probably get away with trying to read the config register
    // operational status first
    readValue = ads->readConversionResult();
    result = (int16_t)readValue * ADS1115_3_3V_REMAP_FACTOR;  
    result = (result > 0xFFFF) ? 0xFFFF : result;

    pins[channelHop] = result;
    channelHop = (channelHop + 1) % 4; // Loop 0-3
    ads->setChannel(channelHop);
    nextTimer =
        getMillis() + uIntervalMS; // interval for read (we can't be too fast)
                                   // }
  }
  
  Gamepad * gamepad = Storage::getInstance().GetGamepad();
  gamepad->state.lx = (uint16_t)(pins[0]);
  gamepad->state.ly = (uint16_t)(pins[1]);
  gamepad->state.rx = (uint16_t)(pins[2]);
  gamepad->state.ry = (uint16_t)(pins[3]);
}
