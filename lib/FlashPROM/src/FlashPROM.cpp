/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

uint8_t FlashPROM::cache[EEPROM_SIZE_BYTES] = { };

void FlashPROM::start()
{
	memcpy(&cache, (uint8_t *)EEPROM_ADDRESS_START, EEPROM_SIZE_BYTES);
}

void FlashPROM::commit()
{
	uint32_t int_status = save_and_disable_interrupts();
	flash_range_erase((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, EEPROM_SIZE_BYTES);
	flash_range_program((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, cache, EEPROM_SIZE_BYTES);
	restore_interrupts(int_status);
}
