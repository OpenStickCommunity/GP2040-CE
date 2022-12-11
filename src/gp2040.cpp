// GP2040 includes
#include "gp2040.h"
#include "helper.h"

#include "configmanager.h" // Global Managers
#include "storagemanager.h"
#include "addonmanager.h"

#include "addons/analog.h" // Inputs for Core0
#include "addons/dualdirectional.h"
#include "addons/i2canalog1219.h"
#include "addons/jslider.h"
#include "addons/reverse.h"
#include "addons/turbo.h"

// Pico includes
#include "pico/bootrom.h"

// TinyUSB
#include "usb_driver.h"
#include "tusb.h"

#define GAMEPAD_DEBOUNCE_MILLIS 5 // make this a class object

GP2040::GP2040() : nextRuntime(0) {
	Storage::getInstance().SetGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
	Storage::getInstance().SetProcessedGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
}

GP2040::~GP2040() {
}

void GP2040::setup() {
    // Setup Gamepad and Gamepad Storage
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->setup();

	// Check for Config or Regular Input (w/ Button Combos)
	InputMode inputMode = gamepad->options.inputMode;
	gamepad->read();
	if (gamepad->pressedF1() && gamepad->pressedUp()) { // BOOTSEL - Go to UF2 Flasher
		reset_usb_boot(0, 0);
	} else if (gamepad->pressedS2()) { 					// START - Config Mode
		Storage::getInstance().SetConfigMode(true);
		inputMode = INPUT_MODE_CONFIG; // force config
        initialize_driver(inputMode);
		ConfigManager::getInstance().setup(CONFIG_TYPE_WEB);
	} else { 											// Gamepad Mode
		Storage::getInstance().SetConfigMode(false);
		if (gamepad->pressedB3())                       // HOLD B3 - D-INPUT
			inputMode = INPUT_MODE_HID;
		else if (gamepad->pressedB1())                  // HOLD B1 - SWITCH
			inputMode = INPUT_MODE_SWITCH;
		else if (gamepad->pressedB2())                  // HOLD B2 - X-INPUT
			inputMode = INPUT_MODE_XINPUT;
		if (inputMode != gamepad->options.inputMode ) { // Save changes
			gamepad->options.inputMode = inputMode;
			gamepad->save();
		}
		initialize_driver(inputMode);
	}

	// Setup Add-ons
	addons.LoadAddon(new AnalogInput(), CORE0_INPUT);
	addons.LoadAddon(new DualDirectionalInput(), CORE0_INPUT);
	addons.LoadAddon(new I2CAnalog1219Input(), CORE0_INPUT);
	addons.LoadAddon(new JSliderInput(), CORE0_INPUT);
	addons.LoadAddon(new ReverseInput(), CORE0_INPUT);
	addons.LoadAddon(new TurboInput(), CORE0_INPUT);
}

void GP2040::run() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
	bool configMode = Storage::getInstance().GetConfigMode();
	while (1) { // LOOP
		// Config Loop (Web-Config does not require gamepad)
		if (configMode == true ) {
			ConfigManager::getInstance().loop();
			continue;
		}

		if (nextRuntime > getMicro()) { // fix for unsigned
			sleep_us(50); // Give some time back to our CPU (lower power consumption)
			continue;
		}

		// Gamepad Features
		gamepad->read(); 	// gpio pin reads
	#if GAMEPAD_DEBOUNCE_MILLIS > 0
		gamepad->debounce();
	#endif
		gamepad->hotkey(); 	// check for MPGS hotkeys
		gamepad->process(); // process through MPGS

		addons.ProcessAddons(ADDON_PROCESS::CORE0_INPUT);

		// Copy Processed Gamepad for Core1 (race condition otherwise)
		memcpy(&processedGamepad->state, &gamepad->state, sizeof(GamepadState));

		// USB FEATURES : Send/Get USB Features (including Player LEDs on X-Input)
		send_report(gamepad->getReport(), gamepad->getReportSize());
		Storage::getInstance().ClearFeatureData();
		receive_report(Storage::getInstance().GetFeatureData());
		tud_task(); // TinyUSB Task update

		nextRuntime = getMicro() + GAMEPAD_POLL_MICRO;
	}
}
