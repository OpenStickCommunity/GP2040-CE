#ifndef _SNESpadAddon_H
#define _SNESpadAddon_H

#include <string>
#include <stdint.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "SNESpad.h"

// SNESpad Module Name
#define SNESpadName "SNESpad"

#ifndef SNES_PAD_ENABLED
#define SNES_PAD_ENABLED 0
#endif

#ifndef SNES_PAD_LATCH_PIN
#define SNES_PAD_LATCH_PIN -1
#endif

#ifndef SNES_PAD_CLOCK_PIN
#define SNES_PAD_CLOCK_PIN -1
#endif

#ifndef SNES_PAD_DATA_PIN
#define SNES_PAD_DATA_PIN -1
#endif

class SNESpadInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // SNESpad Setup
    virtual void process();     // SNESpad Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return SNESpadName; }
private:
    SNESpad * snes;
    uint32_t uIntervalMS;
    uint32_t nextTimer;

    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool buttonL = false;
    bool buttonR = false;

    bool buttonSelect = false;
    bool buttonStart = false;

    bool dpadUp     = false;
    bool dpadDown   = false;
    bool dpadLeft   = false;
    bool dpadRight  = false;

    uint16_t leftX = 0;
    uint16_t leftY = 0;
    uint16_t rightX = 0;
    uint16_t rightY = 0;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif  // _SNESpadAddon_H