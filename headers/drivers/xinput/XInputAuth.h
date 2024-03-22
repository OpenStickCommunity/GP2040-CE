#ifndef _XINPUTAUTH_H_
#define _XINPUTAUTH_H_

#include "drivers/shared/gpauthdriver.h"

typedef enum {
    auth_idle_state = 0,
    send_auth_console_to_dongle = 1,
    send_auth_dongle_to_console = 2,
    wait_auth_console_to_dongle = 3,
    wait_auth_dongle_to_console = 4,
} XInputAuthState;

typedef struct {
    XInputAuthState xboneState;

    // Console-to-Host e.g. Xbox 360 to MagicBoots
    bool authCompleted;
} XInputAuthData;

class XInputAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
private:
};

#endif
