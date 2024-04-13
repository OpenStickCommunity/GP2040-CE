#include "gpio_interface.h"
#include "pico/stdlib.h"
#include "storagemanager.h"

#include "config.pb.h"

uint32_t xor_mask = 0b0;

void gpio_interface_setup() {
    GpioMappingInfo* gpioMappings = Storage::getInstance().getGpioMappings().pins;
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        // If invert is true for the pin, set the corresponding bit in the bitmask
        if (gpioMappings[pin].has_invert && gpioMappings[pin].invert) {
            xor_mask |= (1u << pin);
        }
    }
}

uint32_t gpio_interface_get_all() {
    return gpio_get_all() ^ xor_mask;
}
