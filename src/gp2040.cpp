// GP2040 includes
#include "gp2040.h"
#include "helper.h"
#include "system.h"
#include "enums.pb.h"

#include "build_info.h"
#include "configmanager.h" // Global Managers
#include "storagemanager.h"
#include "addonmanager.h"
#include "usbhostmanager.h"

#include "addons/analog.h" // Inputs for Core0
#include "addons/bootsel_button.h"
#include "addons/focus_mode.h"
#include "addons/dualdirectional.h"
#include "addons/tilt.h"
#include "addons/extra_button.h"
#include "addons/keyboard_host.h"
#include "addons/i2canalog1219.h"
#include "addons/jslider.h"
#include "addons/playernum.h"
#include "addons/reverse.h"
#include "addons/turbo.h"
#include "addons/slider_socd.h"
#include "addons/wiiext.h"
#include "addons/snes_input.h"

// Pico includes
#include "pico/bootrom.h"
#include "pico/time.h"
#include "hardware/adc.h"

// TinyUSB
#include "usb_driver.h"
#include "tusb.h"

#define GAMEPAD_DEBOUNCE_MILLIS 5 // make this a class object

static const uint32_t REBOOT_HOTKEY_ACTIVATION_TIME_MS = 50;
static const uint32_t REBOOT_HOTKEY_HOLD_TIME_MS = 4000;

GP2040::GP2040() : nextRuntime(0) {
	Storage::getInstance().SetGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
	Storage::getInstance().SetProcessedGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
}

GP2040::~GP2040() {
}

void GP2040::setup() {
	// Reduce CPU if any USB host add-on is enabled
	const AddonOptions & addonOptions = Storage::getInstance().getAddonOptions();
	if ( addonOptions.keyboardHostOptions.enabled ||
			addonOptions.psPassthroughOptions.enabled ){
	    set_sys_clock_khz(120000, true); // Set Clock to 120MHz to avoid potential USB timing issues
	}

    // Setup Gamepad and Gamepad Storage
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->setup();

	// Initialize our ADC (various add-ons)
	adc_init();

	// Setup Add-ons
	addons.LoadUSBAddon(new KeyboardHostAddon(), CORE0_INPUT);
	addons.LoadAddon(new AnalogInput(), CORE0_INPUT);
	addons.LoadAddon(new BootselButtonAddon(), CORE0_INPUT);
	addons.LoadAddon(new DualDirectionalInput(), CORE0_INPUT);
	addons.LoadAddon(new ExtraButtonAddon(), CORE0_INPUT);
	addons.LoadAddon(new FocusModeAddon(), CORE0_INPUT);
	addons.LoadAddon(new I2CAnalog1219Input(), CORE0_INPUT);
	addons.LoadAddon(new JSliderInput(), CORE0_INPUT);
	addons.LoadAddon(new ReverseInput(), CORE0_INPUT);
	addons.LoadAddon(new TurboInput(), CORE0_INPUT);
	addons.LoadAddon(new WiiExtensionInput(), CORE0_INPUT);
	addons.LoadAddon(new SNESpadInput(), CORE0_INPUT);
	addons.LoadAddon(new PlayerNumAddon(), CORE0_USBREPORT);
	addons.LoadAddon(new SliderSOCDInput(), CORE0_INPUT);
	addons.LoadAddon(new TiltInput(), CORE0_INPUT);


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
		case BootAction::SET_INPUT_MODE_PS4:
		case BootAction::SET_INPUT_MODE_KEYBOARD:
		case BootAction::NONE:
			{
				InputMode inputMode = gamepad->getOptions().inputMode;
				if (bootAction == BootAction::SET_INPUT_MODE_HID) {
					inputMode = INPUT_MODE_HID;
				} else if (bootAction == BootAction::SET_INPUT_MODE_SWITCH) {
					inputMode = INPUT_MODE_SWITCH;
				} else if (bootAction == BootAction::SET_INPUT_MODE_XINPUT) {
					inputMode = INPUT_MODE_XINPUT;
				} else if (bootAction == BootAction::SET_INPUT_MODE_PS4) {
					inputMode = INPUT_MODE_PS4;
				} else if (bootAction == BootAction::SET_INPUT_MODE_KEYBOARD) {
					inputMode = INPUT_MODE_KEYBOARD;
				}

				if (inputMode != gamepad->getOptions().inputMode) {
					// Save the changed input mode
					gamepad->setInputMode(inputMode);
					gamepad->save();
				}

				initialize_driver(inputMode);
				break;
			}
	}
}

