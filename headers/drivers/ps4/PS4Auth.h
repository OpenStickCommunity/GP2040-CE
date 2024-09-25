#ifndef _PS4AUTH_H_
#define _PS4AUTH_H_

#include "drivers/shared/gpauthdriver.h"
#include "mbedtls/rsa.h"

// We need to keep track of:
//     Xbox 360 Console Auth Init 34 bytes
//     Dongle Serial 29 bytes
//     Console-Dongle Back and Forth 46 bytes & 22 bytes
typedef struct {
    struct mbedtls_rsa_context rsa_context;
    uint8_t ps4_auth_buffer[1064];
    bool valid_rsa = false;
    bool dongle_ready = false;
    GPAuthState passthrough_state;    // PS4 Encryption Passthrough State
    uint8_t nonce_id;                 // for nonce passing
    //uint8_t nonce_buffer[256];      // for nonce passing
} PS4AuthData;

class PS4Auth : public GPAuthDriver {
public:
    PS4Auth(InputModeAuthType inType) { authType = inType; }
    virtual void initialize();
    virtual bool available();
    void process();
    PS4AuthData * getAuthData() { return &ps4AuthData; }
    void resetAuth();
private:
    void keyModeInitialize();
    void keyModeProcess();
    PS4AuthData ps4AuthData;
};

#endif
