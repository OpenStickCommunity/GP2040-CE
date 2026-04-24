#ifndef _I2CAnalog1115_H
#define _I2CAnalog1115_H

#include "GamepadEnums.h"
#include "ads1115_dev.h"
#include "gamepad.h"
#include "gpaddon.h"
#include "peripheralmanager.h"
#include <cstdint>

#ifndef ADS1115_CHANNEL_COUNT
#define ADS1115_CHANNEL_COUNT 4
#endif

#ifndef I2C_ANALOG1115_ENABLED
#define I2C_ANALOG1115_ENABLED 1
#endif

#ifndef I2C_ANALOG1115_SDA_PIN
#define I2C_ANALOG1115_SDA_PIN 0
#endif

#ifndef I2C_ANALOG1115_SCL_PIN
#define I2C_ANALOG1115_SCL_PIN 1
#endif

#ifndef I2C_ANALOG1115_BLOCK
#define I2C_ANALOG1115_BLOCK i2c0
#endif

#ifndef I2C_ANALOG1115_SPEED
#define I2C_ANALOG1115_SPEED 400000
#endif

#ifndef ANALOG1115_CHANNEL_INNER_DEADZONE
#define ANALOG1115_CHANNEL_INNER_DEADZONE 2
#endif

#ifndef ANALOG1115_CHANNEL_OUTER_DEADZONE
#define ANALOG1115_CHANNEL_OUTER_DEADZONE 98
#endif


#ifndef ANALOG1115_CHANNEL_ENABLE
#define ANALOG1115_CHANNEL_ENABLE 0b1111 // channel order: A0,A1,A2,A3
#endif

#ifndef ANALOG1115_INNER_DEADZONE_ENABLE
#define ANALOG1115_INNER_DEADZONE_ENABLE 0b1111 // channel order: A0,A1,A2,A3
#endif


#ifndef ANALOG1115_OUTER_DEADZONE_ENABLE
#define ANALOG1115_OUTER_DEADZONE_ENABLE 0b1111
#endif

#ifndef ANALOG1115_LSTICK_DEADZONE_ENABLE
#define ANALOG1115_LSTICK_DEADZONE_ENABLE 0
#endif

#ifndef ANALOG1115_RSTICK_DEADZONE_ENABLE
#define ANALOG1115_RSTICK_DEADZONE_ENABLE 0
#endif

#ifndef ANALOG1115_LSTICK_DEADZONE
#define ANALOG1115_LSTICK_DEADZONE 5
#endif

#ifndef ANALOG1115_RSTICK_DEADZONE
#define ANALOG1115_RSTICK_DEADZONE 5
#endif


#ifndef ANALOG1115_INVERT
#define ANALOG1115_INVERT 0b0000
#endif

#ifndef ANALOG1115_AUTOCALIBRATE
#define ANALOG1115_AUTOCALIBRATE 0b0000
#endif

#ifndef ANALOG1115_LX_CHANNEL
#define ANALOG1115_LX_CHANNEL 0
#endif
#ifndef ANALOG1115_LY_CHANNEL
#define ANALOG1115_LY_CHANNEL 1
#endif
#ifndef ANALOG1115_RX_CHANNEL
#define ANALOG1115_RX_CHANNEL 2
#endif
#ifndef ANALOG1115_RY_CHANNEL
#define ANALOG1115_RY_CHANNEL 3
#endif

#ifndef ANALOG1115_ALERT_MODE
#define ANALOG1115_ALERT_MODE 0
#endif
#ifndef ANALOG1115_ALERT_PIN
#define ANALOG1115_ALERT_PIN -1
#endif

#ifndef I2C_ANALOG1115_ADDRESS
#define I2C_ANALOG1115_ADDRESS 0x48
#endif


// Analog ADS1115 Module Name
#define I2CAnalog1115Name "ADS1115I2CAnalog"

// ADS1115 instance
typedef struct {
  int32_t inner_deadzone[4];
  int32_t outer_deadzone[4];
  int32_t l_stick_deadzone;
  int32_t r_stick_deadzone;
  uint16_t pins[4];
  uint8_t channel_enable;
  uint8_t inner_deadzone_enable;
  uint8_t outer_deadzone_enable;
  uint8_t lxChannel;
  uint8_t lyChannel;
  uint8_t rxChannel;
  uint8_t ryChannel;
  uint8_t invert;
  uint8_t autoCalibrate;
  bool LStickDeadzoneEnable;
  bool RStickDeadzoneEnable;
  bool alertMode;
  bool alertPin;

} ADS1115_Instance;

// NOTE probably have to rename the file
class I2CAnalog1115Input : public GPAddon {
public:
  virtual bool available();
  virtual void setup(); // Analog Setup
  virtual void preprocess() {}
  virtual void process(); // Analog Process
  virtual void postprocess(bool sent) {}
  virtual void reinit() {}
  virtual std::string name() { return I2CAnalog1115Name; }

private:
  ADS1115Device *ads;
  int channelHop;
  uint32_t uIntervalMS; // ADS1115 Interval
  uint32_t nextTimer;   // Turbo Timer
  ADS1115_Instance instance;
  int16_t CalculateMagnitudeXY(uint16_t &channelX, uint16_t &channelY);
};

#endif