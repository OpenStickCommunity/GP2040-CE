/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef XINPUT_DRIVER_H_
#define XINPUT_DRIVER_H_

#include <stdint.h>
#include "tusb.h"
#include "device/usbd_pvt.h"

#include "XInputDescriptors.h"

// USB endpoint state vars
extern uint8_t endpoint_in;
extern uint8_t endpoint_out;

extern const usbd_class_driver_t xinput_driver;

#ifdef __cplusplus
extern "C" {
#endif

bool send_xinput_report(void *report, uint8_t report_size);

#ifdef __cplusplus
}
#endif

#endif
