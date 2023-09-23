#ifndef _Reverse_H
#define _Reverse_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef REVERSE_ENABLED
#define REVERSE_ENABLED 0
#endif

#ifndef PIN_REVERSE
#define PIN_REVERSE     -1
#endif

// Reverse Module Name
#define ReverseName "Input Reverse"

class ReverseInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Reverse Button Setup
	virtual void preprocess() {}
	virtual void process();     // Reverse process
    virtual std::string name() { return ReverseName; }
private:
    void update();
    uint8_t input(uint32_t valueMask, uint16_t buttonMask, uint16_t buttonMaskReverse, uint8_t action, bool invertAxis);

	bool state;

	uint8_t pinLED;

	GamepadButtonMapping *mapDpadUp;
	GamepadButtonMapping *mapDpadDown;
	GamepadButtonMapping *mapDpadLeft;
	GamepadButtonMapping *mapDpadRight;

	bool invertXAxis;
	bool invertYAxis;

    // 0 - Ignore, 1 - Enabled, 2 - Neutral
    uint8_t actionUp;
    uint8_t actionDown;
    uint8_t actionLeft;
    uint8_t actionRight;

	uint8_t pinButtonReverse;
};

#endif // _Reverse_H_