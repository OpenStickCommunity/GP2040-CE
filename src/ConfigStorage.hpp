#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_ 

#include "FlashPROM.h"
#include "Animation.hpp"
#include "AnimationStation.hpp"
#include "BoardConfig.h"
#include "Effects/StaticColor.hpp"

#define STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX 3
#define STORAGE_LEDS_BRIGHTNESS_INDEX 4

class ConfigStorage
{
public:
  ConfigStorage() {
    // EEPROM.start();
  }
  AnimationMode getBaseAnimation();
  void setup();
  void save(AnimationStation as);
  void setBaseAnimation(AnimationMode mode);
  void commit();
  uint8_t getBrightness();
  void setBrightness(uint8_t brightness);
};


#endif