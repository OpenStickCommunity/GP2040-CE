// Format and derivation based on amiitool, Copyright 2015-2017 Marcos Del Sol Vives,
// Copyright 2016 javiMaD, used under the MIT license (see AMIITOOL_LICENSE).
#include "addons/amiibo_crypto.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include <string.h>

static bool hmac(const uint8_t *key, const uint8_t *data, size_t size, uint8_t *out) {
    return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), key, 16, data, size, out) == 0;
}

bool amiiboKeyValid(const uint8_t *key, bool locked) {
    if (!key || key[31] > 16) return false;
    const char *type = locked ? "locked secret" : "unfixed infos";
    return memcmp(key + 16, type, 14) == 0;
}

static bool derive(const uint8_t *key, const uint8_t *data, uint8_t *derived) {
    uint8_t seed[64] = {}, prepared[80] = {}, block[32];
    memcpy(seed, data + 0x29, 2);
    memcpy(seed + 16, data + 0x1D4, 8);
    memcpy(seed + 24, data + 0x1D4, 8);
    memcpy(seed + 32, data + 0x1E8, 32);
    size_t size = 2;
    memcpy(prepared + size, key + 16, 14); size += 14;
    memcpy(prepared + size, seed, 16 - key[31]); size += 16 - key[31];
    memcpy(prepared + size, key + 32, key[31]); size += key[31];
    memcpy(prepared + size, seed + 16, 16); size += 16;
    for (size_t i = 0; i < 32; i++) prepared[size++] = seed[32 + i] ^ key[48 + i];
    if (!hmac(key, prepared, size, block)) return false;
    memcpy(derived, block, 32);
    prepared[1] = 1;
    if (!hmac(key, prepared, size, block)) return false;
    memcpy(derived + 32, block, 16);
    return true;
}

static void unpackLayout(const uint8_t *tag, uint8_t *data) {
    memcpy(data, tag + 8, 8);
    memcpy(data + 8, tag + 0x80, 32);
    memcpy(data + 0x28, tag + 0x10, 0x24);
    memcpy(data + 0x4C, tag + 0xA0, 0x168);
    memcpy(data + 0x1B4, tag + 0x34, 32);
    memcpy(data + 0x1D4, tag, 8);
    memcpy(data + 0x1DC, tag + 0x54, 0x2C);
}

static void packLayout(const uint8_t *data, uint8_t *tag) {
    memcpy(tag + 8, data, 8);
    memcpy(tag + 0x80, data + 8, 32);
    memcpy(tag + 0x10, data + 0x28, 0x24);
    memcpy(tag + 0xA0, data + 0x4C, 0x168);
    memcpy(tag + 0x34, data + 0x1B4, 32);
    memcpy(tag, data + 0x1D4, 8);
    memcpy(tag + 0x54, data + 0x1DC, 0x2C);
}

static bool cipher(uint8_t *data, const uint8_t *keys) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    bool ok = mbedtls_aes_setkey_enc(&ctx, keys, 128) == 0;
    uint8_t counter[16], stream[16];
    memcpy(counter, keys + 16, 16);
    for (size_t offset = 0; ok && offset < 0x188; offset += 16) {
        ok = mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, counter, stream) == 0;
        if (ok) for (size_t i = 0; i < 16 && offset + i < 0x188; i++) data[0x2C + offset + i] ^= stream[i];
        for (int i = 15; i >= 0 && ++counter[i] == 0; i--) {}
    }
    mbedtls_aes_free(&ctx);
    return ok;
}

static bool authenticate(uint8_t *data, const uint8_t *dataKeys, const uint8_t *tagKeys) {
    return hmac(tagKeys + 32, data + 0x1D4, 0x34, data + 0x1B4)
        && hmac(dataKeys + 32, data + 0x29, 0x1DF, data + 8);
}

bool amiiboRandomize(const uint8_t *original540, uint8_t *output540,
    const uint8_t *unfixed80, const uint8_t *locked80, const uint8_t *uid7) {
    if (!original540 || !output540 || !uid7 || uid7[0] != 4
        || !amiiboKeyValid(unfixed80, false) || !amiiboKeyValid(locked80, true)) return false;
    uint8_t data[520], dataKeys[48], tagKeys[48], signatures[64];
    unpackLayout(original540, data);
    memcpy(signatures, data + 8, 32);
    memcpy(signatures + 32, data + 0x1B4, 32);
    if (!derive(unfixed80, data, dataKeys) || !derive(locked80, data, tagKeys)
        || !cipher(data, dataKeys) || !authenticate(data, dataKeys, tagKeys)) return false;
    uint8_t difference = 0;
    for (size_t i = 0; i < 32; i++) difference |= (signatures[i] ^ data[8 + i]) | (signatures[32 + i] ^ data[0x1B4 + i]);
    if (difference) return false;
    memcpy(data + 0x1D4, uid7, 3);
    data[0x1D7] = 0x88 ^ uid7[0] ^ uid7[1] ^ uid7[2];
    memcpy(data + 0x1D8, uid7 + 3, 4);
    data[0] = uid7[3] ^ uid7[4] ^ uid7[5] ^ uid7[6];
    if (!derive(unfixed80, data, dataKeys) || !derive(locked80, data, tagKeys)
        || !authenticate(data, dataKeys, tagKeys) || !cipher(data, dataKeys)) return false;
    memcpy(output540, original540, 540);
    packLayout(data, output540);
    return true;
}
