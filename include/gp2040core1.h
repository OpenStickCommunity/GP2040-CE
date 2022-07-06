/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040CORE1_H_
#define GP2040CORE1_H_

#include <vector>

#include "modules/i2cdisplay.h"
#include "modules/neopicoleds.h"
#include "modules/pleds.h"

class GP2040Core1 {
public:
	GP2040Core1();
    ~GP2040Core1();
    void setup();
    void run();
private:
    void loop();
    void configLoop();
    void setupModule(GPModule*);
};

#endif
