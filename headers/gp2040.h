/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef GP2040_H_
#define GP2040_H_

#include <map>

// GP2040 Classes
#include "gamepad.h"
#include "addonmanager.h"
#include "eventmanager.h"
#include "gpdriver.h"

#include "pico/types.h"

class GP2040 {
public:
    GP2040(){}
    ~GP2040(){}
    void setup();           // setup core0
    void run();             // loop core0
private:
    Gamepad snapshot;
    AddonManager addons;
    // GPIO debouncer
    void debounceGpioGetAll();
    Mask_t buttonGpios;
    uint32_t gpioDebounceTime[NUM_BANK0_GPIOS];

    struct RebootHotkeys {
        RebootHotkeys();
        void process(Gamepad* gamepad, bool configMode);

        bool active;

        absolute_time_t noButtonsPressedTimeout;
        uint16_t webConfigHotkeyMask;
        uint16_t bootselHotkeyMask;
        absolute_time_t rebootHotkeysHoldTimeout;
    };
    RebootHotkeys rebootHotkeys;

    enum class BootActionType {
			ENTER_USB_MODE,
			SET_INPUT_MODE
		};

		struct BootAction {
			BootActionType type;
			InputMode inputMode;
			uint32_t profileNumber;
		};

		BootAction getGpioMappedBootAction();
		BootAction getButtonMappedBootAction();

    void getReinitGamepad(Gamepad * gamepad);

    // GPIO manipulation for setup and profile reinit
    void initializeStandardGpio();
    void deinitializeStandardGpio();

    // event handling checking
    void checkRawState(GamepadState prevState, GamepadState currState);
    void checkProcessedState(GamepadState prevState, GamepadState currState);

    void checkSaveRebootState();
    bool saveRequested = false;
    bool forceSave = false;
    bool saveSuccessful = false;
    void handleStorageSave(GPEvent* e);

    bool rebootRequested = false;
    void handleSystemReboot(GPEvent* e);

    System::BootMode rebootMode = System::BootMode::DEFAULT;
};

#endif
