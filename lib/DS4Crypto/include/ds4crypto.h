#ifndef _DS4CRYPTO_H_
#define _DS4CRYPTO_H_

#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
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

/**
 * \brief DS4Key context class.
 * Validates a DS4Key block and uses it for authentication.
 */
class LoadedDS4Key {
public:
	/** Steps the algorithm is at when an error happens. */
	enum class Step {
		/** Everything done without an error. */
		DONE,
		/** Internal failure. Used for errors during e.g. initializing md context. */
		INTERNAL,
		/** Parse the key for use with mbedtls_rsa. */
		LOAD_PARSE_KEY,
		/** Generate D from NEPQ. */
		LOAD_COMPLETE_KEY,
		/** Verify the consistency of the keypair. */
		LOAD_VERIFY_KEY,
		/** Prepare hash from nonce. */
		SIGN_HASH,
		/** Sign the nonce hash. */
		SIGN_PSS,
		/** Export the DS4 identity and public key. */
		SIGN_EXPORT_IDENTITY,
	};
	/** Error struct. */
	struct Error {
		/** Step when the error happens. */
		Step step;
		/** mbedTLS error number. */
		int mbedTlsError;
	};
	LoadedDS4Key();
	~LoadedDS4Key();
	/**
	 * \brief Load a DS4Key blob and optionally perform extra validations on it.
	 * \param key The DS4Key blob. The buffer must be as large or larger than sizeof(DS4FullKeyBlock).
	 * \param skipValidation Whether to skip the extra validation.
	 * \return true if completed successfully.
	 */
	bool load(const uint8_t* key, bool skipValidation);
	/**
	 * \brief Sign a nonce with the loaded DS4Key.
	 * \param nonce The nonce buffer.
	 * \param response The response buffer.
	 * \return true if completed successfully.
	 */
	bool sign(const NonceBuffer nonce, ResponseBuffer& response);
	/**
	 * \brief Return the name of the step in LoadedDS4Key::error
	 * \return The name.
	 */
	const char* getStepName();
	/** Error object. */
	Error error;
private:
	/** A copy of the serial number block. */
	uint8_t serial[0x10];
	/** A copy of the identity signature block. */
	uint8_t identitySig[0x100];
	mbedtls_rsa_context rsa;
	mbedtls_md_context_t sha256;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context drbg;
	/** True if the object is ready to sign. */
	bool ready;
};

/**
 * \brief PassingLink key coverter.
 * Converts PassingLink 3-file style key to a single key block.
 *
 * This does not validate the key because LoadedDS4Key is responsible for validation.
 */
class PLKeyConverter {
public:
	/**
	 * \brief Parse and save serial number in hex format.
	 * \param buf Input string.
	 * \param len Strign size (without NUL terminator).
	 * \return mbedTLS error code, or 0 if executed successfully.
	 */
	int parseHexSerial(const char* buf, size_t len);
	/**
	 * \brief Parse and save private key
	 * \param buf Input buffer. Note that for PEM files it must terminate with NUL.
	 * \param len Size of input buffer.
	 * \return mbedTLS error code, or 0 if executed successfully.
	 */
	int parsePrivateKey(const uint8_t* buf, size_t len);
	/**
	 * \brief Set identity signature.
	 * \param buf Input buffer.
	 * \param len Number of bytes to copy. Must be equal to sizeof(DS4SignedIdentityBlock::identitySig).
	 * \return True if successful.
	 */
	bool setIdentitySig(const uint8_t* buf, size_t len);
	/**
	 * \brief Get the converted DS4Key blob as a read-only object.
	 * \return The DS4Key blob.
	 */
	const DS4FullKeyBlock& getKey() { return this->key; }
	/**
	 * \brief Get the converted DS4Key blob as a read-only memory view.
	 * \return The DS4Key blob as a byte buffer.
	 */
	const uint8_t* getKeyView() { return this->keyView; }
private:
	union {
		DS4FullKeyBlock key;
		uint8_t keyView[sizeof(DS4FullKeyBlock)];
	};
};
#endif // _DS4CRYPTO_H_
