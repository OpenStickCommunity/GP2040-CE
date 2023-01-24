#include "system.h"

#include <hardware/flash.h>
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
