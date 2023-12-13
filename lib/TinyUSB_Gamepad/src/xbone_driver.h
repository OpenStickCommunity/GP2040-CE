/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "tusb.h"
#include "device/usbd_pvt.h"

#include "gamepad/descriptors/XBOneDescriptors.h"

#include "xgip_protocol.h"

#define XBONE_OUT_SIZE 64

// USB endpoint state vars
extern uint8_t xbone_out_buffer[XBONE_OUT_SIZE];
extern const usbd_class_driver_t xbone_driver;

//extern void send_xbhost_report(void *report, uint16_t report_size);

extern bool send_xbone_report(void *report, uint16_t report_size);
extern void receive_xbone_report(void);
extern bool xbone_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request);

// Required for large chunked data
extern void queue_xbone_report(void *report, uint16_t report_size);

extern uint32_t timer_wait_for_auth;

void tick_xbone_usb();

typedef enum {
	reset_state = 0,
	ready_to_announce = 1,
	send_descriptor = 2,
	send_auth_console_to_dongle = 3,
	send_auth_dongle_to_console = 4,
	wait_auth_console_to_dongle = 5,
	wait_auth_dongle_to_console = 6,
	idle_state = 7
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
	bool auth_completed;

	// Auth Buffer
	uint8_t authBuffer[1024];
	uint8_t authSequence;
	uint16_t authLen;
	uint8_t authType;

private:
	XboxOneData() {
		xboneState = XboxOneState::reset_state;
		authLen = 0;
		authSequence = 0;
		auth_completed = false;
	}
};
