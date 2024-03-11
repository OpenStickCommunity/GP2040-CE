#ifndef _XINPUTAUTH_H_
#define _XINPUTAUTH_H_

#include "drivers/shared/gpauthdriver.h"

class XInputAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
private:
};

#endif
