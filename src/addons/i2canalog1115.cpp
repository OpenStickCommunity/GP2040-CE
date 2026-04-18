#include "addons/i2canalog1115.h"
#include "ADS1115.h"
#include "addons/analog.h"
#include "config.pb.h"
#include "gamepad.h"
#include "helper.h"
#include "storagemanager.h"
#include <cstdint>
#include <map>
#include <math.h>

// (2^16) / (((2^15) - 1) * 3.3V / 4.096V) // This is the only way to
// remap the output data since the ads1115 can only use its internal reference
// voltage
#ifndef ADS1115_3_3V_REMAP_FACTOR
#define ADS1115_3_3V_REMAP_FACTOR 2.482462122f
#endif

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

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  channelHop = 0;

  uIntervalMS = 8;
  nextTimer = getMillis();

  // Init our ADS1115 library
  ads->resetConfig();
  ads->setChannel(0);                                // Start on Channel 0
  ads->setConversionMode(ads1115Mode_t::CONTINUOUS); // Read analog continuously
  ads->setGain(ads1115Gain_t::ONE);                  // Set gain to 1
  ads->setDataRate(860);
  ads->start(); // start command (no effect when in continuous mode)
                //

  // Setup options
  instance.inner_deadzone_enable = options.inner_deadzone_enabled;
  instance.outer_deadzone_enable = options.outer_deadzone_enabled;

  instance.invert = options.invert;
  instance.autoCalibrate = options.autoCalibrate;
  instance.lxChannel = options.lxChannel;
  instance.lyChannel = options.lyChannel;
  instance.rxChannel = options.rxChannel;
  instance.ryChannel = options.ryChannel;

  instance.alertMode = options.alertMode;
  instance.alertPin = options.alertPin;

  instance.inner_deadzone[0] = options.channel0InnerDeadzone * (1 << 16) / 100;
  instance.outer_deadzone[0] = options.channel0OuterDeadzone * (1 << 16) / 100;
  instance.inner_deadzone[1] = options.channel1InnerDeadzone * (1 << 16) / 100;
  instance.outer_deadzone[1] = options.channel1OuterDeadzone * (1 << 16) / 100;
  instance.inner_deadzone[2] = options.channel2InnerDeadzone * (1 << 16) / 100;
  instance.outer_deadzone[2] = options.channel2OuterDeadzone * (1 << 16) / 100;
  instance.inner_deadzone[3] = options.channel3InnerDeadzone * (1 << 16) / 100;
  instance.outer_deadzone[3] = options.channel3OuterDeadzone * (1 << 16) / 100;

  // initialize default
  for (int i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
    instance.pins[i] = GAMEPAD_JOYSTICK_MID;
  }
}

void I2CAnalog1115Input::process() {
  if (nextTimer < getMillis()) {
    // if (ads->getConfig() & 0x8000) { // This definitely would not work
    float result;
    uint16_t readValue;

    // TODO add ALERT/RDY Pin functionality
    readValue = ads->readConversionResult();
    result = (int16_t)readValue * ADS1115_3_3V_REMAP_FACTOR;
    // result = (result > 0xFFFF) ? 0xFFFF : result;
    result = (float)std::clamp((uint32_t)result, (uint32_t)0, (uint32_t)0xFFFF);

    instance.pins[channelHop] = (uint16_t)result;

    channelHop = (channelHop + 1) % 4; // Loop 0-3
    ads->setChannel(channelHop);
    nextTimer =
        getMillis() + uIntervalMS; // interval for read (we can't be too fast)
                                   // }
    // }
  }

  // apply option modifiers
  for (int i = 0; i < ADS1115_CHANNEL_COUNT; i++) {
    // Clamp value
    instance.pins[i] = std::clamp(instance.pins[i], (uint16_t)0, (uint16_t)0xFFFF);

    int32_t offsetPin = instance.pins[i] - GAMEPAD_JOYSTICK_MID;
    // Apply Deadzone (Not circular)
    if (instance.inner_deadzone_enable & (0b1000 >> i)) {
      if (abs(offsetPin) < instance.inner_deadzone[i]) {
        instance.pins[i] = GAMEPAD_JOYSTICK_MID;
      }
    }
    if (instance.outer_deadzone_enable & (0b1000 >> i)) {
      if (offsetPin > instance.outer_deadzone[i]) {
        instance.pins[i] = (uint16_t)(0xFFFF);
      } else if (offsetPin < -instance.outer_deadzone[i]) {
        instance.pins[i] = 0;
      }
    }

    // Apply Invert
    if (instance.invert & (0b1000 >> i)) {
      instance.pins[i] = GAMEPAD_JOYSTICK_MID - offsetPin;
    }

    // TODO apply auto calibration
    if (instance.autoCalibrate & (0b1000 >> i)) {
    }
  }

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  gamepad->state.lx = (uint16_t)(instance.pins[instance.lxChannel]);
  gamepad->state.ly = (uint16_t)(instance.pins[instance.lyChannel]);
  gamepad->state.rx = (uint16_t)(instance.pins[instance.rxChannel]);
  gamepad->state.ry = (uint16_t)(instance.pins[instance.ryChannel]);
}
