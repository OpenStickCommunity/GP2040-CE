/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "tusb.h"
#include "device/usbd_pvt.h"
#include "ds4crypto.h"

#include "gamepad/descriptors/PS4Descriptors.h"

#define PS4_OUT_SIZE 64

typedef enum
{
	PS4_GET_FEATURE          = 0x03,    // Query enabled PS4 controller features
	PS4_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
	PS4_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
	PS4_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
	PS4_RESET_AUTH           = 0xF3     // Reset auth state and report buffer size
} PS4AuthReport;

// USB endpoint state vars
extern const usbd_class_driver_t ps4_driver;

ssize_t get_ps4_report(uint8_t report_id, uint8_t * buf, uint16_t reqlen);
void set_ps4_report(uint8_t report_id, uint8_t const * buf, uint16_t reqlen);
void receive_ps4_report(void);
bool send_ps4_report(void *report, uint8_t report_size);
void save_nonce(uint8_t nonce_id, uint8_t nonce_page, uint8_t * data, uint16_t size);

typedef enum {
	no_nonce = 0,
	receiving_nonce = 1,
	nonce_ready = 2,
	signed_nonce_ready = 3
} PS4State;

// Storage manager for board, LED options, and thread-safe settings
class PS4Data {
public:
	PS4Data(PS4Data const&) = delete;
	void operator=(PS4Data const&)  = delete;
	static PS4Data& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static PS4Data instance;
		return instance;
	}

	PS4State ps4State;
	bool authsent;
	NonceBuffer nonce_buffer;
	ResponseBuffer ps4_auth_buffer;
private:
	PS4Data() {
		ps4State = PS4State::no_nonce;
		authsent = false;
		memset(nonce_buffer, 0, sizeof(nonce_buffer));
		memset(ps4_auth_buffer.data, 0, sizeof(ps4_auth_buffer.data));
	}
};
