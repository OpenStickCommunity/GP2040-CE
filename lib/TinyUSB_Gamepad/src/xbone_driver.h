/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "tusb.h"
#include "device/usbd_pvt.h"

#include "gamepad/descriptors/XBOneDescriptors.h"

#define XBONE_OUT_SIZE 64

// USB endpoint state vars
extern uint8_t xbone_out_buffer[XBONE_OUT_SIZE];
extern const usbd_class_driver_t xbone_driver;

bool send_xbone_report(void *report, uint8_t report_size);
void receive_xbone_report(void);
bool xbone_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request);

extern uint32_t timer_wait_for_auth;

void tick_xbone_usb();

typedef enum {
	reset_state = 0,
	ready_to_announce = 1,
	send_descriptor = 2,
	send_auth = 3,
	auth_complete = 4,
	idle_state = 5
} XboxOneState;

// Storage manager for board, LED options, and thread-safe settings
class XBOneData {
public:
	XBOneData(XBOneData const&) = delete;
	void operator=(XBOneData const&)  = delete;
	static XBOneData& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static XBOneData instance;
		return instance;
	}

	XboxOneState xboneState;
	
private:
	XBOneData() {
		xboneState = XboxOneState::reset_state;
	}
};
