/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef HID_DRIVER_H_
#define HID_DRIVER_H_

#include "device/usbd_pvt.h"
#include "ps3_device.h"
#include "switch_device.h"

extern const usbd_class_driver_t hid_driver;

#ifdef __cplusplus
extern "C" {
#endif

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size);

#ifdef __cplusplus
}
#endif

#endif
