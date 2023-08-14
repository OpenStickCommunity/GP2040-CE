/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "ps4_driver.h"

#include "CRC32.h"

#include "mbedtls/error.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#include <random>

uint8_t ps4_endpoint_in = 0;
uint8_t ps4_endpoint_out = 0;
uint8_t ps4_out_buffer[PS4_OUT_SIZE] = {};

// Controller descriptor (byte[4] = 0x00 for ps4, 0x07 for ps5)
static constexpr uint8_t output_0x03[] = {
	0x21, 0x27, 0x04, 0xcf, 0x00, 0x2c, 0x56,
    0x08, 0x00, 0x3d, 0x00, 0xe8, 0x03, 0x04, 0x00,
    0xff, 0x7f, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Nonce Page Size: 0x38 (56)
// Response Page Size: 0x38 (56)
static constexpr uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

static uint8_t cur_nonce_id = 1;

// debug
static int rss_error = 0;

static uint8_t send_nonce_part = 0;

struct PS4Key {
  unsigned char serial[16];
  unsigned char signature[256];
  struct mbedtls_rsa_context* rsa_context;
};

struct SignaturePart {
  size_t length;
  size_t (*function)(uint8_t * buf, size_t offset, const void* arg, size_t expected_size);
  const void* arg;
};

ssize_t get_ps4_report(uint8_t report_id, uint8_t * buf, uint16_t reqlen)
{
	uint8_t data[64] = {};
	uint32_t crc32;
	ps4_out_buffer[0] = report_id;
	switch(report_id) {
		// Controller Definition Report
		case PS4AuthReport::PS4_DEFINITION:
			if (reqlen != sizeof(output_0x03)) {
				return -1;
			}
			memcpy(buf, output_0x03, reqlen);
			buf[4] = (uint8_t)PS4Data::getInstance().ps4ControllerType; // Change controller type in definition
			return reqlen;
		// Use our private RSA key to sign the nonce and return chunks
		case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
			// We send 56 byte chunks back to the PS4, we've already calculated these
			data[0] = 0xF1;
			data[1] = cur_nonce_id;    // nonce_id
			data[2] = send_nonce_part; // next_part
			data[3] = 0;

			// 56 byte chunks
			memcpy(&data[4], &PS4Data::getInstance().ps4_auth_buffer[send_nonce_part*56], 56);

			// calculate the CRC32 of the buffer and write it back
			crc32 = CRC32::calculate(data, 60);
			memcpy(&data[60], &crc32, sizeof(uint32_t));
			memcpy(buf, &data[1], 63); // move data over to buffer
			if ( (++send_nonce_part) == 19 ) {
				PS4Data::getInstance().ps4State = PS4State::no_nonce;
				PS4Data::getInstance().authsent = true;
				send_nonce_part = 0;
			}
			return 63;
		// Are we ready to sign?
		case PS4AuthReport::PS4_GET_SIGNING_STATE:
      		data[0] = 0xF2;
			data[1] = cur_nonce_id;
			data[2] = PS4Data::getInstance().ps4State == PS4State::signed_nonce_ready ? 0 : 16; // 0 means auth is ready, 16 means we're still signing
			memset(&data[3], 0, 9);
			crc32 = CRC32::calculate(data, 12);
			memcpy(&data[12], &crc32, sizeof(uint32_t));
			memcpy(buf, &data[1], 15); // move data over to buffer
			return 15;
		case PS4AuthReport::PS4_RESET_AUTH: // Reset the Authentication
			if (reqlen != sizeof(output_0xf3)) {
				return -1;
			}
			memcpy(buf, output_0xf3, reqlen);
			PS4Data::getInstance().ps4State = PS4State::no_nonce;
			return reqlen;
		default:
			break;
	};
	return -1;
}

void set_ps4_report(uint8_t report_id, uint8_t const * data, uint16_t reqlen)
{
	uint8_t nonce_id;
	uint8_t nonce_page;
	uint32_t crc32;
	uint8_t buffer[64];
	uint8_t nonce[56]; // max nonce data
	uint16_t noncelen;
	uint16_t buflen;

	if (report_id == PS4AuthReport::PS4_SET_AUTH_PAYLOAD) {
		if (reqlen != 63 ) {
			return;
		}

		// Setup CRC32 buffer
		buffer[0] = report_id;
		memcpy(&buffer[1], data, reqlen);
		buflen = reqlen + 1;

		nonce_id = data[0];
		nonce_page = data[1];
		// data[2] is zero padding

		crc32 = CRC32::calculate(buffer, buflen-sizeof(uint32_t));
		if ( crc32 != *((unsigned int*)&buffer[buflen-sizeof(uint32_t)])) {
			return; // CRC32 failed on set report
		}

		// 256 byte nonce, with 56 byte packets leaves 24 extra bytes on the last packet?
		if ( nonce_page == 4 ) {
			// Copy/append data from buffer[4:64-28] into our nonce
			noncelen = 32; // from 4 to 64 - 24 - 4
			PS4Data::getInstance().nonce_id = nonce_id; // for pass-through only
		} else {
			// Copy/append data from buffer[4:64-4] into our nonce
			noncelen = 56;
			// from 4 to 64 - 4
		}

		memcpy(nonce, &buffer[4], noncelen);
		save_nonce(nonce_id, nonce_page, nonce, noncelen);
	}
}


void save_nonce(uint8_t nonce_id, uint8_t nonce_page, uint8_t * buffer, uint16_t buflen) {
	if ( nonce_page != 0 && nonce_id != cur_nonce_id ) {
		PS4Data::getInstance().ps4State = PS4State::no_nonce;
		return; // setting nonce with mismatched id
	}

	memcpy(&PS4Data::getInstance().nonce_buffer[nonce_page*56], buffer, buflen);
	if ( nonce_page == 4 ) {
		PS4Data::getInstance().ps4State = PS4State::nonce_ready;
	} else if ( nonce_page == 0 ) {
		cur_nonce_id = nonce_id;
		PS4Data::getInstance().ps4State = PS4State::receiving_nonce;
	}
}

void receive_ps4_report(void)
{
	if (
		tud_ready() &&
		(ps4_endpoint_out != 0) && (!usbd_edpt_busy(0, ps4_endpoint_out)))
	{
		usbd_edpt_claim(0, ps4_endpoint_out);									 // Take control of OUT endpoint
		usbd_edpt_xfer(0, ps4_endpoint_out, ps4_out_buffer, PS4_OUT_SIZE);		 // Retrieve report buffer
		usbd_edpt_release(0, ps4_endpoint_out);									 // Release control of OUT endpoint
	}
}

bool send_ps4_report(void *report, uint8_t report_size)
{
	bool sent = false;

	if (
		tud_ready() &&											// Is the device ready?
		(ps4_endpoint_in != 0) && (!usbd_edpt_busy(0, ps4_endpoint_in)) // Is the IN endpoint available?
	)
	{
		usbd_edpt_claim(0, ps4_endpoint_in);								// Take control of IN endpoint
		usbd_edpt_xfer(0, ps4_endpoint_in, (uint8_t *)report, report_size); // Send report buffer
		usbd_edpt_release(0, ps4_endpoint_in);								// Release control of IN endpoint
		sent = true;
	}

	return sent;
}

const usbd_class_driver_t ps4_driver =
	{
#if CFG_TUSB_DEBUG >= 2
		.name = "PS4",
#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL};
