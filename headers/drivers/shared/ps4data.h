#ifndef _PS4_DATA_H_
#define _PS4_DATA_H_

typedef enum {
	no_nonce = 0,
	receiving_nonce = 1,
	nonce_ready = 2,
	signed_nonce_ready = 3,
	sending_nonce = 4
} PS4State;

typedef enum
{
	PS4_DEFINITION           = 0x03,    // PS4 Controller Definition
	PS4_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
	PS4_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
	PS4_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
	PS4_RESET_AUTH           = 0xF3     // Unknown (PS4 Report 0xF3)
} PS4AuthReport;

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

#endif // _PS4_DATA_H_
