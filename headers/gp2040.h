/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

#include <map>

// GP2040 Classes
#include "gamepad.h"
#include "addonmanager.h"
#include "peripheralmanager.h"

#include "pico/types.h"

class GP2040 {
public:
	GP2040();
    ~GP2040();
    void setup();           // setup core0
    void run();             // loop core0
private:
    Gamepad snapshot;
    AddonManager addons;
    PeripheralManager peripherals;

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
        SET_INPUT_MODE_HID,
        SET_INPUT_MODE_SWITCH,
        SET_INPUT_MODE_XINPUT,
        SET_INPUT_MODE_KEYBOARD,
        SET_INPUT_MODE_PS4,
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

    // GPIO manipulation for setup and profile reinit
    void initializeStandardGpio();
    void deinitializeStandardGpio();

    // input mask, action
    std::map<uint32_t, int32_t> bootActions;
};

#endif
