#include "BTInterface.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "btstack_run_loop.h"
#include "FlashPROM.h"

// we need to adjust the flash bank of btstack to not overlap with our config
// since we took the EEPROM's method for placing storage... the same thing BTStack did
uint32_t custom_btstack_storage_offset() {
  constexpr const uint32_t width = FLASH_SECTOR_SIZE * 2u;
  constexpr const uint32_t offset = (PICO_FLASH_SIZE_BYTES - width) - EEPROM_SIZE_BYTES;
    static_assert(offset + width <= PICO_FLASH_SIZE_BYTES, "PICO_FLASH_BANK_TOTAL_SIZE too big");
#ifndef NDEBUG
    // Check we're not overlapping the binary in flash
    extern char __flash_binary_end;
    assert(((uintptr_t)&__flash_binary_end - XIP_BASE <= offset));
#endif
    return offset;
}



int btstack_main(int argc, const char * argv[]);

void setupBTInterface() {
  setup_default_uart();
  if (cyw43_arch_init()) {
    printf("cyw43_arch_init failed.\n");
    return;
  }
  printf("cyw43_arch_init success.\n");

  // run the app
  btstack_main(0, NULL);
  printf("btstack_main called.\n");
  btstack_run_loop_execute();
  printf("btstack_run_loop_execute called.\n");
}