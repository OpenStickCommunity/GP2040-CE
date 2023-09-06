#include "BTInterface.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "btstack_run_loop.h"

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