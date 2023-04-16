#ifndef _GPDAUGHTER_H
#define _GPDAUGHTER_H

#include <GPDaughter.h>
#include <vector>

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef GP_DAUGHTER_ENABLED
#define GP_DAUGHTER_ENABLED 0
#endif

#ifndef GP_DAUGHTER_SDA_PIN
#define GP_DAUGHTER_SDA_PIN -1
#endif

#ifndef GP_DAUGHTER_SCL_PIN
#define GP_DAUGHTER_SCL_PIN -1
#endif

#ifndef GP_DAUGHTER_BLOCK
#define GP_DAUGHTER_BLOCK i2c1
#endif

#ifndef GP_DAUGHTER_SPEED
#define GP_DAUGHTER_SPEED 100000
#endif

#ifndef GP_DAUGHTER_ADDRESS
#define GP_DAUGHTER_ADDRESS 0x37
#endif

#ifndef GP_DAUGHTER_ADC_MAP
#define GP_DAUGHTER_ADC_MAP "lx,ly,rx,ry,l2,r2"
#endif

#ifndef GP_DAUGHTER_PIN_MAP
#define GP_DAUGHTER_PIN_MAP ""
#endif

// Daughter Board Name
#define GPDaughterName "GPDaughter"

class GPDaughterInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Analog Setup
	virtual void preprocess() {}
	virtual void process();     // Analog Process
    virtual std::string name() { return GPDaughterName; }
private:
    void decode_adc_map(const std::string&);
    void decode_pin_map(const std::string&);
    GPDaughter * daughter;
    std::vector<unsigned long> adc_map;
    std::vector<unsigned long> pin_map;
	uint32_t uIntervalMS; // Daughter board cooldown
	uint32_t nextTimer; // Throttle timer
};

#endif  // _GPDAUGHTER_H_
