/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "pico.h"

// A RAM fast path for function-local static guards. In the ARM EABI, bit 0 of
// the guard is set once the object is constructed. This returns 0 when it is
// set, with the acquire ordering the SDK function uses, and calls the SDK
// function otherwise.
extern "C" {
int __real___cxa_guard_acquire(int* guard);

int __not_in_flash_func(__wrap___cxa_guard_acquire)(int* guard) {
	if (__atomic_load_n(guard, __ATOMIC_ACQUIRE) & 1)
		return 0;
	return __real___cxa_guard_acquire(guard);
}
}
