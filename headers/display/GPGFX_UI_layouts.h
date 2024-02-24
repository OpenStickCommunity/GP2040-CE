#ifndef _GPGFX_UI_LAYOUTS_H_
#define _GPGFX_UI_LAYOUTS_H_

#include <map>
#include <vector>
#include "config.pb.h"
#include "enums.pb.h"

#define GPELEMENT_PARAM_COUNT 12

typedef struct {
    GPElement elementType;
    uint16_t parameters[GPELEMENT_PARAM_COUNT];
} GPButtonLayout;

#endif