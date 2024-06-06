#ifndef _ControllerHost_H
#define _ControllerHost_H

#include "gpaddon.h"
#include "usblistener.h"
#include "gamepad.h"
#include "class/hid/hid.h"
#include "storagemanager.h"

#ifndef CONTROLLER_HOST_ENABLED
#define CONTROLLER_HOST_ENABLED 0
#endif

// ControllerHost Module Name
#define ControllerHostName "ControllerHost"

class ControllerHostAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // ControllerHost Setup
	virtual void process() {}   // ControllerHost Process
	virtual void preprocess();
	virtual std::string name() { return ControllerHostName; }
private:
};


#endif  // _ControllerHost_H_