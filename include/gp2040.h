/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GP2040_H_
#define GP2040_H_

// GP2040 Classes
#include "gamepad.h"
#include "config.h"

class GP2040 {
public:
	GP2040();
    ~GP2040();
    void setup();
    void run();
private:
    void loop();
	void configLoop(); // web-config
    uint64_t nextRuntime;
    uint8_t featureData[32];
    Gamepad snapshot;
};

#endif
