/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "GamepadEnums.h"

struct GamepadOptions
{
	InputMode inputMode {InputMode::INPUT_MODE_XINPUT}; 
	DpadMode dpadMode {DpadMode::DPAD_MODE_DIGITAL};
	SOCDMode socdMode {SOCDMode::SOCD_MODE_NEUTRAL};
	bool invertXAxis;
	bool invertYAxis;
	uint32_t checksum;
};
