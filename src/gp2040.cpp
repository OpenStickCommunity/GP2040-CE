// GP2040 includes
#include "gp2040.h"
#include "helper.h"
#include "system.h"

#include "configmanager.h" // Global Managers
#include "storagemanager.h"
#include "addonmanager.h"

#include "addons/analog.h" // Inputs for Core0
#include "addons/dualdirectional.h"
#include "addons/i2canalog1219.h"
#include "addons/jslider.h"
#include "addons/reverse.h"
#include "addons/turbo.h"
#include "addons/bootsel_button.h"

// Pico includes
#include "pico/bootrom.h"
#include "pico/time.h"

// TinyUSB
#include "usb_driver.h"
#include "tusb.h"

#define GAMEPAD_DEBOUNCE_MILLIS 5 // make this a class object

static const uint32_t WEBCONFIG_HOTKEY_HOLD_TIME_MS = 4000;

GP2040::GP2040() : nextRuntime(0), webConfigHotkeyHoldTimeout(nil_time) {
	Storage::getInstance().SetGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
	Storage::getInstance().SetProcessedGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
}

GP2040::~GP2040() {
}

void GP2040::setup() {
    // Setup Gamepad and Gamepad Storage
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->setup();

	const BootAction bootAction = getBootAction();
	switch (bootAction) {
		case BootAction::ENTER_WEBCONFIG_MODE:
			{
				Storage::getInstance().SetConfigMode(true);
				initialize_driver(INPUT_MODE_CONFIG);
				ConfigManager::getInstance().setup(CONFIG_TYPE_WEB);
				break;	
			}

		case BootAction::ENTER_USB_MODE:
			{
				reset_usb_boot(0, 0);
				break;	
			}

		case BootAction::SET_INPUT_MODE_HID:
		case BootAction::SET_INPUT_MODE_SWITCH:
		case BootAction::SET_INPUT_MODE_XINPUT:
		case BootAction::NONE:
			{
				InputMode inputMode = gamepad->options.inputMode;
				if (bootAction == BootAction::SET_INPUT_MODE_HID) {
					inputMode = INPUT_MODE_HID;
				} else if (bootAction == BootAction::SET_INPUT_MODE_SWITCH) {
					inputMode = INPUT_MODE_SWITCH;
				} else if (bootAction == BootAction::SET_INPUT_MODE_XINPUT) {
					inputMode = INPUT_MODE_XINPUT;
				}

				if (inputMode != gamepad->options.inputMode) {
					// Save the changed input mode
					gamepad->options.inputMode = inputMode;
					gamepad->save();
				}

				initialize_driver(inputMode);
				break;
			}
	}

	// Setup Add-ons
	addons.LoadAddon(new AnalogInput(), CORE0_INPUT);
	addons.LoadAddon(new DualDirectionalInput(), CORE0_INPUT);
	addons.LoadAddon(new I2CAnalog1219Input(), CORE0_INPUT);
	addons.LoadAddon(new JSliderInput(), CORE0_INPUT);
	addons.LoadAddon(new ReverseInput(), CORE0_INPUT);
	addons.LoadAddon(new TurboInput(), CORE0_INPUT);
	addons.LoadAddon(new BootselButtonAddon(), CORE0_INPUT);

	webConfigHotkeyMask = GAMEPAD_MASK_S2;
}

void GP2040::run() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
	bool configMode = Storage::getInstance().GetConfigMode();
	while (1) { // LOOP
		// Config Loop (Web-Config does not require gamepad)
		if (configMode == true ) {
			ConfigManager& configManager = ConfigManager::getInstance();
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

		// Restart in webconfig mode if the hotkey is held for a period of time
		if (gamepad->state.buttons == webConfigHotkeyMask) {
			if (is_nil_time(webConfigHotkeyHoldTimeout)) {
				webConfigHotkeyHoldTimeout = make_timeout_time_ms(WEBCONFIG_HOTKEY_HOLD_TIME_MS);
			}

			if (time_reached(webConfigHotkeyHoldTimeout)) {
				System::reboot(System::BootMode::WEBCONFIG);
			}
		} else {
			webConfigHotkeyHoldTimeout = nil_time;
		}

		// Pre-Process add-ons for MPGS
		addons.PreprocessAddons(ADDON_PROCESS::CORE0_INPUT);
		
		gamepad->process(); // process through MPGS

		// (Post) Process for add-ons
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

GP2040::BootAction GP2040::getBootAction() {
	switch (System::takeBootMode()) {
		case System::BootMode::WEBCONFIG: return BootAction::ENTER_WEBCONFIG_MODE;
		case System::BootMode::USB: return BootAction::ENTER_USB_MODE;
		default:
			{
				// Determine boot action based on gamepad state during boot
				Gamepad * gamepad = Storage::getInstance().GetGamepad();
				gamepad->read();

				if (gamepad->pressedF1() && gamepad->pressedUp()) {
					return BootAction::ENTER_USB_MODE;
				} else if (gamepad->pressedS2()) {
					return BootAction::ENTER_WEBCONFIG_MODE;
				} else if (gamepad->pressedB3()) {
					return BootAction::SET_INPUT_MODE_HID;
				} else if (gamepad->pressedB1()) {
					return BootAction::SET_INPUT_MODE_SWITCH;
				} else if (gamepad->pressedB2()) {
					return BootAction::SET_INPUT_MODE_XINPUT;
				} else {
					return BootAction::NONE;
				}
			}
	}
}
