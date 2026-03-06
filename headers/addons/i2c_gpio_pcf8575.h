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

#define PCF8575AddonName "PCF8575"

class PCF8575Addon : public GPAddon {
public:
    virtual uint16_t priority() { return 0; }
    virtual bool available();
    virtual void setup();
    virtual void preprocess(); // 判定前に実行
    virtual void process();    // 中身は空
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return PCF8575AddonName; }

    std::map<uint8_t, GpioMappingInfo> pinRef;

private:
    PCF8575* pcf;
    int bootSkipCount = 0; 

    // ボタン状態保持用フラグ
    bool inputButtonUp, inputButtonDown, inputButtonLeft, inputButtonRight;
    bool inputButtonB1, inputButtonB2, inputButtonB3, inputButtonB4;
    bool inputButtonL1, inputButtonR1, inputButtonL2, inputButtonR2;
    bool inputButtonS1, inputButtonS2, inputButtonL3, inputButtonR3;
    bool inputButtonA1, inputButtonA2, inputButtonA3, inputButtonA4, inputButtonFN;
    bool inputButtonEXT1, inputButtonEXT2, inputButtonEXT3, inputButtonEXT4;
    bool inputButtonEXT5, inputButtonEXT6, inputButtonEXT7, inputButtonEXT8;
    bool inputButtonEXT9, inputButtonEXT10, inputButtonEXT11, inputButtonEXT12;
    bool inputButtonMacro, inputButtonMacro1, inputButtonMacro2, inputButtonMacro3;
    bool inputButtonMacro4, inputButtonMacro5, inputButtonMacro6;
};

#endif
