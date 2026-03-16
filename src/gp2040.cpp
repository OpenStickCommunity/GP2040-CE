#include <optional>

// GP2040 includes
#include "gp2040.h"
#include "helper.h"
#include "system.h"
#include "enums.pb.h"

#include "build_info.h"
#include "peripheralmanager.h"
#include "storagemanager.h"
#include "addonmanager.h"
#include "types.h"
#include "usbhostmanager.h"

// Inputs for Core0
#include "addons/analog.h"
#include "addons/bootsel_button.h"
#include "addons/focus_mode.h"
#include "addons/dualdirectional.h"
#include "addons/tilt.h"
#include "addons/keyboard_host.h"
#include "addons/i2canalog1219.h"
#include "addons/reverse.h"
#include "addons/turbo.h"
#include "addons/slider_socd.h"
#include "addons/spi_analog_ads1256.h"
#include "addons/wiiext.h"
#include "addons/input_macro.h"
#include "addons/snes_input.h"
#include "addons/rotaryencoder.h"
#include "addons/i2c_gpio_pcf8575.h"
#include "addons/gamepad_usb_host.h"
#include "addons/he_trigger.h"
#include "addons/tg16_input.h"

// Pico includes
#include "pico/bootrom.h"
#include "pico/time.h"
#include "hardware/adc.h"

#include "rndis.h"

// TinyUSB
#include "tusb.h"

// USB Input Class Drivers
#include "drivermanager.h"

static const uint32_t REBOOT_HOTKEY_ACTIVATION_TIME_MS = 50;
static const uint32_t REBOOT_HOTKEY_HOLD_TIME_MS = 4000;

const static uint32_t rebootDelayMs = 500;
static absolute_time_t rebootDelayTimeout = nil_time;


void GP2040::setup() {
	Storage::getInstance().init();

	// Reduce CPU if USB host is enabled
	PeripheralManager::getInstance().initUSB();
	if ( PeripheralManager::getInstance().isUSBEnabled(0) ) {
		set_sys_clock_khz(120000, true); // Set Clock to 120MHz to avoid potential USB timing issues
	}

	// I2C & SPI rely on the system clock
	PeripheralManager::getInstance().initSPI();
	PeripheralManager::getInstance().initI2C();

	Gamepad * gamepad = new Gamepad();
	Gamepad * processedGamepad = new Gamepad();
	Storage::getInstance().SetGamepad(gamepad);
	Storage::getInstance().SetProcessedGamepad(processedGamepad);

	BootModeOptions& bootModeOptions = Storage::getInstance().getBootModeOptions();
	BootAction bootAction;

	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	uint32_t prevProfile = gamepadOptions.profileNumber;
	bool profileChanged = false;

	if (bootModeOptions.enabled) {
		bootAction = getGpioMappedBootAction();
		profileChanged = bootAction.profileNumber != prevProfile;
		gamepadOptions.profileNumber = bootAction.profileNumber;
	}

	// Set pin mappings for all GPIO functions
	Storage::getInstance().setFunctionalPinMappings();

	// power up...
	gamepad->auxState.power.pluggedIn = true;
	gamepad->auxState.power.charging = false;
	gamepad->auxState.power.level = GAMEPAD_AUX_MAX_POWER;

	// Setup Gamepad
	gamepad->setup();

	// now we can load the latest configured profile, which will map the
	// new set of GPIOs to use...
  this->initializeStandardGpio();

	// Initialize our ADC (various add-ons)
	adc_init();

	// Setup Add-ons
	addons.LoadUSBAddon(new KeyboardHostAddon());
	addons.LoadUSBAddon(new GamepadUSBHostAddon());
	addons.LoadAddon(new AnalogInput());
	addons.LoadAddon(new HETriggerAddon());
	addons.LoadAddon(new BootselButtonAddon());
	addons.LoadAddon(new DualDirectionalInput());
	addons.LoadAddon(new FocusModeAddon());
	addons.LoadAddon(new I2CAnalog1219Input());
	addons.LoadAddon(new SPIAnalog1256Input());
	addons.LoadAddon(new WiiExtensionInput());
	addons.LoadAddon(new SNESpadInput());
	addons.LoadAddon(new SliderSOCDInput());
	addons.LoadAddon(new TiltInput());
	addons.LoadAddon(new RotaryEncoderInput());
	addons.LoadAddon(new PCF8575Addon());
	addons.LoadAddon(new TG16padInput());

	// Input override addons
	addons.LoadAddon(new ReverseInput());
	addons.LoadAddon(new TurboInput()); // Turbo overrides button states and should be close to the end
	addons.LoadAddon(new InputMacro());

	if (!bootModeOptions.enabled) {
		// Use the old method of selecting input mode via mapped button, i.e. AFTER initializing GPIO
		// pins with the currently active profile
		bootAction = getButtonMappedBootAction();
	}

	// Initialize last reinit profile to current so we don't reinit on first loop
	gamepad->lastReinitProfileNumber = bootAction.profileNumber;

	if (bootAction.type == BootActionType::ENTER_USB_MODE) {
		reset_usb_boot(0, 0);
		return;
	}

	InputMode inputMode = bootAction.inputMode;
	uint32_t profile = bootAction.profileNumber;

	// Setup USB Driver
	DriverManager::getInstance().setup(inputMode);

	if (inputMode != INPUT_MODE_CONFIG) {
		bool inputModeChanged = inputMode != gamepadOptions.inputMode;
		if (inputModeChanged) gamepad->setInputMode(inputMode);

		// save to match user expectations on choosing mode at boot, and this is
		// before USB host will be used so we can force it to ignore the check
		if (inputModeChanged || profileChanged) Storage::getInstance().save(true);
	}

	// register system event handlers
	EventManager::getInstance().registerEventHandler(GP_EVENT_STORAGE_SAVE, GPEVENT_CALLBACK(this->handleStorageSave(event)));
	EventManager::getInstance().registerEventHandler(GP_EVENT_RESTART, GPEVENT_CALLBACK(this->handleSystemReboot(event)));
}

