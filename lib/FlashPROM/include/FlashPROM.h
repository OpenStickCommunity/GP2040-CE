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

#define EEPROM_SIZE_BYTES    4096       // Reserve 4k of flash memory (ensure this value is divisible by 256)
#define EEPROM_ADDRESS_START 0x101FF000 // The arduino-pico EEPROM lib starts here, so we'll do the same
#define EEPROM_WRITE_WAIT    100        // Amount of time in ms to wait before blocking core1 and committing to flash

class FlashPROM
{
	public:
		FlashPROM();

		void start();
		void commit();

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

		static absolute_time_t nextWriteTime;

	private:
		static spin_lock_t *flashLock;
		static uint8_t cache[EEPROM_SIZE_BYTES];

		static int64_t writeToFlash(alarm_id_t id, void *user_data);

};

static FlashPROM EEPROM;

#endif
