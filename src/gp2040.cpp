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
#include "addons/keyboard_host.h"
#include "addons/i2canalog1219.h"
#include "addons/jslider.h"
#include "addons/playernum.h"
#include "addons/reverse.h"
#include "addons/turbo.h"
#include "addons/slider_socd.h"
#include "addons/wiiext.h"
#include "addons/input_macro.h"
#include "addons/snes_input.h"

// Pico includes
#include "pico/bootrom.h"
#include "pico/time.h"
#include "hardware/adc.h"

// TinyUSB
#include "usb_driver.h"
#include "tusb.h"


static const uint32_t REBOOT_HOTKEY_ACTIVATION_TIME_MS = 50;
static const uint32_t REBOOT_HOTKEY_HOLD_TIME_MS = 4000;

GP2040::GP2040() {
	Storage::getInstance().SetGamepad(new Gamepad());
	Storage::getInstance().SetProcessedGamepad(new Gamepad());
}

GP2040::~GP2040() {
}

void GP2040::setup() {
	// Reduce CPU if any USB host add-on is enabled
	const AddonOptions & addonOptions = Storage::getInstance().getAddonOptions();
	if ( addonOptions.keyboardHostOptions.enabled ||
			addonOptions.psPassthroughOptions.enabled ||
			addonOptions.xbonePassthroughOptions.enabled ){
	    set_sys_clock_khz(120000, true); // Set Clock to 120MHz to avoid potential USB timing issues
	}

    // Setup Gamepad and Gamepad Storage
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->setup();
    this->initializeStandardGpio();

    const GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();

    // check setup options and add modes to the list
    // user modes
    bootActions.insert({GAMEPAD_MASK_B1, gamepadOptions.inputModeB1});
    bootActions.insert({GAMEPAD_MASK_B2, gamepadOptions.inputModeB2});
    bootActions.insert({GAMEPAD_MASK_B3, gamepadOptions.inputModeB3});
    bootActions.insert({GAMEPAD_MASK_B4, gamepadOptions.inputModeB4});
    bootActions.insert({GAMEPAD_MASK_L1, gamepadOptions.inputModeL1});
    bootActions.insert({GAMEPAD_MASK_L2, gamepadOptions.inputModeL2});
    bootActions.insert({GAMEPAD_MASK_R1, gamepadOptions.inputModeR1});
    bootActions.insert({GAMEPAD_MASK_R2, gamepadOptions.inputModeR2});

	// Initialize our ADC (various add-ons)
	adc_init();

	// Setup Add-ons
	addons.LoadUSBAddon(new KeyboardHostAddon(), CORE0_INPUT);
	addons.LoadAddon(new AnalogInput(), CORE0_INPUT);
	addons.LoadAddon(new BootselButtonAddon(), CORE0_INPUT);
	addons.LoadAddon(new DualDirectionalInput(), CORE0_INPUT);
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
	addons.LoadAddon(new InputMacro(), CORE0_INPUT);


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
		case BootAction::SET_INPUT_MODE_XBONE:
		case BootAction::SET_INPUT_MODE_KEYBOARD:
		case BootAction::SET_INPUT_MODE_NEOGEO:
		case BootAction::SET_INPUT_MODE_MDMINI:
		case BootAction::SET_INPUT_MODE_PCEMINI:
		case BootAction::SET_INPUT_MODE_EGRET:
		case BootAction::SET_INPUT_MODE_ASTRO:
		case BootAction::SET_INPUT_MODE_PSCLASSIC:
		case BootAction::SET_INPUT_MODE_XBOXORIGINAL:
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
				} else if (bootAction == BootAction::SET_INPUT_MODE_XBONE) {
					inputMode = INPUT_MODE_XBONE;
				} else if (bootAction == BootAction::SET_INPUT_MODE_KEYBOARD) {
					inputMode = INPUT_MODE_KEYBOARD;
				} else if (bootAction == BootAction::SET_INPUT_MODE_NEOGEO) {
					inputMode = INPUT_MODE_NEOGEO;
				} else if (bootAction == BootAction::SET_INPUT_MODE_MDMINI) {
					inputMode = INPUT_MODE_MDMINI;
				} else if (bootAction == BootAction::SET_INPUT_MODE_PCEMINI) {
					inputMode = INPUT_MODE_PCEMINI;
				} else if (bootAction == BootAction::SET_INPUT_MODE_EGRET) {
					inputMode = INPUT_MODE_EGRET;
				} else if (bootAction == BootAction::SET_INPUT_MODE_ASTRO) {
					inputMode = INPUT_MODE_ASTRO;
				} else if (bootAction == BootAction::SET_INPUT_MODE_PSCLASSIC) {
					inputMode = INPUT_MODE_PSCLASSIC;
				} else if (bootAction == BootAction::SET_INPUT_MODE_XBOXORIGINAL) {
					inputMode = INPUT_MODE_XBOXORIGINAL;
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

/**
 * @brief Initialize standard input button GPIOs that are present in the currently loaded profile.
 */
void GP2040::initializeStandardGpio() {
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		// (NONE=-10, RESERVED=-5, ASSIGNED_TO_ADDON=0, everything else is ours)
		if (pinMappings[pin] > 0)
		{
			gpio_init(pin);             // Initialize pin
			gpio_set_dir(pin, GPIO_IN); // Set as INPUT
			gpio_pull_up(pin);          // Set as PULLUP
		}
	}
}

/**
 * @brief Deinitialize standard input button GPIOs that are present in the currently loaded profile.
 */