/**
 * @brief Initialize standard input button GPIOs that are present in the currently loaded profile.
 */
void GP2040::initializeStandardGpio() {
	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
	buttonGpios = 0;
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		// (NONE=-10, RESERVED=-5, ASSIGNED_TO_ADDON=0, everything else is ours)
		if (pinMappings[pin].action > 0)
		{
			gpio_init(pin);             // Initialize pin
			gpio_set_dir(pin, GPIO_IN); // Set as INPUT
			gpio_pull_up(pin);          // Set as PULLUP
			buttonGpios |= 1 << pin;    // mark this pin as mattering for GPIO debouncing
		}
	}
}

/**
 * @brief Deinitialize standard input button GPIOs that are present in the currently loaded profile.
 */
void GP2040::deinitializeStandardGpio() {
	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		// (NONE=-10, RESERVED=-5, ASSIGNED_TO_ADDON=0, everything else is ours)
		if (pinMappings[pin].action > 0)
		{
			gpio_deinit(pin);
		}
	}
}

/**
 * @brief Populate a debounced version of gpio_get_all suitable for use for buttons.
 *
 * For GPIO that are assigned to buttons (based on GpioMappings, see GP2040::initializeStandardGpio),
 * we can centralize their debouncing here and provide access to it to button users.
 *
 * For ease of use this provides the mask bitwise NOTed so that callers don't have to. To avoid misuse
 * and to simplify this method, non-button GPIO IS NOT PRESENT in this result. Use gpio_get_all directly
 * instead, if you don't want debounced data.
 */
