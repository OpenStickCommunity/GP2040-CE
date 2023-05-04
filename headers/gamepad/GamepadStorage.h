/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#include "config_legacy.h"

class GamepadStorage
{
	public:
		virtual void start(); // TODO: Should be pure virtual.
		virtual void save(); // TODO: Should be pure virtual.

		ConfigLegacy::GamepadOptions getGamepadOptions();
		void setGamepadOptions(ConfigLegacy::GamepadOptions options);
};

static GamepadStorage GamepadStore;
