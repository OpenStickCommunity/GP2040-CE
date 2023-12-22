#ifndef _BoardLed_H
#define _BoardLed_H

#include "gpaddon.h"
#include "pico/stdlib.h"
#include "storagemanager.h"

#ifndef BOARD_LED_ENABLED
#define BOARD_LED_ENABLED 0
#endif

#ifndef BOARD_LEDS_PIN
#define BOARD_LEDS_PIN -1
#endif

#ifndef BOARD_LED_TYPE 
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_OFF
#endif  

// BoardLed Module Name
#define OnBoardLedName "OnBoardLed"

#define BOARD_LED_PIN 25
#define BLINK_INTERVAL_USB_UNMOUNTED 200
#define BLINK_INTERVAL_CONFIG_MODE 1000

class BoardLedAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // BoardLed Setup
	virtual void process();     // BoardLed Process
	virtual void preprocess() {}
	virtual std::string name() { return OnBoardLedName; }
private:
	OnBoardLedMode onBoardLedMode;
	bool isConfigMode;
	bool prevState;
	uint32_t timeSinceBlink;
};

#endif  // _BoardLed_H_