#ifndef _PsxAddon_H
#define _PsxAddon_H

#include <string>
#include <stdint.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"

#define PsxAddonName "Psx"

#ifndef PSX_ENABLED
    #define PSX_ENABLED 0
#endif

#ifndef PSX_DATA_PIN
    #define PSX_DATA_PIN -1
#endif

#ifndef PSX_ATTENTION_PIN
    #define PSX_ATTENTION_PIN -1
#endif

#ifndef PSX_COMMAND_PIN
    #define PSX_COMMAND_PIN -1
#endif

#ifndef PSX_CLOCK_PIN
    #define PSX_CLOCK_PIN -1
#endif

#ifndef PSX_ACKNOWLEDGE_PIN
    #define PSX_ACKNOWLEDGE_PIN -1
#endif

class PsxAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void process();
	virtual void preprocess() {}
	virtual std::string name() { return PsxAddonName; }
private:
};

#endif  // _PsxAddon_H