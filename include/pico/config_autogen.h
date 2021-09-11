#ifndef CONFIG_AUTOGEN_H_
#define CONFIG_AUTOGEN_H_

#define PI_PICO_LAYOUT     0
#define PIMORONI_PICO_16MB 1

#if BOARD_DEFINITION == PIMORONI_PICO_16MB
#include "boards/pimoroni_picolipo_16mb.h"
#else
#include "boards/pico.h"
#endif

#endif
