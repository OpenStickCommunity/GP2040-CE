#ifndef _PS4AUTHKEYS_H_
#define _PS4AUTHKEYS_H_

#include "drivers/shared/gpauthdriver.h"
#include "mbedtls/rsa.h"

class PS4AuthKeys : public GPAuthDriver {
public:
    PS4AuthKeys() { authType = InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS; }
    virtual void initialize();
    virtual bool available();
    virtual void process();
private:
	struct mbedtls_rsa_context rsa_context;
	bool ready;
};

#endif
