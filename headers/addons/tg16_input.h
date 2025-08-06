#ifndef _TG16padAddon_H
#define _TG16padAddon_H

#include <string>
#include <stdint.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"

// TG16pad Module Name
#define TG16padName "TG16pad"

#ifndef TG16_PAD_ENABLED
#define TG16_PAD_ENABLED 0
#endif

#ifndef TG16_PAD_OE_PIN
#define TG16_PAD_OE_PIN -1
#endif

#ifndef TG16_PAD_SELECT_PIN
#define TG16_PAD_SELECT_PIN -1
#endif

#ifndef TG16_PAD_DATA_PIN0
#define TG16_PAD_DATA_PIN0 -1
#endif
#ifndef TG16_PAD_DATA_PIN1
#define TG16_PAD_DATA_PIN1 -1
#endif
#ifndef TG16_PAD_DATA_PIN2
#define TG16_PAD_DATA_PIN2 -1
#endif
#ifndef TG16_PAD_DATA_PIN3
#define TG16_PAD_DATA_PIN3 -1
#endif

class TG16padInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // TG16pad Setup
    virtual void process();     // TG16pad Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return TG16padName; }
private:
    uint32_t uIntervalMS;
    uint32_t nextTimer;

    bool buttonI = false;
    bool buttonII = false;
    bool buttonSelect = false;
    bool buttonRun = false;
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;

    uint16_t leftX = 0;
    uint16_t leftY = 0;
    uint16_t rightX = 0;
    uint16_t rightY = 0;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    uint8_t readController();
    void updateButtons(uint16_t data);
};

#endif  // _TG16padAddon_H 