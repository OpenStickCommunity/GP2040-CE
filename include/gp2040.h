/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

#include <stdint.h>
#include "pico/stdlib.h"
#include "gamepad.h"

class GPModule
{
public:
	virtual void setup();
	virtual void loop();
	virtual void process(Gamepad *gamepad);
	absolute_time_t nextRunTime;
	const uint32_t intervalMS = 10;
};

#endif
