/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _GPCONFIG_H_
#define _GPCONFIG_H_

class GPConfig {
public:
    virtual void setup() = 0;
    virtual void loop() = 0;
private:
};

#endif