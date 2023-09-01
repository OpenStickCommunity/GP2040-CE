#ifndef _GPAddon_H_
#define _GPAddon_H_

#include "gamepad.h"

#include <string>

class GPAddon
{
public:
	virtual ~GPAddon() { }
	virtual bool available() = 0;
	virtual void setup() = 0;
	virtual void process() = 0;
	virtual void preprocess() = 0;
	virtual std::string name() = 0;
};

#endif