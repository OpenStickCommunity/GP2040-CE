/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef USB_DRIVER_H_
#define USB_DRIVER_H_

#include "GamepadDescriptors.h"

#ifdef __cplusplus
extern "C"
{
#endif

	InputMode get_input_mode(void);
	void initialize_driver(InputMode mode);
	void send_report(uint8_t *report, uint8_t report_size);

#ifdef __cplusplus
}
#endif

#endif
