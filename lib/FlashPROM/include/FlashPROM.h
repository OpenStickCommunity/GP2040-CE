/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef FLASHPROM_H_
#define FLASHPROM_H_

#include <stdint.h>
#include <string.h>
#include <hardware/flash.h>
#include <hardware/sync.h>

#define EEPROM_SIZE_BYTES    4096       // Reserve 4k of flash memory (ensure this value is divisible by 256)
#define EEPROM_ADDRESS_START 0x101FF000 // The arduino-pico EEPROM lib starts here, so we'll do the same

class FlashPROM
{
	public:
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
			uint8_t size = sizeof(T);
			if ((index + size) <= EEPROM_SIZE_BYTES)
				memcpy(&cache[index], &value, sizeof(T));
		}

	private:
		static uint8_t cache[EEPROM_SIZE_BYTES];
};

static FlashPROM EEPROM;

#endif