void GP2040::debounceGpioGetAll() {
	Mask_t raw_gpio = ~gpio_get_all();
	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	// return if state isn't different than the actual
	if (gamepad->debouncedGpio == (raw_gpio & buttonGpios)) return;

	uint32_t debounceDelay = Storage::getInstance().getGamepadOptions().debounceDelay;
	// abort if no delay is configured
	if (debounceDelay == 0) {
		gamepad->debouncedGpio = raw_gpio;
		return;
	}

	uint32_t now = getMillis();
	// check each button use case GPIO for state
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
		Mask_t pin_mask = 1 << pin;
		if (buttonGpios & pin_mask) {
			// Allow debouncer to change state if button state changed and debounce delay threshold met
			if ((gamepad->debouncedGpio & pin_mask) != \
					(raw_gpio & pin_mask) && ((now - gpioDebounceTime[pin]) > debounceDelay)) {
				gamepad->debouncedGpio ^= pin_mask;
				gpioDebounceTime[pin] = now;
			}
		}
	}
}

void GP2040::run() {
	bool configMode = DriverManager::getInstance().isConfigMode();
	GPDriver * inputDriver = DriverManager::getInstance().getDriver();
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
	GamepadState prevState;

	// Start the TinyUSB Device functionality
	tud_init(TUD_OPT_RHPORT);

	// Initialize our USB manager
	USBHostManager::getInstance().start();

	if (configMode == true ) {
		rndis_init();
	}

	while (1) { // LOOP
		this->getReinitGamepad(gamepad);

		memcpy(&prevState, &gamepad->state, sizeof(GamepadState));

		// Debounce
		debounceGpioGetAll();
		// Read Gamepad
		gamepad->read();

		checkRawState(prevState, gamepad->state);

		// Process USB Host on Core0
		USBHostManager::getInstance().process();

		// Config Loop (Web-Config skips Core0 add-ons)
		if (configMode == true) {
			inputDriver->process(gamepad);
			rebootHotkeys.process(gamepad, configMode);
			checkSaveRebootState();
			continue;
		}

		// Pre-Process add-ons for MPGS
		addons.PreprocessAddons();

		gamepad->hotkey(); 	// check for MPGS hotkeys
		rebootHotkeys.process(gamepad, configMode);

		gamepad->process(); // process through MPGS

		// (Post) Process for add-ons
		addons.ProcessAddons();

		checkProcessedState(processedGamepad->state, gamepad->state);

		// Copy Processed Gamepad for Core1 (race condition otherwise)
		memcpy(&processedGamepad->state, &gamepad->state, sizeof(GamepadState));

		// Process Input Driver
		bool processed = inputDriver->process(gamepad);

		// TinyUSB Task update
		tud_task();

		// Post-Process Add-ons with USB Report Processed Sent
		addons.PostprocessAddons(processed);

		// Check if we have a pending save
		checkSaveRebootState();
	}
}

void GP2040::getReinitGamepad(Gamepad * gamepad) {
	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();

	// Check if profile has changed since last reinit
	if (gamepad->lastReinitProfileNumber != gamepadOptions.profileNumber) {
		uint32_t previousProfile = gamepad->lastReinitProfileNumber;
		uint32_t currentProfile = gamepadOptions.profileNumber;

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
		addons.ReinitializeAddons();

		// Update the last reinit profile
		gamepad->lastReinitProfileNumber = currentProfile;

		// Trigger the profile change event now that reinit is complete
		EventManager::getInstance().triggerEvent(new GPProfileChangeEvent(previousProfile, currentProfile));
	}
}

