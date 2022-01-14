/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "device/usbd_pvt.h"
#include "descriptors/HIDDescriptors.h"
#include "descriptors/SwitchDescriptors.h"

extern const usbd_class_driver_t hid_driver;

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size);
