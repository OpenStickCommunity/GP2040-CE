/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PLEDS_H_
#define PLEDS_H_

#include "BoardConfig.h"
#include <stdint.h>
#include "AnimationStation.hpp"
#include "PlayerLEDs.h"
#include "gp2040.h"

#define PLED_REPORT_SIZE 32

#ifndef PLED1_PIN
#define PLED1_PIN -1
#endif
#ifndef PLED2_PIN
#define PLED2_PIN -1
#endif
#ifndef PLED3_PIN
#define PLED3_PIN -1
#endif
#ifndef PLED4_PIN
#define PLED4_PIN -1
#endif
#ifndef PLED_TYPE
#define PLED_TYPE PLED_TYPE_NONE
#endif

#define PLED_MASK_ALL ((1U << PLED1_PIN) | (1U << PLED2_PIN) | (1U << PLED3_PIN) | (1U << PLED4_PIN))

extern NeoPico *neopico;
extern AnimationStation as;

class PWMPlayerLEDs : public PlayerLEDs
{
public:
	void setup();
	void display();
};

class RGBPlayerLEDs : public PlayerLEDs
{
public:
	void setup();
	void display();
};

class PLEDModule : public GPModule
{
public:
	PLEDModule(PLEDType type) : type(type) { }

	void setup();
	void loop();
	void process(Gamepad *gamepad);
	queue_t featureQueue;
protected:
	PLEDType type;
	PlayerLEDs *pleds = nullptr;
	PLEDAnimationState animationState;
};

extern PLEDModule pledModule;

#endif
