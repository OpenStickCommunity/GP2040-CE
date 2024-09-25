#ifndef _GPGFX_TYPES_H_
#define _GPGFX_TYPES_H_

#include <map>
#include "peripheral_i2c.h"
#include "peripheral_spi.h"

#define GPGFX_FONT_CHAR_OFFSET 32

// to retain legacy display sizes
typedef enum {
    SIZE_128x128 = 1,
    SIZE_128x32,
    SIZE_128x64,
    SIZE_132x64,
    SIZE_64x128,
} GPGFX_DisplaySize;

typedef enum {
    DISPLAY_TYPE_NONE,
    DISPLAY_TYPE_SSD1306,
    DISPLAY_TYPE_COUNT
} GPGFX_DisplayType;

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t* fontData;
} GPGFX_DisplayFont;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t depth; // bits per pixel
} GPGFX_DisplayMetrics;

typedef struct {
    GPGFX_DisplayType displayType;
    PeripheralI2C* i2c;
    PeripheralSPI* spi;
    uint16_t size;
    uint16_t address;
    uint8_t orientation;
    bool inverted;
    GPGFX_DisplayFont font;
} GPGFX_DisplayTypeOptions;

#endif