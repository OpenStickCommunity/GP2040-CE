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

    enum class BootAction {
        NONE,
        ENTER_WEBCONFIG_MODE,
        ENTER_USB_MODE,
        SET_INPUT_MODE_SWITCH,
        SET_INPUT_MODE_XINPUT,
        SET_INPUT_MODE_KEYBOARD,
        SET_INPUT_MODE_GENERIC,
        SET_INPUT_MODE_PS3,
        SET_INPUT_MODE_PS4,
        SET_INPUT_MODE_PS5,
        SET_INPUT_MODE_XBONE,
        SET_INPUT_MODE_NEOGEO,
        SET_INPUT_MODE_MDMINI,
        SET_INPUT_MODE_PCEMINI,
        SET_INPUT_MODE_EGRET,
        SET_INPUT_MODE_ASTRO,
        SET_INPUT_MODE_PSCLASSIC,
        SET_INPUT_MODE_XBOXORIGINAL
    };
    BootAction getBootAction();
    void getReinitGamepad(Gamepad * gamepad);

    // GPIO manipulation for setup and profile reinit
    void initializeStandardGpio();
    void deinitializeStandardGpio();

    // event handling checking
    void checkRawState(GamepadState prevState, GamepadState currState);
    void checkProcessedState(GamepadState prevState, GamepadState currState);

    // input mask, action
    std::map<uint32_t, int32_t> bootActions;

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
