/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

// Pi Pico includes
#include "pico/multicore.h"

// GP2040 includes
#include "gp2040.h"
#include "gp2040aux.h"

#include <cstdlib>

// Custom implementation of __gnu_cxx::__verbose_terminate_handler() to reduce binary size
namespace __gnu_cxx {
void __verbose_terminate_handler()
{
	abort();
}
}

static GP2040 * gp2040Core0 = nullptr;
static GP2040Aux * gp2040Core1 = nullptr;

// Launch our second core with additional modules loaded in
void core1() {
	multicore_lockout_victim_init(); // block core 1

	// Create GP2040 w/ Additional Modules for Core 1	
	gp2040Core1->setup();
	gp2040Core1->run();
}

int main() {
	// Create GP2040 Main Core (core0), Core1 is dependent on Core0
	gp2040Core0 = new GP2040();
	gp2040Core1 = new GP2040Aux();

	// Create GP2040 Main Core - Setup Core0
	gp2040Core0->setup();

	// Create GP2040 Thread for Core1
	multicore_launch_core1(core1);

	// Sync Core0 and Core1
	while(gp2040Core1->ready() == false ) {
		__asm volatile ("nop\n");
	}
	gp2040Core0->run();

	return 0;
}
