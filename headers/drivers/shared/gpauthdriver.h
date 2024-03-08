#ifndef _GPAUTHDRIVER_H_
#define _GPAUTHDRIVER_H_

#include "enums.pb.h"

#include "usblistener.h"

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
