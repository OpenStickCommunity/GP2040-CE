#ifndef _PS5AUTH_H_
#define _PS5AUTH_H_

#include "drivers/shared/gpauthdriver.h"

typedef enum {
    ps5_auth_idle = 0,
    ps5_auth_send_f0,
    ps5_auth_send_f0_wait,
    ps5_auth_recv_f1,
    ps5_auth_recv_f1_wait,
    ps5_auth_recv_f2_delay_500mS,
    ps5_auth_recv_f2,
    ps5_auth_recv_f2_wait,
} PS5AuthState; // PS5 Auth State for F0/F1/F2 auth

typedef struct {
    uint8_t hash_pending_buffer[64];
    uint8_t hash_finish_buffer[64];
    uint8_t auth_buffer[64];
    uint64_t auth_recv_f2_us;
    bool dongle_ready = false;
    bool hash_pending = false;
    bool hash_ready = false;
    PS5AuthState ps5_passthrough_state;

    // Encrypted key data from Mayflash S5 device
	uint8_t encrypted_keys[12]; // 12-byte encrypted keys
	uint8_t incount[4];			// 4-byte incount
	uint8_t aes_cmac[8];		// 8-byte AES CMAC
	bool keys_ready;			// Flag indicating encrypted keys are ready
} PS5AuthData;

class PS5Auth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    PS5AuthData * getAuthData() { return &ps5AuthData; }
private:
    PS5AuthData ps5AuthData;
};

#endif
