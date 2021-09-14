/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include <vector>

#define DEBUG_BOARD           0
#define PICO_BOARD            1
#define OPEN_STICK_BOARD      2
#define PIMORONI_16MB_BOARD   3
#define TEST_BOARD          255

#if BOARD_CONFIG == DEBUG_BOARD
#include "definitions/DebugBoard.h"
#include "LEDConfig.h"
#elif BOARD_CONFIG == OPEN_STICK_BOARD
#include "definitions/OpenStickBoard.h"
#include "LEDConfig.h"
#elif BOARD_CONFIG == TEST_BOARD
#include "definitions/TestBoard.h"
#else
#include "definitions/RP2040Board.h"
#endif


#endif
