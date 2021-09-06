/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#define DEBUG_BOARD        0
#define PICO_BOARD         1
#define OPEN_STICK_BOARD   2
#define TEST_BOARD       255

#if BOARD_DEFINITION == DEBUG_BOARD
#include "definitions/DebugBoard.h"
#elif BOARD_DEFINITION == PICO_BOARD
#include "definitions/RP2040Board.h"
#elif BOARD_DEFINITION == OPEN_STICK_BOARD
#include "definitions/OpenStickBoard.h"
#elif BOARD_DEFINITION == TEST_BOARD
#include "definitions/TestBoard.h"
#endif

#include "LEDConfig.h"
#endif
