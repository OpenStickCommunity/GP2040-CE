/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef USB_DRIVER_H_
#define USB_DRIVER_H_

#include "GamepadDescriptors.h"

typedef enum
{
	USB_MODE_HID,
	USB_MODE_NET,
} UsbMode;

#ifdef __cplusplus
extern "C"
{
#endif

	InputMode get_input_mode(void);
	void initialize_driver(InputMode mode);
	void receive_report(uint8_t *buffer);
	void send_report(void *report, uint16_t report_size);

#ifdef __cplusplus
}
#endif

#endif
