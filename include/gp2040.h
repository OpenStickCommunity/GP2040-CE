/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

#include <stdint.h>
#include <string>
#include "pico/stdlib.h"
#include "gamepad.h"

using namespace std;

const string BUTTON_LABEL_UP = "Up";
const string BUTTON_LABEL_DOWN = "Down";
const string BUTTON_LABEL_LEFT = "Left";
const string BUTTON_LABEL_RIGHT = "Right";
const string BUTTON_LABEL_B1 = "B1";
const string BUTTON_LABEL_B2 = "B2";
const string BUTTON_LABEL_B3 = "B3";
const string BUTTON_LABEL_B4 = "B4";
const string BUTTON_LABEL_L1 = "L1";
const string BUTTON_LABEL_R1 = "R1";
const string BUTTON_LABEL_L2 = "L2";
const string BUTTON_LABEL_R2 = "R2";
const string BUTTON_LABEL_S1 = "S1";
const string BUTTON_LABEL_S2 = "S2";
const string BUTTON_LABEL_L3 = "L3";
const string BUTTON_LABEL_R3 = "R3";
const string BUTTON_LABEL_A1 = "A1";
const string BUTTON_LABEL_A2 = "A2";

class GPModule
{
public:
	virtual void setup() = 0;
	virtual void loop() = 0;
	virtual void process(Gamepad *gamepad) = 0;
	absolute_time_t nextRunTime;
	const uint32_t intervalMS = 10;
	inline bool isEnabled() { return enabled; }
protected:
	bool enabled = true;
};

#endif
