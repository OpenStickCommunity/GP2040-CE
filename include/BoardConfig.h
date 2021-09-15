/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include <vector>

#ifdef CONFIG_DEBUG_BOARD
#include "definitions/DebugBoard.h"
#elif CONFIG_OSFRD
#include "definitions/OSFRDConfig.h"
#else
#include "definitions/PicoConfig.h"
#endif

#ifdef CONFIG_USE_LEDS
#include "LEDConfig.h"
#endif

#endif
