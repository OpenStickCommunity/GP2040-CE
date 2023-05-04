/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

// GP2040 Classes
#include "gamepad.h"
#include "addonmanager.h"

#include "pico/types.h"

class GP2040 {
public:
	GP2040();
    ~GP2040();
    void setup();           // setup core0
    void run();             // loop core0
private:
    uint64_t nextRuntime;
    Gamepad snapshot;
    AddonManager addons;

    struct WebConfigHotkey {
        WebConfigHotkey();
        void process(Gamepad* gamepad, bool configMode);

        bool active;

        absolute_time_t noButtonsPressedTimeout;
        uint16_t webConfigHotkeyMask;
        absolute_time_t webConfigHotkeyHoldTimeout;
    };
    WebConfigHotkey webConfigHotkey;

    enum class BootAction {
        NONE,
        ENTER_WEBCONFIG_MODE,
        ENTER_USB_MODE,
        SET_INPUT_MODE_HID,
        SET_INPUT_MODE_SWITCH,
        SET_INPUT_MODE_XINPUT,
        SET_INPUT_MODE_KEYBOARD,
        SET_INPUT_MODE_PS4
    };
    static BootAction getBootAction();
};

#endif
