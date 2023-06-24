#ifndef _DS4CRYPTO_H_
#define _DS4CRYPTO_H_

#include <mbedtls/rsa.h>
#include <mbedtls/md.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

struct DS4IdentityBlock {
	uint8_t serial[0x10];
	uint8_t modulus[0x100];
	uint8_t exponent[0x100];
};

struct DS4PrivateKeyBlock {
	uint8_t p[0x80];
	uint8_t q[0x80];
	uint8_t dp1[0x80];
	uint8_t dq1[0x80];
	uint8_t pq[0x80];
};

struct DS4SignedIdentityBlock {
	DS4IdentityBlock identity;
	uint8_t identitySig[0x100];
};

struct DS4FullKeyBlock {
	union {
		struct {
			DS4IdentityBlock identity;
			uint8_t identitySig[0x100];
		};
		DS4SignedIdentityBlock signedIdentity;
	};
	DS4PrivateKeyBlock privateKey;
};
static_assert(sizeof(DS4FullKeyBlock) == 1424, "Wrong DS4 key block size. Unexpected alignment?");

typedef DS4FullKeyBlock DS4FullKeyBlockUnaligned __attribute__((aligned(1)));

typedef uint8_t NonceBuffer[256];
	// Send back in 56 byte chunks:
	//    256 byte - nonce signature
	//    16 byte  - ps4 serial
	//    256 byte - RSA N
	//    256 byte - RSA E
	//    256 byte - ps4 signature
	//    24 byte  - zero padding

	// buffer = 256 + 16 + 256 + 256 + 256 + 24
	// == 1064 bytes (slightly more than 1 kb)
struct ResponseBuffer {
	union {
		struct {
			uint8_t sig[0x100];
			DS4IdentityBlock identity;
			uint8_t identitySig[0x100];
			uint8_t padding[24];
		};
	uint8_t data[1064];
	};
};
static_assert(sizeof(ResponseBuffer) ==
	sizeof(ResponseBuffer::sig) +
	sizeof(ResponseBuffer::identity) +
	sizeof(ResponseBuffer::identitySig) +
	sizeof(ResponseBuffer::padding)
, "ResponseBuffer byte view size mismatch with the struct.");

class LoadedDS4Key {
public:
	enum class Step {
		DONE,
		INTERNAL,
		LOAD_PARSE_KEY,
		LOAD_COMPLETE_KEY,
		LOAD_VERIFY_KEY,
		SIGN_HASH,
		SIGN_PSS,
		SIGN_EXPORT_IDENTITY,
	};
	struct Error {
		Step step;
		int mbedTlsError;
	};
	LoadedDS4Key();
	~LoadedDS4Key();
	bool load(const uint8_t* key, bool skipValidation);
	bool sign(const NonceBuffer nonce, ResponseBuffer& response);
	const char* getStepName();
	Error error;
private:
	uint8_t serial[0x10];
	uint8_t identitySig[0x100];
	mbedtls_rsa_context rsa;
	mbedtls_md_context_t sha256;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context drbg;
	bool ready;
};

#endif // _DS4CRYPTO_H_
