#ifndef _PS4AUTH_H_
#define _PS4AUTH_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/ps4/PS4Driver.h"
#include "usblistener.h"
#include "mbedtls/rsa.h"

class PS4Auth : public GPAuthDriver {
public:
    PS4Auth(InputModeAuthType inType) { authType = inType; }
    virtual void initialize();
    virtual bool available();
    void process(PS4State, uint8_t, uint8_t*);
    uint8_t * getAuthBuffer() { return ps4_auth_buffer; }
    bool getAuthReady();
    void resetAuth();
private:
    struct mbedtls_rsa_context rsa_context;
    bool valid_rsa;

    // buffer = 256 + 16 + 256 + 256 + 256 + 24
    // == 1064 bytes (almost 1 kb)
    uint8_t ps4_auth_buffer[1064];
    bool ps4_keys_signature_ready;
};

#endif
