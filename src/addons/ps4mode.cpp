/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "addons/ps4mode.h"
#include "helper.h"
#include "config.pb.h"

#include "ps4_driver.h"

#include "mbedtls/error.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#define NEW_CONFIG_MPI(name, buf, size) \
	mbedtls_mpi_uint *bytes ## name = new mbedtls_mpi_uint[size / sizeof(mbedtls_mpi_uint)]; \
	mbedtls_mpi name = { .s=1, .n=size / sizeof(mbedtls_mpi_uint), .p=bytes ## name }; \
	memcpy(bytes ## name, buf, size);

#define DELETE_CONFIG_MPI(name) delete bytes ## name;

bool PS4ModeAddon::available() {
  const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
	return options.enabled
		&& options.serial.size == sizeof(options.serial.bytes)
		&& options.signature.size == sizeof(options.signature.bytes)
		&& options.rsaN.size == sizeof(options.rsaN.bytes)
		&& options.rsaE.size == sizeof(options.rsaE.bytes)
		&& options.rsaP.size == sizeof(options.rsaP.bytes)
		&& options.rsaQ.size == sizeof(options.rsaQ.bytes);
}

void PS4ModeAddon::setup() {
	const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
	ready = false;

	NEW_CONFIG_MPI(N, options.rsaN.bytes, options.rsaN.size)
	NEW_CONFIG_MPI(E, options.rsaE.bytes, options.rsaE.size)
	NEW_CONFIG_MPI(P, options.rsaP.bytes, options.rsaP.size)
	NEW_CONFIG_MPI(Q, options.rsaQ.bytes, options.rsaQ.size)

	mbedtls_rsa_init(&rsa_context, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

	if (mbedtls_rsa_import(&rsa_context, &N, &P, &Q, nullptr, &E) == 0 &&
			mbedtls_rsa_complete(&rsa_context) == 0) {
		ready = true;
	}

	DELETE_CONFIG_MPI(N)
	DELETE_CONFIG_MPI(E)
	DELETE_CONFIG_MPI(P)
	DELETE_CONFIG_MPI(Q)
}

void PS4ModeAddon::process() {
	if (!ready) {
		return;
	}

	const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;

	int rss_error = 0;

	// Check to see if the PS4 Authentication needs work
	if ( PS4Data::getInstance().ps4State == PS4State::nonce_ready ) {

		// Generate some random for RNG
		srand(getMillis());
		auto rng = [](void*p_rng, unsigned char* p, size_t len) {
			(void) p_rng;
			p[0] = rand();
			return 0;
		};

		uint8_t hashed_nonce[32];
		uint8_t * nonce_buffer = PS4Data::getInstance().nonce_buffer;
		uint8_t * ps4_auth_buffer = PS4Data::getInstance().ps4_auth_buffer;

		// Sign the nonce now that we got it!
		//
		if ( mbedtls_sha256_ret(nonce_buffer, 256, hashed_nonce, 0) < 0 ) {
			return;
		}

		rss_error = mbedtls_rsa_rsassa_pss_sign(&rsa_context, rng, nullptr,
				MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,
				32, hashed_nonce,
				&ps4_auth_buffer[0]);

		if ( rss_error < 0 ) {
			return;
		}
		// copy the parts into our authentication buffer
		size_t offset = 256;
		memcpy(&ps4_auth_buffer[offset], options.serial.bytes, 16);
		offset += 16;
		mbedtls_rsa_export_raw(
			&rsa_context,
			&ps4_auth_buffer[offset], 256,
			nullptr, 0,
			nullptr, 0,
			nullptr, 0,
			&ps4_auth_buffer[offset+256], 256
		);
		offset += 512;
		memcpy(&ps4_auth_buffer[offset], options.signature.bytes, 256);
		offset += 256;
		memset(&ps4_auth_buffer[offset], 0, 24);

		PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready; // signed and ready to party
	}
}
