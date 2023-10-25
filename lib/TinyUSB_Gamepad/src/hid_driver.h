/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "device/usbd_pvt.h"
#include "gamepad/descriptors/HIDDescriptors.h"
#include "gamepad/descriptors/SwitchDescriptors.h"

extern const usbd_class_driver_t hid_driver;
extern void sof_callback(uint8_t rhport, uint32_t frame_count);

bool send_hid_report(uint8_t report_id, void *report, uint8_t report_size);
bool send_keyboard_report(void *report);