GP2040::BootAction GP2040::getButtonMappedBootAction() {
	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	BootAction bootAction = {
		BootActionType::SET_INPUT_MODE,
		gamepadOptions.inputMode,
		gamepadOptions.profileNumber
	};

	switch (System::takeBootMode()) {
		case System::BootMode::GAMEPAD:
			return bootAction;
		case System::BootMode::WEBCONFIG:
			bootAction.inputMode = InputMode::INPUT_MODE_CONFIG;
			return bootAction;
		case System::BootMode::USB:
			bootAction.type = BootActionType::ENTER_USB_MODE;
			return bootAction;
		case System::BootMode::DEFAULT:
			break;
	}
	// Determine boot action based on gamepad state during boot
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();

	debounceGpioGetAll();
	gamepad->read();

	// Pre-Process add-ons for MPGS
	addons.PreprocessAddons();

	gamepad->process(); // process through MPGS

	// Process for add-ons
	addons.ProcessAddons();

	// Copy Processed Gamepad for Core1 (race condition otherwise)
	memcpy(&processedGamepad->state, &gamepad->state, sizeof(GamepadState));

	const ForcedSetupOptions& forcedSetupOptions = Storage::getInstance().getForcedSetupOptions();
	bool modeSwitchLocked = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_MODE_SWITCH ||
													forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

	bool webConfigLocked  = forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_WEB_CONFIG ||
													forcedSetupOptions.mode == FORCED_SETUP_MODE_LOCK_BOTH;

	if (gamepad->pressedS1() && gamepad->pressedS2() && gamepad->pressedUp()) {
		bootAction.type = BootActionType::ENTER_USB_MODE;
		return bootAction;
	}
	if (!webConfigLocked && gamepad->pressedS2()) {
		bootAction.inputMode =  InputMode::INPUT_MODE_CONFIG;
		return bootAction;
	}
	// input mask, action
	std::map<uint32_t, int32_t> bootActions;

	// check setup options and add modes to the list
	bootActions.insert({GAMEPAD_MASK_B1, gamepadOptions.inputModeB1});
	bootActions.insert({GAMEPAD_MASK_B2, gamepadOptions.inputModeB2});
	bootActions.insert({GAMEPAD_MASK_B3, gamepadOptions.inputModeB3});
	bootActions.insert({GAMEPAD_MASK_B4, gamepadOptions.inputModeB4});
	bootActions.insert({GAMEPAD_MASK_L1, gamepadOptions.inputModeL1});
	bootActions.insert({GAMEPAD_MASK_L2, gamepadOptions.inputModeL2});
	bootActions.insert({GAMEPAD_MASK_R1, gamepadOptions.inputModeR1});
	bootActions.insert({GAMEPAD_MASK_R2, gamepadOptions.inputModeR2});

	if (!modeSwitchLocked) {
		if (auto search = bootActions.find(gamepad->state.buttons); search != bootActions.end()) {
			bootAction.inputMode = static_cast<InputMode>(search->second);
			return bootAction;
		}
	}
	return bootAction;
}

/**
 * @brief Get input mode and profile to load at startup via mapped GPIO pins
 *
 * GPIO pins are initialized by constructing a temporary, virtual profile using BootModeOptions and
 * de-initialized before returning the BootAction.
 */
