#include "gpio_interface.h"
#include "pico/stdlib.h"

uint32_t gpio_interface_get_all() {
    uint32_t xor_mask = 0b00000000000111111111111000000000;
    return gpio_get_all() ^ xor_mask;
}
