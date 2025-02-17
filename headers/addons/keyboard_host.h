#ifndef _KeyboardHost_H
#define _KeyboardHost_H

#include "gpaddon.h"

#ifndef KEYBOARD_HOST_ENABLED
#define KEYBOARD_HOST_ENABLED 0
#endif

#ifndef KEYBOARD_HOST_PIN_DPLUS
#define KEYBOARD_HOST_PIN_DPLUS -1
#endif

#ifndef KEYBOARD_HOST_PIN_5V
#define KEYBOARD_HOST_PIN_5V -1
#endif

// KeyboardHost Module Name
#define KeyboardHostName "KeyboardHost"

class KeyboardHostAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // KeyboardHost Setup
    virtual void process() {}   // KeyboardHost Process
    virtual void preprocess();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return KeyboardHostName; }
private:
};

#endif  // _KeyboardHost_H_