#ifndef _GPAUTHDRIVER_H_
#define _GPAUTHDRIVER_H_

#include "enums.pb.h"
#include "usblistener.h"

typedef enum {
    auth_idle_state = 0,
    send_auth_console_to_dongle = 1,
    send_auth_dongle_to_console = 2,
    wait_auth_console_to_dongle = 3,
    wait_auth_dongle_to_console = 4,
} GPAuthState;

class GPAuthDriver {
public:
    virtual void initialize() = 0;
    virtual bool available() = 0;
    virtual USBListener * getListener() { return listener; }
    InputModeAuthType getAuthType() { return authType; }
protected:
    USBListener * listener;
    InputModeAuthType authType;
};

#endif
