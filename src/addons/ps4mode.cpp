/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "addons/ps4mode.h"

#include "ps4_driver.h"

#include "mbedtls/error.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

bool PS4ModeAddon::available() {
	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();
	return addonOptions.PS4ModeAddonEnabled;
}

void PS4ModeAddon::setup() {
    PS4Options * ps4Options = Storage::getInstance().getPS4Options();

    rsa_context = {
      .ver = 0,
      .len = 256,
      .N = { .s=1, .n=64, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_n) },
      .E = { .s=1, .n=1, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_e) },
      .D = { .s=1, .n=64, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_d) },
      .P = { .s=1, .n=32, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_p) },
      .Q = { .s=1, .n=32, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_q) },
      .DP = { .s=1, .n=32, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_dp) },
      .DQ = { .s=1, .n=32, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_dq) },
      .QP = { .s=1, .n=32, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_qp) },
      .RN = { .s=1, .n=64, .p=const_cast<mbedtls_mpi_uint*>(ps4Options->rsa_rn) },
      .RP = { .s=0, .n=0, .p=nullptr },
      .RQ = { .s=0, .n=0, .p=nullptr },
      .Vi = { .s=0, .n=0, .p=nullptr },
      .Vf = { .s=0, .n=0, .p=nullptr },
      .padding = MBEDTLS_RSA_PKCS_V21, .hash_id = MBEDTLS_MD_SHA256,
    };
}

void PS4ModeAddon::process() {
    PS4Options * ps4Options = Storage::getInstance().getPS4Options();

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

      uint8_t nonce_signature[256];
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
              nonce_signature);

      if ( rss_error < 0 ) {
        return;
      }
      // copy the parts into our authentication buffer
      int offset = 0;
      memcpy(&ps4_auth_buffer[offset], nonce_signature, 256);
      offset += 256;
      memcpy(&ps4_auth_buffer[offset], ps4Options->serial, 16);
      offset += 16;
      mbedtls_mpi* mpi = static_cast<mbedtls_mpi*>(&rsa_context.N);
      mbedtls_mpi_write_binary(mpi, &ps4_auth_buffer[offset], 256);
      offset += 256;
      mpi = static_cast<mbedtls_mpi*>(&rsa_context.E);
      mbedtls_mpi_write_binary(mpi, &ps4_auth_buffer[offset], 256);
      offset += 256;
      memcpy(&ps4_auth_buffer[offset], ps4Options->signature, 256);
      offset += 256;
      memset(&ps4_auth_buffer[offset], 0, 24);

      PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready; // signed and ready to party
    }
}
