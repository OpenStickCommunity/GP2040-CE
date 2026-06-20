/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _SWITCHPROHOST_H_
#define _SWITCHPROHOST_H_

#include <gphost.h>

typedef struct __attribute__((packed)) {
    uint8_t command;
    uint8_t counter;
    uint8_t rumble_l[4];
    uint8_t rumble_r[4];
    uint8_t subcommand;
    uint8_t subcommand_args[3];
} SwitchProHostReport;

#endif
