#ifndef PS4MODE_H_
#define PS4MODE_H_

#include "gpaddon.h"
#include "storagemanager.h"

#include <ds4crypto.h>

#ifndef PS4MODE_ADDON_ENABLED
#define PS4MODE_ADDON_ENABLED 0
#endif

// Turbo Module Name
#define PS4ModeName "PS4Mode"

class PS4ModeAddon : public GPAddon {
public:
	PS4ModeAddon();
	virtual bool available();
	virtual void setup();       // TURBO Button Setup
	virtual void preprocess() {}
	virtual void process();     // TURBO Setting of buttons (Enable/Disable)
	virtual std::string name() { return PS4ModeName; }
private:
	LoadedDS4Key ds4Key;
};

#endif  // PS4MODE_H_