void GP2040::deinitializeStandardGpio() {
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		// (NONE=-10, RESERVED=-5, ASSIGNED_TO_ADDON=0, everything else is ours)
		if (pinMappings[pin] > 0)
		{
			gpio_deinit(pin);
		}
	}
}

void GP2040::run() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
	bool configMode = Storage::getInstance().GetConfigMode();
	uint8_t * featureData = Storage::getInstance().GetFeatureData();
	memset(featureData, 0, 32); // X-Input is the only feature data currently supported
	while (1) { // LOOP
		// check if we should reinitialize the gamepad
		if (gamepad->userRequestedReinit) {
			// deinitialize the ordinary (non-reserved, non-addon) GPIO pins, since
			// we are moving off of them and onto potentially different pin assignments
			// we currently don't support ASSIGNED_TO_ADDON pins being reinitialized,
			// but if they were to be, that'd be the addon's duty, not ours
			this->deinitializeStandardGpio();

			// now we can load the latest configured profile, which will map the
			// new set of GPIOs to use...
			Storage::getInstance().setFunctionalPinMappings();

			// ...and initialize the pins again
			this->initializeStandardGpio();

			// now we can tell the gamepad that the new mappings are in place
			// and ready to use, and the pins are ready, so it should reinitialize itself
			gamepad->reinit();
			// ...and addons on this core, if they implemented reinit (just things
			// with simple GPIO pin usage, at time of writing)
			addons.ReinitializeAddons(ADDON_PROCESS::CORE0_INPUT);

			// and we're done
			gamepad->userRequestedReinit = false;
		}

		Storage::getInstance().performEnqueuedSaves();
		// Config Loop (Web-Config does not require gamepad)
		if (configMode == true) {
			ConfigManager::getInstance().loop();
			gamepad->read();
			rebootHotkeys.process(gamepad, configMode);
			continue;
		}

		USBHostManager::getInstance().process();

		// Gamepad Features
		gamepad->read(); 	// gpio pin reads
		
		// Debounce if set
		if (Storage::getInstance().getGamepadOptions().debounceDelay)
			gamepad->debounce();

		// Pre-Process add-ons for MPGS
		addons.PreprocessAddons(ADDON_PROCESS::CORE0_INPUT);

		gamepad->hotkey(); 	// check for MPGS hotkeys
		rebootHotkeys.process(gamepad, configMode);
		
		gamepad->process(); // process through MPGS

		// (Post) Process for add-ons
		addons.ProcessAddons(ADDON_PROCESS::CORE0_INPUT);

		// Copy Processed Gamepad for Core1 (race condition otherwise)
		memcpy(&processedGamepad->state, &gamepad->state, sizeof(GamepadState));

		// Update input driver
		update_input_driver();

		// USB FEATURES : Send/Get USB Features (including Player LEDs on X-Input)
		if ( send_report(gamepad->getReport(), gamepad->getReportSize()) ) {
			gamepad->sendReportSuccess();
		}
		
		// GET USB REPORT (If Endpoint Available)
		receive_report(featureData);

		// Process USB Report Addons
		addons.ProcessAddons(ADDON_PROCESS::CORE0_USBREPORT);
		
		tud_task(); // TinyUSB Task update
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

                const ForcedSetupOptions& forcedSetupOptions = Storage::getInstance().getForcedSetupOptions();
                bool modeSwitchLocked = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_MODE_SWITCH ||
                                        forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

                bool webConfigLocked  = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_WEB_CONFIG ||
                                        forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

				if (gamepad->pressedS1() && gamepad->pressedS2() && gamepad->pressedUp()) {
					return BootAction::ENTER_USB_MODE;
				} else if (!webConfigLocked && gamepad->pressedS2()) {
					return BootAction::ENTER_WEBCONFIG_MODE;
                } else {
                    if (!modeSwitchLocked) {
                        if (auto search = bootActions.find(gamepad->state.buttons); search != bootActions.end()) {
                            switch (search->second) {
                                case INPUT_MODE_XINPUT: 
                                    return BootAction::SET_INPUT_MODE_XINPUT;
                                case INPUT_MODE_SWITCH: 
                                    return BootAction::SET_INPUT_MODE_SWITCH;
                                case INPUT_MODE_HID: 
                                    return BootAction::SET_INPUT_MODE_HID;
                                case INPUT_MODE_KEYBOARD: 
                                    return BootAction::SET_INPUT_MODE_KEYBOARD;
                                case INPUT_MODE_PS4: 
                                    return BootAction::SET_INPUT_MODE_PS4;
                                case INPUT_MODE_NEOGEO: 
                                    return BootAction::SET_INPUT_MODE_NEOGEO;
                                case INPUT_MODE_MDMINI: 
                                    return BootAction::SET_INPUT_MODE_MDMINI;
                                case INPUT_MODE_PCEMINI: 
                                    return BootAction::SET_INPUT_MODE_PCEMINI;
                                case INPUT_MODE_EGRET: 
                                    return BootAction::SET_INPUT_MODE_EGRET;
                                case INPUT_MODE_ASTRO: 
                                    return BootAction::SET_INPUT_MODE_ASTRO;
                                case INPUT_MODE_PSCLASSIC: 
                                    return BootAction::SET_INPUT_MODE_PSCLASSIC;
                                case INPUT_MODE_XBOXORIGINAL: 
                                    return BootAction::SET_INPUT_MODE_XBOXORIGINAL;
								                case INPUT_MODE_XBONE:
									                  return BootAction::SET_INPUT_MODE_XBONE;
                                default:
                                    return BootAction::NONE;
                            }
                        }
                    }
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
