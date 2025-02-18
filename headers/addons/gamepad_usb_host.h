#ifndef _GamepadUSBHost_H
#define _GamepadUSBHost_H

#include "gpaddon.h"
#include "usblistener.h"
#include "gamepad.h"
#include "class/hid/hid.h"
#include "storagemanager.h"

#ifndef GAMEPAD_USB_HOST_ENABLED
#define GAMEPAD_USB_HOST_ENABLED 0
#endif

// GamepadUSBHost Module Name
#define GamepadUSBHostName "GamepadUSBHost"

class GamepadUSBHostAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // GamepadUSBHost Setup
    virtual void process() {}   // GamepadUSBHost Process
    virtual void preprocess();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return GamepadUSBHostName; }
private:
};


#endif  // GamepadUSBHost_H_