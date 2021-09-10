/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

uint8_t FlashPROM::cache[EEPROM_SIZE_BYTES] = { };
spin_lock_t *FlashPROM::flashLock;
absolute_time_t FlashPROM::nextWriteTime = 0;
alarm_id_t flashWriteAlarm = 0;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
	// Can return a value here in us to fire in the future
	return 0;
}

FlashPROM::FlashPROM()
{
	flashLock = spin_lock_instance(spin_lock_claim_unused(true));
}

void FlashPROM::start()
{
	memcpy(cache, (uint8_t *)EEPROM_ADDRESS_START, EEPROM_SIZE_BYTES);
}

/* We don't have an actual EEPROM, so we need to be extra careful about minimizing writes. Instead
	of writing when a commit is requested, we update a time to actually commit. That way, if we receive multiple requests
	to commit in that timeframe, we'll hold off until the user is done sending changes. */
void FlashPROM::commit()
{
	if (flashWriteAlarm)
		cancel_alarm(flashWriteAlarm);

	flashWriteAlarm = add_alarm_in_ms(EEPROM_WRITE_WAIT, FlashPROM::writeToFlash, NULL, true);
}

int64_t FlashPROM::writeToFlash(alarm_id_t id, void *user_data)
{
	multicore_lockout_start_blocking();

	while (is_spin_locked(flashLock));
	uint32_t interrupts = spin_lock_blocking(flashLock);

	flash_range_erase((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, EEPROM_SIZE_BYTES);
	flash_range_program((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, cache, EEPROM_SIZE_BYTES);

	spin_unlock(flashLock, interrupts);
	flashWriteAlarm = 0;

	multicore_lockout_end_blocking();
	return 0;
}
