#ifndef _GPAUTHDRIVER_H_
#define _GPAUTHDRIVER_H_

#include "enums.pb.h"

class GPAuthDriver {
public:
    virtual void initialize() = 0;
    virtual bool available() = 0;
    virtual void process() = 0;
    InputModeAuthType getAuthType() { return authType; }
protected:
    InputModeAuthType authType;
};

#endif
