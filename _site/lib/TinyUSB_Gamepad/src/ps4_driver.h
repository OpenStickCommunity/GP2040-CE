/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include "tusb.h"
#include "device/usbd_pvt.h"

#include "gamepad/descriptors/PS4Descriptors.h"

#include "enums.pb.h"

#define PS4_OUT_SIZE 64

typedef enum
{
	PS4_DEFINITION           = 0x03,    // PS4 Controller Definition
	PS4_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
	PS4_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
	PS4_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
	PS4_RESET_AUTH           = 0xF3     // Unknown (PS4 Report 0xF3)
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
	signed_nonce_ready = 3,
	sending_nonce = 4
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
	uint8_t nonce_buffer[256];
	uint8_t nonce_id; // used in pass-through mode

	// Send back in 56 byte chunks:
	//    256 byte - nonce signature
	//    16 byte  - ps4 serial
	//    256 byte - RSA N
	//    256 byte - RSA E
	//    256 byte - ps4 signature
	//    24 byte  - zero padding

	// buffer = 256 + 16 + 256 + 256 + 256 + 24
	// == 1064 bytes (almost 1 kb)
	uint8_t ps4_auth_buffer[1064];
	uint32_t ps4ControllerType;

private:
	PS4Data() {
		ps4State = PS4State::no_nonce;
		authsent = false;
		memset(nonce_buffer, 0, 256);
		memset(ps4_auth_buffer, 0, 1064);
		ps4ControllerType = PS4ControllerType::PS4_CONTROLLER;
	}
};
