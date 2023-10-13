/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef _PLEDS_H_
#define _PLEDS_H_

#include "BoardConfig.h"
#include <stdint.h>
#include "AnimationStation.hpp"
#include "storagemanager.h"
#include "PlayerLEDs.h"
#include "gpaddon.h"
#include "helper.h"

#include "enums.pb.h"

// This needs to be moved to storage if we're going to share between modules
extern NeoPico *neopico;
extern AnimationStation as;

class PWMPlayerLEDs : public PlayerLEDs
{
public:
	void setup();
	void display();
};

// Player LED Module
#define PLEDName "PLED"

// Player LED Module
class PlayerLEDAddon : public GPAddon
{
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return PLEDName; }
	PlayerLEDAddon() {
		type = static_cast<PLEDType>(Storage::getInstance().getLedOptions().pledType);
	}
	PlayerLEDAddon(PLEDType type) : type(type) {}

protected:
	PLEDType type;
	PWMPlayerLEDs *pwmLEDs = nullptr;
	PLEDAnimationState animationState;

private:
	bool turnOffWhenSuspended;
};

#endif
