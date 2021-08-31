/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef USB_DRIVER_H_
#define USB_DRIVER_H_

typedef enum
{
	XINPUT,
	SWITCH,
	PS3,
} InputMode;

extern InputMode current_input_mode;

#ifdef __cplusplus
extern "C"
{
#endif

	void initialize_driver(void);
	void send_report(void *report, uint8_t report_size);

#ifdef __cplusplus
}
#endif

#endif
