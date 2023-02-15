#ifndef TURBO_H_
#define TURBO_H_

#include "gpaddon.h"

#ifndef TURBO_ENABLED
#define TURBO_ENABLED true
#endif

#ifndef DEFAULT_SHOT_PER_SEC
#define DEFAULT_SHOT_PER_SEC 15
#endif  // DEFAULT_SHOT_PER_SEC

// TURBO Button Mask
#define TURBO_BUTTON_MASK (GAMEPAD_MASK_B1 | GAMEPAD_MASK_B2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4 | \
                            GAMEPAD_MASK_L1 | GAMEPAD_MASK_R1 | GAMEPAD_MASK_L2 | GAMEPAD_MASK_R2)

// TURBO LED
#ifndef TURBO_LED_PIN
#define TURBO_LED_PIN   -1
#endif

// Turbo Module Name
#define TurboName "Turbo"

class TurboInput : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // TURBO Button Setup
    virtual void preprocess() {}
	virtual void process();     // TURBO Setting of buttons (Enable/Disable)
    virtual std::string name() { return TurboName; }
private:
    virtual bool read();        // Get TURBO Button State
    virtual void debounce();    // TURBO Button Debouncer
    bool bDebState;             // Debounce TURBO Button State
    uint32_t uDebTime;          // Debounce TURBO Button Time
    uint16_t lastPressed;       // Last buttons pressed (for Turbo Enable)
    uint16_t lastDpad;          // Last d-pad pressed (for Turbo Change)
    uint16_t buttonsEnabled;    // Turbo Buttons Enabled
    uint32_t uIntervalMS;       // Turbo Interval
    bool bTurboState;           // Turbo Buttons State
    bool bTurboFlicker;         // Turbo Enable Buttons Toggle OFF Flag ??
    uint32_t nextTimer;         // Turbo Timer
    uint8_t pinButtonTurbo;     // Turbo Button Pin
};
#endif  // TURBO_H_
