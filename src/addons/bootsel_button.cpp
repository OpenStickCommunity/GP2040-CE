#include "addons/bootsel_button.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "helper.h"
#include "config.pb.h"

bool __no_inline_not_in_flash_func(BootselButtonAddon::isBootselPressed)() {
	const uint CS_PIN_INDEX = 1;

	// Must disable interrupts, as interrupt handlers may be in flash, and we
	// are about to temporarily disable flash access!
	uint32_t flags = save_and_disable_interrupts();

	// Set chip select to Hi-Z
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
					GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
					IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	// Note we can't call into any sleep functions in flash right now
	for (volatile int i = 0; i < 1000; ++i);

	// The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
	// Note the button pulls the pin *low* when pressed.
	bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

	// Need to restore the state of chip select, else we are going to have a
	// bad time when we return to code in flash!
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
					GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
					IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	restore_interrupts(flags);

	return button_state;
}

bool BootselButtonAddon::available() {
    const BootselButtonOptions& options = Storage::getInstance().getAddonOptions().bootselButtonOptions;
	return options.enabled && options.buttonMap != 0;
}

void BootselButtonAddon::setup() {
    const BootselButtonOptions& options = Storage::getInstance().getAddonOptions().bootselButtonOptions;
	bootselButtonMap = options.buttonMap;
}

void BootselButtonAddon::preprocess() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	if (isBootselPressed()) {
		if (bootselButtonMap > (GAMEPAD_MASK_A2)) {
			switch (bootselButtonMap) {
				case (1U << 14):
					gamepad->state.dpad |= GAMEPAD_MASK_UP;
					break;
				case (1U << 15):
					gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
					break;
				case (1U << 16):
					gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
					break;
				case (1U << 17):
					gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
					break;
			}
		}
		else gamepad->state.buttons |= bootselButtonMap;
	}
}