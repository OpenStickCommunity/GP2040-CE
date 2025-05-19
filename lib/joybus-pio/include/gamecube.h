#ifndef _GAMECUBE_H_
#define _GAMECUBE_H_

#include "hardware/pio.h"
#include "gamecube.pio.h"

// GameCube protocol timing constants
#define GC_POLL_TIME_US 1000
#define GC_RESPONSE_TIME_US 850

// GameCube report structure
typedef struct {
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t y : 1;
    uint8_t start : 1;
    uint8_t padding : 3;
    uint8_t dpadLeft : 1;
    uint8_t dpadRight : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;
    uint8_t z : 1;
    uint8_t r : 1;
    uint8_t l : 1;
    uint8_t padding2 : 1;
    uint8_t xAxis;      // -128 to 127
    uint8_t yAxis;      // -128 to 127
    uint8_t cxAxis;     // -128 to 127
    uint8_t cyAxis;     // -128 to 127
    uint8_t left;       // 0-255 for analog trigger
    uint8_t right;      // 0-255 for analog trigger
} __attribute__((packed)) gc_report_t;

// Default report with neutral values
static const gc_report_t default_gc_report = {
    .a = 0,
    .b = 0,
    .x = 0,
    .y = 0,
    .start = 0,
    .padding = 0,
    .dpadLeft = 0,
    .dpadRight = 0,
    .dpadDown = 0,
    .dpadUp = 0,
    .z = 0,
    .r = 0,
    .l = 0,
    .padding2 = 0,
    .xAxis = 0,
    .yAxis = 0,
    .cxAxis = 0,
    .cyAxis = 0,
    .left = 0,
    .right = 0
};

#endif // _GAMECUBE_H_
