#ifndef CONFIG_AUTOGEN_H_
#define CONFIG_AUTOGEN_H_

// Boards supported in PlatformIO build configuration

#ifdef BOARD_ADAFRUIT_FEATHER_RP2040
#include "boards/adafruit_feather_rp2040.h"
#elif BOARD_ADAFRUIT_ITSYBITSY_RP2040
#include "boards/adafruit_itsybitsy_rp2040.h"
#elif BOARD_SPARKFUN_MICRO_RP2040
#include "boards/sparkfun_promicro.h"
#elif BOARD_SPARKFUN_THING_PLUS
#include "boards/sparkfun_thingplus.h"
#elif BOARD_ARDUINO_NANO_CONNECT
#include "boards/arduino_nano_rp2040_connect.h"
#elif BOARD_PIMORONI_PICO_LIPO
#include "boards/pimoroni_picolipo_4mb.h"
#else
#include "boards/pico.h"
#endif

#endif
