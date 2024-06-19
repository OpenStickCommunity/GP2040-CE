#include "system.h"

#include "usbhostmanager.h"

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <hardware/watchdog.h>
#include <pico/multicore.h>

#include <malloc.h>

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __bss_end__;
extern char __StackLimit;
extern char __StackTop;

uint32_t System::getTotalFlash() {
#if defined(PICO_FLASH_SIZE_BYTES)
    return PICO_FLASH_SIZE_BYTES;
#else
    #warning PICO_FLASH_SIZE_BYTES is not set, defaulting to 2MB
    return 2 * 1024 * 1024;
#endif
}

uint32_t System::getUsedFlash() {
    return &__flash_binary_end - &__flash_binary_start;
}

#define STORAGE_CMD_TOTAL_BYTES 8

uint32_t System::getPhysicalFlash() {
    uint8_t txbuf[STORAGE_CMD_TOTAL_BYTES] = {0x9f};
    uint8_t rxbuf[STORAGE_CMD_TOTAL_BYTES] = {0};
    flash_do_cmd(txbuf, rxbuf, STORAGE_CMD_TOTAL_BYTES);
    return 1 << rxbuf[3];
}

uint32_t System::getStaticAllocs() {
    const uint32_t inMemorySegmentsSize = reinterpret_cast<uint32_t>(&__bss_end__) - SRAM_BASE;
    const uint32_t stackSize = &__StackTop - &__StackLimit;
    return inMemorySegmentsSize + stackSize;
}

uint32_t System::getTotalHeap() {
    return &__StackLimit  - &__bss_end__;
}

uint32_t System::getUsedHeap() {
    return mallinfo().uordblks;
}

void System::reboot(BootMode bootMode) {
    // Halt all running USB instances
    USBHostManager::getInstance().shutdown();

    // Make sure that the other core is halted
    // We do not want it to be talking to devices (e.g. OLED display) while we reboot
	multicore_lockout_start_timeout_us(0xfffffffffffffff);

	watchdog_hw->scratch[5] = static_cast<uint32_t>(bootMode);

    // This is based on MicroPythons machine.reset()
	watchdog_reboot(0, 0, 0);
	for (;;) {
		__wfi();
	}
}

System::BootMode System::takeBootMode() {
    // If the boot was not caused by software we don't enter any of the special modes
    if (!watchdog_caused_reboot()) {
        return BootMode::DEFAULT;
    }

    BootMode bootMode = static_cast<BootMode>(watchdog_hw->scratch[5]);
    if (bootMode != BootMode::GAMEPAD && bootMode != BootMode::WEBCONFIG && bootMode != BootMode::USB) {
        bootMode = BootMode::DEFAULT;
    }

    // Reset the scratch register
    // Subsequent reboots should revert to BootMode::DEFAULT
    watchdog_hw->scratch[5] = static_cast<uint32_t>(BootMode::DEFAULT);

    return bootMode;
}
