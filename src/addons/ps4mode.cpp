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

bool PS4ModeAddon::available() {
  const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
	return options.enabled
      && options.rsaN.size == sizeof(options.rsaN.bytes)
      && options.rsaE.size == sizeof(options.rsaE.bytes)
      && options.rsaD.size == sizeof(options.rsaD.bytes)
      && options.rsaP.size == sizeof(options.rsaP.bytes)
      && options.rsaQ.size == sizeof(options.rsaQ.bytes)
      && options.rsaDP.size == sizeof(options.rsaDP.bytes)
      && options.rsaDQ.size == sizeof(options.rsaDQ.bytes)
      && options.rsaQP.size == sizeof(options.rsaQP.bytes)
      && options.rsaRN.size == sizeof(options.rsaRN.bytes);
}

void PS4ModeAddon::setup() {
    const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;

    memcpy(bytesN, options.rsaN.bytes, options.rsaN.size);
    memcpy(bytesE, options.rsaE.bytes, options.rsaE.size);
    memcpy(bytesD, options.rsaD.bytes, options.rsaD.size);
    memcpy(bytesP, options.rsaP.bytes, options.rsaP.size);
    memcpy(bytesQ, options.rsaQ.bytes, options.rsaQ.size);
    memcpy(bytesDP, options.rsaDP.bytes, options.rsaDP.size);
    memcpy(bytesDQ, options.rsaDQ.bytes, options.rsaDQ.size);
    memcpy(bytesQP, options.rsaQP.bytes, options.rsaQP.size);
    memcpy(bytesRN, options.rsaRN.bytes, options.rsaRN.size);

    rsa_context = {
      .ver = 0,
      .len = 256,
      .N =  { .s=1, .n=64, .p=bytesN  },
      .E =  { .s=1, .n=1,  .p=bytesE  },
      .D =  { .s=1, .n=64, .p=bytesD  },
      .P =  { .s=1, .n=32, .p=bytesP  },
      .Q =  { .s=1, .n=32, .p=bytesQ  },
      .DP = { .s=1, .n=32, .p=bytesDP },
      .DQ = { .s=1, .n=32, .p=bytesDQ },
      .QP = { .s=1, .n=32, .p=bytesQP },
      .RN = { .s=1, .n=64, .p=bytesRN },
      .RP = { .s=0, .n=0, .p=nullptr },
      .RQ = { .s=0, .n=0, .p=nullptr },
      .Vi = { .s=0, .n=0, .p=nullptr },
      .Vf = { .s=0, .n=0, .p=nullptr },
      .padding = MBEDTLS_RSA_PKCS_V21, .hash_id = MBEDTLS_MD_SHA256,
    };
}

void PS4ModeAddon::process() {
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
      memcpy(&ps4_auth_buffer[offset], options.serial.bytes, 16);
      offset += 16;
      mbedtls_mpi* mpi = static_cast<mbedtls_mpi*>(&rsa_context.N);
      mbedtls_mpi_write_binary(mpi, &ps4_auth_buffer[offset], 256);
      offset += 256;
      mpi = static_cast<mbedtls_mpi*>(&rsa_context.E);
      mbedtls_mpi_write_binary(mpi, &ps4_auth_buffer[offset], 256);
      offset += 256;
      memcpy(&ps4_auth_buffer[offset], options.signature.bytes, 256);
      offset += 256;
      memset(&ps4_auth_buffer[offset], 0, 24);

      PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready; // signed and ready to party
    }
}
