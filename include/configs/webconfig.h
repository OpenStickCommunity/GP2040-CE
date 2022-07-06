#ifndef _WEBCONFIG_H_
#define _WEBCONFIG_H_

#include "gpconfig.h"

class WebConfig : public GPConfig
{
public:
    virtual void setup();
    virtual void loop();
private:
};

#endif