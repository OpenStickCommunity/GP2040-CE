/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040CORE1_H_
#define GP2040CORE1_H_

#include <vector>

#include "addons/i2cdisplay.h"
#include "addons/neopicoleds.h"
#include "addons/playerleds.h"

class GP2040Aux {
public:
	GP2040Aux();
    ~GP2040Aux();
    void setup();           // setup core1
    void run();             // loop core1
private:
    void setupAddon(GPAddon*);
};

#endif
