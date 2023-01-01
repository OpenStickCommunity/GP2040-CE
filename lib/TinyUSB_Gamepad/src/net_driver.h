/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "device/usbd_pvt.h"
#include "class/net/net_device.h"

extern const usbd_class_driver_t net_driver;

bool send_hid_report(uint8_t report_id, uint8_t *report, uint8_t report_size);
