#ifndef _GPGFX_UI_TYPES_H_
#define _GPGFX_UI_TYPES_H_

#include <string>
#include <functional>
#include <vector>

typedef struct {
    std::string label;
    uint8_t* icon;
    std::function<void()> action;
} MenuEntry;

typedef struct {
    uint16_t top = 0;
    uint16_t left = 0;
    uint16_t bottom = 0;
    uint16_t right = 0;
} GPViewport;

#endif