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

extern void send_xbhost_report(void *report, uint8_t report_size);

extern bool send_xbone_report(void *report, uint8_t report_size);
extern void receive_xbone_report(void);
extern bool xbone_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
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
class XboxOneData {
public:
	XboxOneData(XboxOneData const&) = delete;
	void operator=(XboxOneData const&)  = delete;
	static XboxOneData& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static XboxOneData instance;
		return instance;
	}

	XboxOneState xboneState;

	// Console-to-Host e.g. Xbox One to MagicBoots
	//  Note: the Xbox One Passthrough can call send_xbone_report() directly but not the other way around
	uint8_t console_to_host[256]; 
	uint16_t console_to_host_len;
	bool console_to_host_ready;

	bool auth_completed;

private:
	XboxOneData() {
		xboneState = XboxOneState::reset_state;
		console_to_host_ready = false;
		console_to_host_len = 0;

		auth_completed = false;
	}
};
