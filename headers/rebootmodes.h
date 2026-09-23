/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _REBOOTMODES_H_
#define _REBOOTMODES_H_

#include <stdint.h>

#include "system.h"

// Reboot modes of the web configurator's /api/reboot (MUST MATCH
// NAVIGATION.JSX). Host Lighting's REBOOT targets are the same values.
enum BOOT_MODES {
	GAMEPAD = 0,
	WEBCONFIG = 1,
	BOOTSEL = 2,
};

// The System boot mode a reboot mode selects; false for an unknown mode
inline bool rebootModeToBootMode(uint32_t rebootMode, System::BootMode & bootMode) {
	switch (rebootMode) {
		case BOOT_MODES::GAMEPAD:
			bootMode = System::BootMode::GAMEPAD;
			return true;
		case BOOT_MODES::WEBCONFIG:
			bootMode = System::BootMode::WEBCONFIG;
			return true;
		case BOOT_MODES::BOOTSEL:
			bootMode = System::BootMode::USB;
			return true;
		default:
			return false;
	}
}

#endif
