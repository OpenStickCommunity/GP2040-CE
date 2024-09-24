/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef GP2040CORE1_H_
#define GP2040CORE1_H_

#include "addonmanager.h"
#include "drivermanager.h"

class GP2040Aux {
public:
	GP2040Aux();
    ~GP2040Aux();
    void setup();           // setup core1
    void run();             // loop core1
    bool ready(){ return isReady; }
private:
    GPDriver * inputDriver;
    AddonManager addons;
    bool isReady;
};

#endif
