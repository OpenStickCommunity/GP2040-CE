/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

static alarm_id_t flashWriteAlarm = 0;
static spin_lock_t *flashLock = nullptr;

int64_t writeToFlash(alarm_id_t id, void *cache)
{
	multicore_lockout_start_blocking();

	while (is_spin_locked(flashLock));
	uint32_t interrupts = spin_lock_blocking(flashLock);

	flash_range_erase((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, EEPROM_SIZE_BYTES);
	flash_range_program((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, (uint8_t *)cache, EEPROM_SIZE_BYTES);

	spin_unlock(flashLock, interrupts);
	flashWriteAlarm = 0;

	multicore_lockout_end_blocking();
	return 0;
}

void FlashPROM::start()
{
	if (flashLock == nullptr)
		flashLock = spin_lock_instance(spin_lock_claim_unused(true));

	memcpy(cache, reinterpret_cast<uint8_t *>(EEPROM_ADDRESS_START), EEPROM_SIZE_BYTES);
}

/* We don't have an actual EEPROM, so we need to be extra careful about minimizing writes. Instead
	of writing when a commit is requested, we update a time to actually commit. That way, if we receive multiple requests
	to commit in that timeframe, we'll hold off until the user is done sending changes. */
void FlashPROM::commit()
{
	if (flashWriteAlarm)
		cancel_alarm(flashWriteAlarm);

	flashWriteAlarm = add_alarm_in_ms(EEPROM_WRITE_WAIT, writeToFlash, cache, true);
}
