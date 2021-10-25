/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef NET_DRIVER_H_
#define NET_DRIVER_H_

#include "device/usbd_pvt.h"
#include "class/net/net_device.h"

extern const usbd_class_driver_t net_driver;

#ifdef __cplusplus
extern "C" {
#endif

bool send_hid_report(uint8_t report_id, uint8_t *report, uint8_t report_size);

#ifdef __cplusplus
}
#endif

#endif
