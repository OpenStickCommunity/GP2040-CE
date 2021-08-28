/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef STATE_CONVERTER_H_
#define STATE_CONVERTER_H_

#include "usb_driver.h"
#include "Gamepad.h"

void *fill_report(GamepadState *state, bool has_analog_triggers);
void *select_report(uint8_t *report_size, InputMode mode);

#endif
