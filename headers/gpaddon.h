#ifndef _GPAddon_H_
#define _GPAddon_H_

#include "gamepad.h"
#include "usblistener.h"

#include <string>

class GPAddon
{
public:
    virtual ~GPAddon() { }
    virtual bool available() = 0;
    virtual void setup() = 0;
    virtual void process() = 0;
    virtual void preprocess() = 0;
    virtual void postprocess(bool) = 0;
    virtual std::string name() = 0;

    /**
     * Reinitialize the addon --- only implement this if it makes sense to, e.g. if this
     * addon allows its pin assignments to be changed, in which case it needs to rebuild
     * its pin masks, as is needed for DDI and sliders.
     */
    virtual void reinit() = 0;

    // For add-ons that require a USB-host listener, get listener
    virtual USBListener * getListener() { return listener; }

protected:
    USBListener * listener;
};

#endif
