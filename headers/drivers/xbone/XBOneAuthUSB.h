#ifndef _XBONEAUTHUSB_H_
#define _XBONEAUTHUSB_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/xgip_protocol.h"

class XBOneAuthUSB : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
private:
};

#endif
