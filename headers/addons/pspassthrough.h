#ifndef _PSPassthrough_H
#define _PSPassthrough_H

#include "gpaddon.h"

#include "ps4_driver.h"

#ifndef PSPASSTHROUGH_ENABLED
#define PSPASSTHROUGH_ENABLED 1
#endif

#ifndef PPSPASSTHROUGH_PIN_DPLUS
#define PSPASSTHROUGH_PIN_DPLUS 16
#endif

#ifndef PSPASSTHROUGH_PIN_5V
#define PSPASSTHROUGH_PIN_5V 18
#endif

#ifndef PSPASSTHROUGH_DEBUG
#define PSPASSTHROUGH_DEBUG true
#endif

// KeyboardHost Module Name
#define PSPassthroughName "PSPassthrough"

class PSPassthroughAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // PSPassthrough Setup
	virtual void process();     // PSPassthrough Process
	virtual void preprocess() {}
	virtual std::string name() { return PSPassthroughName; }
private:
};

#endif  // _PSPassthrough_H_