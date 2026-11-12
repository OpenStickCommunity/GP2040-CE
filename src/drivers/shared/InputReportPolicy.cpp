/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/shared/InputReportPolicy.h"
#include "storagemanager.h"
#include "addons/he_trigger.h"

static bool isDigitalAxisAction(GpioAction action) {
    switch (action) {
        case ANALOG_DIRECTION_LS_X_NEG:
        case ANALOG_DIRECTION_LS_X_POS:
        case ANALOG_DIRECTION_LS_Y_NEG:
        case ANALOG_DIRECTION_LS_Y_POS:
        case ANALOG_DIRECTION_RS_X_NEG:
        case ANALOG_DIRECTION_RS_X_POS:
        case ANALOG_DIRECTION_RS_Y_NEG:
        case ANALOG_DIRECTION_RS_Y_POS:
            return true;
        default:
            return false;
    }
}

bool hasDigitalAxisMappings() {
    Storage &storage = Storage::getInstance();
    GpioMappingInfo const *pinMappings = storage.getProfilePinMappings();
    bool dualDirectionalMapped = false;
    for (Pin_t pin = 0; pin < static_cast<Pin_t>(NUM_BANK0_GPIOS); ++pin) {
        GpioAction const action = pinMappings[pin].action;
        dualDirectionalMapped |= action == BUTTON_PRESS_DDI_UP || action == BUTTON_PRESS_DDI_DOWN ||
            action == BUTTON_PRESS_DDI_LEFT || action == BUTTON_PRESS_DDI_RIGHT;
        if (isDigitalAxisAction(action)) {
            return true;
        }
    }

    AddonOptions const &addons = storage.getAddonOptions();
    if (dualDirectionalMapped && addons.dualDirectionalOptions.enabled &&
        addons.dualDirectionalOptions.dpadMode != DPAD_MODE_DIGITAL) {
        return true;
    }
    if (addons.heTriggerOptions.enabled) {
        HETriggerOptions const &heTriggers = addons.heTriggerOptions;
        for (uint32_t i = 0; i < HETRIGGER_COUNT; ++i) {
            if (isDigitalAxisAction(heTriggers.triggers[i].action) && HETriggerAddon::isTriggerScanned(heTriggers, i)) {
                return true;
            }
        }
    }

    if (!storage.getGamepadOptions().lockHotkeys) {
        HotkeyOptions const &hotkeys = storage.getHotkeyOptions();
#define HOTKEY_ACTION(options, allocation, rule, type, field, tag, other) options.field.action,
        GamepadHotkey const actions[] = { HotkeyOptions_FIELDLIST(HOTKEY_ACTION, hotkeys) };
#undef HOTKEY_ACTION
        for (GamepadHotkey const action : actions) {
            switch (action) {
                case HOTKEY_LS_UP:
                case HOTKEY_LS_DOWN:
                case HOTKEY_LS_LEFT:
                case HOTKEY_LS_RIGHT:
                case HOTKEY_RS_UP:
                case HOTKEY_RS_DOWN:
                case HOTKEY_RS_LEFT:
                case HOTKEY_RS_RIGHT:
                    return true;
                default:
                    break;
            }
        }
    }
    return false;
}
