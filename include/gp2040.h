/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

// GP2040 Classes
#include "gamepad.h"
#include "gpaddon.h"

class GP2040 {
public:
	GP2040();
    ~GP2040();
    void setup();           // setup core0
    void run();             // loop core0
private:
    void setupInput(GPAddon*);
    uint64_t nextRuntime;
    Gamepad snapshot;
};

#endif
