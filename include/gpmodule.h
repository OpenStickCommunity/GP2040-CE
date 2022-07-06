#ifndef _GPModule_H_
#define _GPModule_H_

#include "gamepad.h"

class GPModule
{
public:
	virtual bool available() = 0;
	virtual void setup() = 0;
	virtual void loop() = 0;
	virtual void process(Gamepad *gamepad) = 0;
	virtual std::string name() = 0;
private:
};

#endif