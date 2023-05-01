/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef FLASHPROM_H_
#define FLASHPROM_H_

#include <stdint.h>
#include <string.h>
#include <pico/lock_core.h>
#include <pico/multicore.h>
#include <hardware/flash.h>
#include <hardware/timer.h>

#define EEPROM_SIZE_BYTES    0x2000           // Reserve 8k of flash memory (ensure this value is divisible by 256)
#define EEPROM_ADDRESS_START _u(0x101FE000) // The arduino-pico EEPROM lib starts here, so we'll do the same
// Warning: If the write wait is too long it can stall other processes
#define EEPROM_WRITE_WAIT    50             // Amount of time in ms to wait before blocking core1 and committing to flash

class FlashPROM
{
	public:
		void start();
		void commit();
		void reset();

		template<typename T>
		T &get(uint16_t const index, T &value)
		{
			if (index < EEPROM_SIZE_BYTES)
				memcpy(&value, &cache[index], sizeof(T));

			return value;
		}

		template<typename T>
		void set(uint16_t const index, const T &value)
		{
			uint16_t size = sizeof(T);

			if ((index + size) <= EEPROM_SIZE_BYTES)
				memcpy(&cache[index], &value, sizeof(T));
		}

	// private:
		static uint8_t cache[EEPROM_SIZE_BYTES];
};

static FlashPROM EEPROM;

#endif