void GP2040::run() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
	bool configMode = Storage::getInstance().GetConfigMode();
	while (1) { // LOOP
		Storage::getInstance().performEnqueuedSaves();
		// Config Loop (Web-Config does not require gamepad)
		if (configMode == true) {
			ConfigManager& configManager = ConfigManager::getInstance();
			ConfigManager::getInstance().loop();

			gamepad->read();
			rebootHotkeys.process(gamepad, configMode);

			continue;
		}

		USBHostManager::getInstance().process();

		// We can't send faster than USB can poll
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
		rebootHotkeys.process(gamepad, configMode);

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

		// Process USB Reports
		addons.ProcessAddons(ADDON_PROCESS::CORE0_USBREPORT);

		tud_task(); // TinyUSB Task update

		nextRuntime = getMicro() + GAMEPAD_POLL_MICRO;
	}
}

GP2040::BootAction GP2040::getBootAction() {
	switch (System::takeBootMode()) {
		case System::BootMode::GAMEPAD: return BootAction::NONE;
		case System::BootMode::WEBCONFIG: return BootAction::ENTER_WEBCONFIG_MODE;
		case System::BootMode::USB: return BootAction::ENTER_USB_MODE;
		case System::BootMode::DEFAULT:
			{
				// Determine boot action based on gamepad state during boot
				Gamepad * gamepad = Storage::getInstance().GetGamepad();
				Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
				
				gamepad->read();

				// Pre-Process add-ons for MPGS
				addons.PreprocessAddons(ADDON_PROCESS::CORE0_INPUT);
				
				gamepad->process(); // process through MPGS

				// (Post) Process for add-ons
				addons.ProcessAddons(ADDON_PROCESS::CORE0_INPUT);

				// Copy Processed Gamepad for Core1 (race condition otherwise)
				memcpy(&processedGamepad->state, &gamepad->state, sizeof(GamepadState));

				ForcedSetupOptions& forcedSetupOptions = Storage::getInstance().getForcedSetupOptions();
				bool modeSwitchLocked = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_MODE_SWITCH ||
										forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

				bool webConfigLocked  = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_WEB_CONFIG ||
										forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

				if (gamepad->pressedS1() && gamepad->pressedS2() && gamepad->pressedUp()) {
					return BootAction::ENTER_USB_MODE;
				} else if (!webConfigLocked && gamepad->pressedS2()) {
					return BootAction::ENTER_WEBCONFIG_MODE;
				} else if (!modeSwitchLocked && gamepad->pressedB3()) { // P1
					return BootAction::SET_INPUT_MODE_HID;
				} else if (!modeSwitchLocked && gamepad->pressedB4()) { // P2
					return BootAction::SET_INPUT_MODE_PS4;
				} else if (!modeSwitchLocked && gamepad->pressedB1()) { // K1
					return BootAction::SET_INPUT_MODE_SWITCH;
				} else if (!modeSwitchLocked && gamepad->pressedB2()) { // K2
					return BootAction::SET_INPUT_MODE_XINPUT;
				} else if (!modeSwitchLocked && gamepad->pressedR2()) { // K3
					return BootAction::SET_INPUT_MODE_KEYBOARD;
				} else {
					return BootAction::NONE;
				}

				break;
			}
	}

	return BootAction::NONE;
}

GP2040::RebootHotkeys::RebootHotkeys() :
	active(false),
	noButtonsPressedTimeout(nil_time),
	webConfigHotkeyMask(GAMEPAD_MASK_S2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4),
	bootselHotkeyMask(GAMEPAD_MASK_S1 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4),
	rebootHotkeysHoldTimeout(nil_time) {
}

void GP2040::RebootHotkeys::process(Gamepad* gamepad, bool configMode) {
	// We only allow the hotkey to trigger after we observed no buttons pressed for a certain period of time.
	// We do this to avoid detecting buttons that are held during the boot process. In particular we want to avoid
	// oscillating between webconfig and default mode when the user keeps holding the hotkey buttons.
	if (!active) {
		if (gamepad->state.buttons == 0) {
			if (is_nil_time(noButtonsPressedTimeout)) {
				noButtonsPressedTimeout = make_timeout_time_us(REBOOT_HOTKEY_ACTIVATION_TIME_MS);
			}

			if (time_reached(noButtonsPressedTimeout)) {
				active = true;
			}
		} else {
			noButtonsPressedTimeout = nil_time;
		}
	} else {
		if (gamepad->state.buttons == webConfigHotkeyMask || gamepad->state.buttons == bootselHotkeyMask) {
			if (is_nil_time(rebootHotkeysHoldTimeout)) {
				rebootHotkeysHoldTimeout = make_timeout_time_ms(REBOOT_HOTKEY_HOLD_TIME_MS);
			}

			if (time_reached(rebootHotkeysHoldTimeout)) {
				if (gamepad->state.buttons == webConfigHotkeyMask) {
					// If we are in webconfig mode we go to gamepad mode and vice versa
					System::reboot(configMode ? System::BootMode::GAMEPAD : System::BootMode::WEBCONFIG);
				} else if (gamepad->state.buttons == bootselHotkeyMask) {
					System::reboot(System::BootMode::USB);
				}
			}
		} else {
			rebootHotkeysHoldTimeout = nil_time;
		}
	}
}
