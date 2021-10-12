/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

volatile static alarm_id_t flashWriteAlarm = 0;
static spin_lock_t *flashLock = nullptr;

int64_t writeToFlash(alarm_id_t id, void *flashCache)
{
	multicore_lockout_start_blocking();

	while (is_spin_locked(flashLock));
	uint32_t interrupts = spin_lock_blocking(flashLock);

	flash_range_erase((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, EEPROM_SIZE_BYTES);
	flash_range_program((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, reinterpret_cast<uint8_t *>(flashCache), EEPROM_SIZE_BYTES);

	spin_unlock(flashLock, interrupts);

	multicore_lockout_end_blocking();
	flashWriteAlarm = 0;

	return 0;
}

void FlashPROM::start()
{
	if (flashLock == nullptr)
		flashLock = spin_lock_instance(spin_lock_claim_unused(true));

	memcpy(cache, reinterpret_cast<uint8_t *>(EEPROM_ADDRESS_START), EEPROM_SIZE_BYTES);

	// When flash is new/reset, all bits are set to 1.
	// If all bits from the FlashPROM section are 1's then set to 0's.
	bool reset = true;
	for (int i = 0; i < EEPROM_SIZE_BYTES; i++)
	{
		if (cache[i] != 0xFF)
		{
			reset = false;
			break;
		}
	}

	if (reset)
	{
		memset(cache, 0, EEPROM_SIZE_BYTES);
		commit();
	}
}

/* We don't have an actual EEPROM, so we need to be extra careful about minimizing writes. Instead
	of writing when a commit is requested, we update a time to actually commit. That way, if we receive multiple requests
	to commit in that timeframe, we'll hold off until the user is done sending changes. */
void FlashPROM::commit()
{
	if (flashWriteAlarm && !is_spin_locked(flashLock))
	{
		cancel_alarm(flashWriteAlarm);
	}
	else
	{
		while (is_spin_locked(flashLock));
	}

	flashWriteAlarm = add_alarm_in_ms(EEPROM_WRITE_WAIT, writeToFlash, cache, true);
}
