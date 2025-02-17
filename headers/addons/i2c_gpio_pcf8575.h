#ifndef PCF8575ADDON_H
#define PCF8575ADDON_H

#include "pcf8575.h"

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "peripheralmanager.h"

#include <map>

#ifndef I2C_PCF8575_ENABLED
#define I2C_PCF8575_ENABLED 0
#endif

#ifndef I2C_PCF8575_BLOCK
#define I2C_PCF8575_BLOCK i2c0
#endif

#define PCF8575_PIN_COUNT 16

// IO pin defaults
#ifndef PCF8575_PIN00_DIRECTION
#define PCF8575_PIN00_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN00_ACTION
#define PCF8575_PIN00_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN01_DIRECTION
#define PCF8575_PIN01_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN01_ACTION
#define PCF8575_PIN01_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN02_DIRECTION
#define PCF8575_PIN02_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN02_ACTION
#define PCF8575_PIN02_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN03_DIRECTION
#define PCF8575_PIN03_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN03_ACTION
#define PCF8575_PIN03_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN04_DIRECTION
#define PCF8575_PIN04_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN04_ACTION
#define PCF8575_PIN04_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN05_DIRECTION
#define PCF8575_PIN05_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN05_ACTION
#define PCF8575_PIN05_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN06_DIRECTION
#define PCF8575_PIN06_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN06_ACTION
#define PCF8575_PIN06_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN07_DIRECTION
#define PCF8575_PIN07_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN07_ACTION
#define PCF8575_PIN07_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN08_DIRECTION
#define PCF8575_PIN08_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN08_ACTION
#define PCF8575_PIN08_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN09_DIRECTION
#define PCF8575_PIN09_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN09_ACTION
#define PCF8575_PIN09_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN10_DIRECTION
#define PCF8575_PIN10_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN10_ACTION
#define PCF8575_PIN10_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN11_DIRECTION
#define PCF8575_PIN11_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN11_ACTION
#define PCF8575_PIN11_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN12_DIRECTION
#define PCF8575_PIN12_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN12_ACTION
#define PCF8575_PIN12_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN13_DIRECTION
#define PCF8575_PIN13_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN13_ACTION
#define PCF8575_PIN13_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN14_DIRECTION
#define PCF8575_PIN14_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN14_ACTION
#define PCF8575_PIN14_ACTION GpioAction::NONE
#endif

#ifndef PCF8575_PIN15_DIRECTION
#define PCF8575_PIN15_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#endif
#ifndef PCF8575_PIN15_ACTION
#define PCF8575_PIN15_ACTION GpioAction::NONE
#endif

// IO Module Name
#define PCF8575AddonName "PCF8575"

class PCF8575Addon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return PCF8575AddonName; }

    std::map<uint8_t, GpioMappingInfo> pinRef;
private:
    PCF8575* pcf;

    bool inputButtonUp = false;
    bool inputButtonDown = false;
    bool inputButtonLeft = false;
    bool inputButtonRight = false;
    bool inputButtonB1 = false;
    bool inputButtonB2 = false;
    bool inputButtonB3 = false;
    bool inputButtonB4 = false;
    bool inputButtonL1 = false;
    bool inputButtonR1 = false;
    bool inputButtonL2 = false;
    bool inputButtonR2 = false;
    bool inputButtonS1 = false;
    bool inputButtonS2 = false;
    bool inputButtonL3 = false;
    bool inputButtonR3 = false;
    bool inputButtonA1 = false;
    bool inputButtonA2 = false;
    bool inputButtonFN = false;
};

#endif  // _I2CAnalog_H_