GP2040::BootAction GP2040::getGpioMappedBootAction() {
	Storage::getInstance().setBootModeFunctionalPinMappings();
	initializeStandardGpio();

	const GamepadOptions& gamepad = Storage::getInstance().getGamepadOptions();
	const BootModeOptions& bootModeOptions = Storage::getInstance().getBootModeOptions();
	BootAction action = { BootActionType::SET_INPUT_MODE, gamepad.inputMode, gamepad.profileNumber };

	switch (System::takeBootMode()) {
		case System::BootMode::GAMEPAD:
			return action;
		case System::BootMode::WEBCONFIG:
			action.inputMode = InputMode::INPUT_MODE_CONFIG;
			return action;
		case System::BootMode::USB:
			action.type = BootActionType::ENTER_USB_MODE;
			return action;
		default:
			break;
	}
	Mask_t gpio = ~gpio_get_all() & buttonGpios;

	if (gpio == bootModeOptions.usbModePinMask) {
		action.type = BootActionType::ENTER_USB_MODE;
		return action;
	}

	if (gpio == bootModeOptions.webConfigPinMask) {
		action.inputMode = InputMode::INPUT_MODE_CONFIG;
		return action;
	}

	for (size_t i = 0; i < bootModeOptions.inputModeMappings_count; i++) {
		InputModeMapping m = bootModeOptions.inputModeMappings[i];
		if (m.pinMask < 0)
			continue;
		if (gpio == static_cast<Mask_t>(m.pinMask)) {
			action.inputMode = static_cast<InputMode>(m.inputMode);
			if (m.profileNumber > 0) {
				action.profileNumber = m.profileNumber;
			}
			break;
		}
	}

	deinitializeStandardGpio();
	return action;
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

void GP2040::checkRawState(GamepadState prevState, GamepadState currState) {
    // buttons pressed
    if (
        ((currState.aux & ~prevState.aux) != 0) ||
        ((currState.dpad & ~prevState.dpad) != 0) ||
        ((currState.buttons & ~prevState.buttons) != 0)
    ) {
        EventManager::getInstance().triggerEvent(new GPButtonDownEvent((currState.dpad & ~prevState.dpad), (currState.buttons & ~prevState.buttons), (currState.aux & ~prevState.aux)));
    }

    // buttons released
    if (
        ((prevState.aux & ~currState.aux) != 0) ||
        ((prevState.dpad & ~currState.dpad) != 0) ||
        ((prevState.buttons & ~currState.buttons) != 0)
    ) {
        EventManager::getInstance().triggerEvent(new GPButtonUpEvent((prevState.dpad & ~currState.dpad), (prevState.buttons & ~currState.buttons), (prevState.aux & ~currState.aux)));
    }
}

void GP2040::checkProcessedState(GamepadState prevState, GamepadState currState) {
    // buttons pressed
    if (
        ((currState.aux & ~prevState.aux) != 0) ||
        ((currState.dpad & ~prevState.dpad) != 0) ||
        ((currState.buttons & ~prevState.buttons) != 0)
    ) {
        EventManager::getInstance().triggerEvent(new GPButtonProcessedDownEvent((currState.dpad & ~prevState.dpad), (currState.buttons & ~prevState.buttons), (currState.aux & ~prevState.aux)));
    }

    // buttons released
    if (
        ((prevState.aux & ~currState.aux) != 0) ||
        ((prevState.dpad & ~currState.dpad) != 0) ||
        ((prevState.buttons & ~currState.buttons) != 0)
    ) {
        EventManager::getInstance().triggerEvent(new GPButtonProcessedUpEvent((prevState.dpad & ~currState.dpad), (prevState.buttons & ~currState.buttons), (prevState.aux & ~currState.aux)));
    }

    if (
        (currState.lx != prevState.lx) ||
        (currState.ly != prevState.ly) ||
        (currState.rx != prevState.rx) ||
        (currState.ry != prevState.ry) ||
        (currState.lt != prevState.lt) ||
        (currState.rt != prevState.rt)
    ) {
        EventManager::getInstance().triggerEvent(new GPAnalogProcessedMoveEvent(currState.lx, currState.ly, currState.rx, currState.ry, currState.lt, currState.rt));
    }
}

void GP2040::checkSaveRebootState() {
	if (saveRequested) {
		saveRequested = false;
		Storage::getInstance().save(forceSave);
	}

	if (rebootRequested) {
		rebootRequested = false;
		rebootDelayTimeout = make_timeout_time_ms(rebootDelayMs);
	}

	if (!is_nil_time(rebootDelayTimeout) && time_reached(rebootDelayTimeout)) {
		System::reboot(rebootMode);
	}
}

void GP2040::handleStorageSave(GPEvent* e) {
	saveRequested = true;
	forceSave = ((GPStorageSaveEvent*)e)->forceSave;
	rebootRequested = ((GPStorageSaveEvent*)e)->restartAfterSave;
	rebootMode = System::BootMode::DEFAULT;
}

void GP2040::handleSystemReboot(GPEvent* e) {
	rebootRequested = true;
	rebootMode = ((GPRestartEvent*)e)->bootMode;
}
