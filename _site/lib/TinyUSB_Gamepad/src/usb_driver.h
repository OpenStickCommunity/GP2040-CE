/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "gamepad/GamepadDescriptors.h"
#include "enums.pb.h"

typedef enum
{
	USB_MODE_HID,
	USB_MODE_NET,
} UsbMode;

InputMode get_input_mode(void);
bool get_usb_mounted(void);
bool get_usb_suspended(void);
void initialize_driver(InputMode mode);
void receive_report(uint8_t *buffer);
bool send_report(void *report, uint16_t report_size);

