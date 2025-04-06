#include "config_utils.h"

#include "config.pb.h"
#include "enums.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "types.h"

#include "BoardConfig.h"
#include "GamepadConfig.h"
#include "version.h"
#include "addons/analog.h"
#include "addons/board_led.h"
#include "addons/bootsel_button.h"
#include "addons/buzzerspeaker.h"
#include "addons/dualdirectional.h"
#include "addons/tilt.h"
#include "addons/focus_mode.h"
#include "addons/i2canalog1219.h"
#include "addons/display.h"
#include "addons/keyboard_host.h"
#include "addons/neopicoleds.h"
#include "addons/pleds.h"
#include "addons/reactiveleds.h"
#include "addons/reverse.h"
#include "addons/slider_socd.h"
#include "addons/spi_analog_ads1256.h"
#include "addons/turbo.h"
#include "addons/wiiext.h"
#include "addons/snes_input.h"
#include "addons/input_macro.h"
#include "addons/rotaryencoder.h"
#include "addons/i2c_gpio_pcf8575.h"
#include "addons/drv8833_rumble.h"
#include "addons/gamepad_usb_host.h"

#include "CRC32.h"
#include "FlashPROM.h"
#include "base64.h"

#include <ArduinoJson.h>

#include <cassert>
#include <cstring>
#include <memory>

#include "pico/platform.h"

// -----------------------------------------------------
// Default values
// -----------------------------------------------------

#define PREPROCESSOR_JOIN2(x, y) x ## y
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN2(x, y)

#define INIT_UNSET_PROPERTY(parent, property, value) \
    if (!parent.PREPROCESSOR_JOIN(has_, property)) \
    { \
        parent.property = value; \
        parent.PREPROCESSOR_JOIN(has_, property) = true; \
    } \

#define INIT_UNSET_PROPERTY_STR(parent, property, str) \
    if (!parent.PREPROCESSOR_JOIN(has_, property)) \
    { \
        strncpy(parent.property, str, sizeof(parent.property)); \
        parent.property[sizeof(parent.property) - 1] = '\0'; \
        parent.PREPROCESSOR_JOIN(has_, property) = true; \
    } \

#define INIT_UNSET_PROPERTY_BYTES(parent, property, byteArray) \
    if (!parent.PREPROCESSOR_JOIN(has_, property)) \
    { \
        parent.property.size = sizeof(byteArray); \
        memcpy(parent.property.bytes, byteArray, std::min(sizeof(byteArray), sizeof(parent.property.bytes))); \
        parent.PREPROCESSOR_JOIN(has_, property) = true; \
    } \

#ifndef DEFAULT_INPUT_MODE
    #define DEFAULT_INPUT_MODE INPUT_MODE_XINPUT
#endif
#ifndef DEFAULT_INPUT_MODE_B1
    #define DEFAULT_INPUT_MODE_B1 INPUT_MODE_SWITCH
#endif
#ifndef DEFAULT_INPUT_MODE_B2
    #define DEFAULT_INPUT_MODE_B2 INPUT_MODE_XINPUT
#endif
#ifndef DEFAULT_INPUT_MODE_B3
    #define DEFAULT_INPUT_MODE_B3 INPUT_MODE_PS3
#endif
#ifndef DEFAULT_INPUT_MODE_B4
    #define DEFAULT_INPUT_MODE_B4 INPUT_MODE_PS4
#endif
#ifndef DEFAULT_INPUT_MODE_L1
    #define DEFAULT_INPUT_MODE_L1 -1
#endif
#ifndef DEFAULT_INPUT_MODE_L2
    #define DEFAULT_INPUT_MODE_L2 -1
#endif
#ifndef DEFAULT_INPUT_MODE_R1
    #define DEFAULT_INPUT_MODE_R1 -1
#endif
#ifndef DEFAULT_INPUT_MODE_R2
    #define DEFAULT_INPUT_MODE_R2 INPUT_MODE_KEYBOARD
#endif
#ifndef DEFAULT_DPAD_MODE
    #define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#endif
#ifndef DEFAULT_SOCD_MODE
    #define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#endif
#ifndef DEFAULT_FORCED_SETUP_MODE
    #define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_OFF
#endif
#ifndef DEFAULT_LOCK_HOTKEYS
    #define DEFAULT_LOCK_HOTKEYS false
#endif
#ifndef DEFAULT_PS4CONTROLLER_TYPE
    #define DEFAULT_PS4CONTROLLER_TYPE PS4_CONTROLLER
#endif

#ifndef DEFAULT_DEBOUNCE_DELAY
    #define DEFAULT_DEBOUNCE_DELAY 5
#endif

#ifndef DEFAULT_PS4_REPORTHACK
    #define DEFAULT_PS4_REPORTHACK false
#endif

#ifndef DEFAULT_PS4AUTHENTICATION_TYPE
    #define DEFAULT_PS4AUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_NONE
#endif

#ifndef DEFAULT_PS5AUTHENTICATION_TYPE
    #define DEFAULT_PS5AUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_NONE
#endif

#ifndef DEFAULT_XINPUTAUTHENTICATION_TYPE
    #define DEFAULT_XINPUTAUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_NONE
#endif

#ifndef DEFAULT_PS4_ID_MODE
    #define DEFAULT_PS4_ID_MODE PS4_ID_CONSOLE
#endif

#ifndef DEFAULT_USB_DESC_OVERRIDE
   #define DEFAULT_USB_DESC_OVERRIDE false
#endif

#ifndef DEFAULT_USB_DESC_PRODUCT
   #define DEFAULT_USB_DESC_PRODUCT "GP2040-CE (Custom)"
#endif

#ifndef DEFAULT_USB_DESC_MANUFACTURER
   #define DEFAULT_USB_DESC_MANUFACTURER "Open Stick Community"
#endif

#ifndef DEFAULT_USB_DESC_VERSION
   #define DEFAULT_USB_DESC_VERSION "1.0"
#endif

#ifndef DEFAULT_USB_ID_OVERRIDE
   #define DEFAULT_USB_ID_OVERRIDE false
#endif

#ifndef DEFAULT_USB_VENDOR_ID
   #define DEFAULT_USB_VENDOR_ID 0x10C4
#endif

#ifndef DEFAULT_USB_PRODUCT_ID
   #define DEFAULT_USB_PRODUCT_ID 0x82C0
#endif

#ifndef MINI_MENU_GAMEPAD_INPUT
   #define MINI_MENU_GAMEPAD_INPUT 0
#endif

#ifndef GPIO_PIN_00
    #define GPIO_PIN_00 GpioAction::NONE
#endif
#ifndef GPIO_PIN_01
    #define GPIO_PIN_01 GpioAction::NONE
#endif
#ifndef GPIO_PIN_02
    #define GPIO_PIN_02 GpioAction::NONE
#endif
#ifndef GPIO_PIN_03
    #define GPIO_PIN_03 GpioAction::NONE
#endif
#ifndef GPIO_PIN_04
    #define GPIO_PIN_04 GpioAction::NONE
#endif
#ifndef GPIO_PIN_05
    #define GPIO_PIN_05 GpioAction::NONE
#endif
#ifndef GPIO_PIN_06
    #define GPIO_PIN_06 GpioAction::NONE
#endif
#ifndef GPIO_PIN_07
    #define GPIO_PIN_07 GpioAction::NONE
#endif
#ifndef GPIO_PIN_08
    #define GPIO_PIN_08 GpioAction::NONE
#endif
#ifndef GPIO_PIN_09
    #define GPIO_PIN_09 GpioAction::NONE
#endif
#ifndef GPIO_PIN_10
    #define GPIO_PIN_10 GpioAction::NONE
#endif
#ifndef GPIO_PIN_11
    #define GPIO_PIN_11 GpioAction::NONE
#endif
#ifndef GPIO_PIN_12
    #define GPIO_PIN_12 GpioAction::NONE
#endif
#ifndef GPIO_PIN_13
    #define GPIO_PIN_13 GpioAction::NONE
#endif
#ifndef GPIO_PIN_14
    #define GPIO_PIN_14 GpioAction::NONE
#endif
#ifndef GPIO_PIN_15
    #define GPIO_PIN_15 GpioAction::NONE
#endif
#ifndef GPIO_PIN_16
    #define GPIO_PIN_16 GpioAction::NONE
#endif
#ifndef GPIO_PIN_17
    #define GPIO_PIN_17 GpioAction::NONE
#endif
#ifndef GPIO_PIN_18
    #define GPIO_PIN_18 GpioAction::NONE
#endif
#ifndef GPIO_PIN_19
    #define GPIO_PIN_19 GpioAction::NONE
#endif
#ifndef GPIO_PIN_20
    #define GPIO_PIN_20 GpioAction::NONE
#endif
#ifndef GPIO_PIN_21
    #define GPIO_PIN_21 GpioAction::NONE
#endif
#ifndef GPIO_PIN_22
    #define GPIO_PIN_22 GpioAction::NONE
#endif
#ifndef GPIO_PIN_23
    #define GPIO_PIN_23 GpioAction::NONE
#endif
#ifndef GPIO_PIN_24
    #define GPIO_PIN_24 GpioAction::NONE
#endif
#ifndef GPIO_PIN_25
    #define GPIO_PIN_25 GpioAction::NONE
#endif
#ifndef GPIO_PIN_26
    #define GPIO_PIN_26 GpioAction::NONE
#endif
#ifndef GPIO_PIN_27
    #define GPIO_PIN_27 GpioAction::NONE
#endif
#ifndef GPIO_PIN_28
    #define GPIO_PIN_28 GpioAction::NONE
#endif
#ifndef GPIO_PIN_29
    #define GPIO_PIN_29 GpioAction::NONE
#endif

// -----------------------------------------------------
// Migration leftovers
// -----------------------------------------------------

#ifndef EXTRA_BUTTON_PIN
    #define EXTRA_BUTTON_PIN -1
#endif
#ifndef EXTRA_BUTTON_MASK
    #define EXTRA_BUTTON_MASK 0
#endif

void ConfigUtils::initUnsetPropertiesWithDefaults(Config& config)
{
    const uint8_t emptyByteArray[0] = {};

    INIT_UNSET_PROPERTY_STR(config, boardVersion, GP2040VERSION);
    INIT_UNSET_PROPERTY_STR(config, boardConfig, GP2040_BOARDCONFIG);

    // gamepadOptions
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputMode, DEFAULT_INPUT_MODE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, dpadMode, DEFAULT_DPAD_MODE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, socdMode, DEFAULT_SOCD_MODE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, invertXAxis, false);
    INIT_UNSET_PROPERTY(config.gamepadOptions, switchTpShareForDs4, false);
    INIT_UNSET_PROPERTY(config.gamepadOptions, lockHotkeys, DEFAULT_LOCK_HOTKEYS);
    INIT_UNSET_PROPERTY(config.gamepadOptions, fourWayMode, false);
    INIT_UNSET_PROPERTY(config.gamepadOptions, profileNumber, 1);
    INIT_UNSET_PROPERTY(config.gamepadOptions, ps4ControllerType, DEFAULT_PS4CONTROLLER_TYPE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, debounceDelay, DEFAULT_DEBOUNCE_DELAY);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeB1, DEFAULT_INPUT_MODE_B1);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeB2, DEFAULT_INPUT_MODE_B2);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeB3, DEFAULT_INPUT_MODE_B3);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeB4, DEFAULT_INPUT_MODE_B4);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeL1, DEFAULT_INPUT_MODE_L1);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeL2, DEFAULT_INPUT_MODE_L2);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeR1, DEFAULT_INPUT_MODE_R1);
    INIT_UNSET_PROPERTY(config.gamepadOptions, inputModeR2, DEFAULT_INPUT_MODE_R2);
    INIT_UNSET_PROPERTY(config.gamepadOptions, ps4AuthType, DEFAULT_PS4AUTHENTICATION_TYPE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, ps5AuthType, DEFAULT_PS5AUTHENTICATION_TYPE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, xinputAuthType, DEFAULT_XINPUTAUTHENTICATION_TYPE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, ps4ControllerIDMode, DEFAULT_PS4_ID_MODE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, usbDescOverride, DEFAULT_USB_DESC_OVERRIDE);
    INIT_UNSET_PROPERTY_STR(config.gamepadOptions, usbDescProduct, DEFAULT_USB_DESC_PRODUCT);
    INIT_UNSET_PROPERTY_STR(config.gamepadOptions, usbDescManufacturer, DEFAULT_USB_DESC_MANUFACTURER);
    INIT_UNSET_PROPERTY_STR(config.gamepadOptions, usbDescVersion, DEFAULT_USB_DESC_VERSION);
    INIT_UNSET_PROPERTY(config.gamepadOptions, usbOverrideID, DEFAULT_USB_ID_OVERRIDE);
    INIT_UNSET_PROPERTY(config.gamepadOptions, usbVendorID, DEFAULT_USB_VENDOR_ID);
    INIT_UNSET_PROPERTY(config.gamepadOptions, usbProductID, DEFAULT_USB_PRODUCT_ID);
    INIT_UNSET_PROPERTY(config.gamepadOptions, miniMenuGamepadInput, MINI_MENU_GAMEPAD_INPUT);

    // hotkeyOptions
    HotkeyOptions& hotkeyOptions = config.hotkeyOptions;
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey01, auxMask, HOTKEY_01_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey01, buttonsMask, HOTKEY_01_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey01, dpadMask, HOTKEY_01_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey01, action, GamepadHotkey(HOTKEY_01_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey02, auxMask, HOTKEY_02_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey02, buttonsMask, HOTKEY_02_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey02, dpadMask, HOTKEY_02_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey02, action, GamepadHotkey(HOTKEY_02_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey03, auxMask, HOTKEY_03_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey03, buttonsMask, HOTKEY_03_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey03, dpadMask, HOTKEY_03_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey03, action, GamepadHotkey(HOTKEY_03_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey04, auxMask, HOTKEY_04_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey04, buttonsMask, HOTKEY_04_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey04, dpadMask, HOTKEY_04_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey04, action, GamepadHotkey(HOTKEY_04_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey05, auxMask, HOTKEY_05_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey05, buttonsMask, HOTKEY_05_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey05, dpadMask, HOTKEY_05_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey05, action, GamepadHotkey(HOTKEY_05_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey06, auxMask, HOTKEY_06_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey06, buttonsMask, HOTKEY_06_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey06, dpadMask, HOTKEY_06_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey06, action, GamepadHotkey(HOTKEY_06_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey07, auxMask, HOTKEY_07_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey07, buttonsMask, HOTKEY_07_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey07, dpadMask, HOTKEY_07_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey07, action, GamepadHotkey(HOTKEY_07_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey08, auxMask, HOTKEY_08_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey08, buttonsMask, HOTKEY_08_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey08, dpadMask, HOTKEY_08_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey08, action, GamepadHotkey(HOTKEY_08_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey09, auxMask, HOTKEY_09_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey09, buttonsMask, HOTKEY_09_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey09, dpadMask, HOTKEY_09_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey09, action, GamepadHotkey(HOTKEY_09_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey10, auxMask, HOTKEY_10_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey10, buttonsMask, HOTKEY_10_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey10, dpadMask, HOTKEY_10_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey10, action, GamepadHotkey(HOTKEY_10_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey11, auxMask, HOTKEY_11_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey11, buttonsMask, HOTKEY_11_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey11, dpadMask, HOTKEY_11_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey11, action, GamepadHotkey(HOTKEY_11_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey12, auxMask, HOTKEY_12_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey12, buttonsMask, HOTKEY_12_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey12, dpadMask, HOTKEY_12_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey12, action, GamepadHotkey(HOTKEY_12_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey13, auxMask, HOTKEY_13_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey13, buttonsMask, HOTKEY_13_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey13, dpadMask, HOTKEY_13_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey13, action, GamepadHotkey(HOTKEY_13_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey14, auxMask, HOTKEY_14_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey14, buttonsMask, HOTKEY_14_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey14, dpadMask, HOTKEY_14_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey14, action, GamepadHotkey(HOTKEY_14_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey15, auxMask, HOTKEY_15_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey15, buttonsMask, HOTKEY_15_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey15, dpadMask, HOTKEY_15_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey15, action, GamepadHotkey(HOTKEY_15_ACTION));
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey16, auxMask, HOTKEY_16_AUX_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey16, buttonsMask, HOTKEY_16_BUTTONS_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey16, dpadMask, HOTKEY_16_DPAD_MASK);
    INIT_UNSET_PROPERTY(hotkeyOptions.hotkey16, action, GamepadHotkey(HOTKEY_16_ACTION));

    // forcedSetupMode
    INIT_UNSET_PROPERTY(config.forcedSetupOptions, mode, DEFAULT_FORCED_SETUP_MODE);

    // keyboardMapping
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyDpadUp, KEY_DPAD_UP);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyDpadDown, KEY_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyDpadRight, KEY_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyDpadLeft, KEY_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonB1, KEY_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonB2, KEY_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonR2, KEY_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonL2, KEY_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonB3, KEY_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonB4, KEY_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonR1, KEY_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonL1, KEY_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonS1, KEY_BUTTON_S1);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonS2, KEY_BUTTON_S2);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonL3, KEY_BUTTON_L3);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonR3, KEY_BUTTON_R3);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonA1, KEY_BUTTON_A1);
    INIT_UNSET_PROPERTY(config.keyboardMapping, keyButtonA2, KEY_BUTTON_A2);

    // displayOptions
    INIT_UNSET_PROPERTY(config.displayOptions, enabled, !!HAS_I2C_DISPLAY);
    INIT_UNSET_PROPERTY(config.displayOptions, deprecatedI2cBlock, (DISPLAY_I2C_BLOCK == i2c0) ? 0 : 1);
    INIT_UNSET_PROPERTY(config.displayOptions, deprecatedI2cSDAPin, -1);
    INIT_UNSET_PROPERTY(config.displayOptions, deprecatedI2cSCLPin, -1);
    INIT_UNSET_PROPERTY(config.displayOptions, deprecatedI2cAddress, DISPLAY_I2C_ADDR);
    INIT_UNSET_PROPERTY(config.displayOptions, deprecatedI2cSpeed, I2C_SPEED);
    INIT_UNSET_PROPERTY(config.displayOptions, buttonLayout, BUTTON_LAYOUT);
    INIT_UNSET_PROPERTY(config.displayOptions, buttonLayoutRight, BUTTON_LAYOUT_RIGHT);
    INIT_UNSET_PROPERTY(config.displayOptions, turnOffWhenSuspended, DISPLAY_TURN_OFF_WHEN_SUSPENDED);
    INIT_UNSET_PROPERTY(config.displayOptions, inputMode, 1);
    INIT_UNSET_PROPERTY(config.displayOptions, turboMode, 1);
    INIT_UNSET_PROPERTY(config.displayOptions, dpadMode, 1);
    INIT_UNSET_PROPERTY(config.displayOptions, socdMode, 1);
    INIT_UNSET_PROPERTY(config.displayOptions, macroMode, 1);
    INIT_UNSET_PROPERTY(config.displayOptions, profileMode, 0);
    INIT_UNSET_PROPERTY(config.displayOptions, inputHistoryEnabled, !!INPUT_HISTORY_ENABLED);
    INIT_UNSET_PROPERTY(config.displayOptions, inputHistoryLength, INPUT_HISTORY_LENGTH);
    INIT_UNSET_PROPERTY(config.displayOptions, inputHistoryCol, INPUT_HISTORY_COL);
    INIT_UNSET_PROPERTY(config.displayOptions, inputHistoryRow, INPUT_HISTORY_ROW);

    ButtonLayoutParamsLeft& paramsLeft = config.displayOptions.buttonLayoutCustomOptions.paramsLeft;
    INIT_UNSET_PROPERTY(paramsLeft, layout, BUTTON_LAYOUT);
    INIT_UNSET_PROPERTY(paramsLeft.common, startX, 8);
    INIT_UNSET_PROPERTY(paramsLeft.common, startY, 28);
    INIT_UNSET_PROPERTY(paramsLeft.common, buttonRadius, 8);
    INIT_UNSET_PROPERTY(paramsLeft.common, buttonPadding, 2);

    ButtonLayoutParamsRight& paramsRight = config.displayOptions.buttonLayoutCustomOptions.paramsRight;
    INIT_UNSET_PROPERTY(paramsRight, layout, BUTTON_LAYOUT_RIGHT);
    INIT_UNSET_PROPERTY(paramsRight.common, startX, 8);
    INIT_UNSET_PROPERTY(paramsRight.common, startY, 28);
    INIT_UNSET_PROPERTY(paramsRight.common, buttonRadius, 8);
    INIT_UNSET_PROPERTY(paramsRight.common, buttonPadding, 2);

    INIT_UNSET_PROPERTY(config.displayOptions, splashMode, SPLASH_MODE);
    INIT_UNSET_PROPERTY(config.displayOptions, splashChoice, SPLASH_CHOICE);
    INIT_UNSET_PROPERTY(config.displayOptions, splashDuration, SPLASH_DURATION);
	const unsigned char defaultSplash[] = { DEFAULT_SPLASH };
    INIT_UNSET_PROPERTY_BYTES(config.displayOptions, splashImage, defaultSplash);
    INIT_UNSET_PROPERTY(config.displayOptions, size, DISPLAY_SIZE);
    INIT_UNSET_PROPERTY(config.displayOptions, flip, DISPLAY_FLIP);
    INIT_UNSET_PROPERTY(config.displayOptions, invert, !!DISPLAY_INVERT);
    INIT_UNSET_PROPERTY(config.displayOptions, displaySaverTimeout, DISPLAY_SAVER_TIMEOUT);
    INIT_UNSET_PROPERTY(config.displayOptions, displaySaverMode, DISPLAY_SAVER_MODE);
    INIT_UNSET_PROPERTY(config.displayOptions, buttonLayoutOrientation, DISPLAY_LAYOUT_ORIENTATION);

    // peripheralOptions
    PeripheralOptions& peripheralOptions = config.peripheralOptions;
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C0, enabled, (!!I2C0_ENABLED));
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C0, sda, (!!I2C0_ENABLED) ? I2C0_PIN_SDA : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C0, scl, (!!I2C0_ENABLED) ? I2C0_PIN_SCL : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C0, speed, I2C0_SPEED);
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C1, enabled, (!!I2C1_ENABLED));
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C1, sda, (!!I2C1_ENABLED) ? I2C1_PIN_SDA : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C1, scl, (!!I2C1_ENABLED) ? I2C1_PIN_SCL : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockI2C1, speed, I2C1_SPEED);

    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI0, enabled, (!!SPI0_ENABLED));
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI0, rx, (!!SPI0_ENABLED) ?  SPI0_PIN_RX : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI0, cs, (!!SPI0_ENABLED) ?  SPI0_PIN_CS : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI0, sck, (!!SPI0_ENABLED) ? SPI0_PIN_SCK : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI0, tx, (!!SPI0_ENABLED) ?  SPI0_PIN_TX : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI1, enabled, (!!SPI1_ENABLED));
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI1, rx, (!!SPI1_ENABLED) ?  SPI1_PIN_RX : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI1, cs, (!!SPI1_ENABLED) ?  SPI1_PIN_CS : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI1, sck, (!!SPI1_ENABLED) ? SPI1_PIN_SCK : -1);
    INIT_UNSET_PROPERTY(peripheralOptions.blockSPI1, tx, (!!SPI1_ENABLED) ?  SPI1_PIN_TX : -1);

    INIT_UNSET_PROPERTY(peripheralOptions.blockUSB0, enabled, USB_PERIPHERAL_ENABLED);
    INIT_UNSET_PROPERTY(peripheralOptions.blockUSB0, dp, USB_PERIPHERAL_PIN_DPLUS);
    INIT_UNSET_PROPERTY(peripheralOptions.blockUSB0, order, USB_PERIPHERAL_PIN_ORDER);
    INIT_UNSET_PROPERTY(peripheralOptions.blockUSB0, enable5v, USB_PERIPHERAL_PIN_5V);

    // ledOptions
    INIT_UNSET_PROPERTY(config.ledOptions, dataPin, BOARD_LEDS_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, ledFormat, static_cast<LEDFormat_Proto>(LED_FORMAT));
    INIT_UNSET_PROPERTY(config.ledOptions, ledLayout, BUTTON_LAYOUT);
    INIT_UNSET_PROPERTY(config.ledOptions, ledsPerButton, LEDS_PER_PIXEL);
    INIT_UNSET_PROPERTY(config.ledOptions, brightnessMaximum, LED_BRIGHTNESS_MAXIMUM);
    INIT_UNSET_PROPERTY(config.ledOptions, brightnessSteps, LED_BRIGHTNESS_STEPS);
    INIT_UNSET_PROPERTY(config.ledOptions, turnOffWhenSuspended, LEDS_TURN_OFF_WHEN_SUSPENDED);

    INIT_UNSET_PROPERTY(config.ledOptions, indexUp, LEDS_DPAD_UP);
    INIT_UNSET_PROPERTY(config.ledOptions, indexDown, LEDS_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.ledOptions, indexLeft, LEDS_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.ledOptions, indexRight, LEDS_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.ledOptions, indexB1, LEDS_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.ledOptions, indexB2, LEDS_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.ledOptions, indexB3, LEDS_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.ledOptions, indexB4, LEDS_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.ledOptions, indexL1, LEDS_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.ledOptions, indexR1, LEDS_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.ledOptions, indexL2, LEDS_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.ledOptions, indexR2, LEDS_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.ledOptions, indexS1, LEDS_BUTTON_S1);
    INIT_UNSET_PROPERTY(config.ledOptions, indexS2, LEDS_BUTTON_S2);
    INIT_UNSET_PROPERTY(config.ledOptions, indexL3, LEDS_BUTTON_L3);
    INIT_UNSET_PROPERTY(config.ledOptions, indexR3, LEDS_BUTTON_R3);
    INIT_UNSET_PROPERTY(config.ledOptions, indexA1, LEDS_BUTTON_A1);
    INIT_UNSET_PROPERTY(config.ledOptions, indexA2, LEDS_BUTTON_A2);

    INIT_UNSET_PROPERTY(config.ledOptions, pledType, PLED_TYPE);
    INIT_UNSET_PROPERTY(config.ledOptions, pledPin1, PLED1_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledPin2, PLED2_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledPin3, PLED3_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledPin4, PLED4_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledColor, static_cast<uint32_t>(PLED_COLOR.r) << 16 | static_cast<uint32_t>(PLED_COLOR.g) << 8 | static_cast<uint32_t>(PLED_COLOR.b));
    // hacky, but previous versions used PLED1_PIN for either PWM GPIO pins or RGB indexes
    // so we're just going to copy the defined values into both locations and have the migration
    // to pin mappings sort it out
    INIT_UNSET_PROPERTY(config.ledOptions, pledIndex1, PLED1_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledIndex2, PLED2_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledIndex3, PLED3_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, pledIndex4, PLED4_PIN);

    INIT_UNSET_PROPERTY(config.ledOptions, caseRGBType, CASE_RGB_TYPE);
    INIT_UNSET_PROPERTY(config.ledOptions, caseRGBIndex, CASE_RGB_INDEX);
    INIT_UNSET_PROPERTY(config.ledOptions, caseRGBCount, CASE_RGB_COUNT);

    // animationOptions
    INIT_UNSET_PROPERTY(config.animationOptions, baseAnimationIndex, LEDS_BASE_ANIMATION_INDEX);
    INIT_UNSET_PROPERTY(config.animationOptions, brightness, LEDS_BRIGHTNESS);
    INIT_UNSET_PROPERTY(config.animationOptions, staticColorIndex, LEDS_STATIC_COLOR_INDEX);
    INIT_UNSET_PROPERTY(config.animationOptions, buttonColorIndex, LEDS_BUTTON_COLOR_INDEX);
    INIT_UNSET_PROPERTY(config.animationOptions, chaseCycleTime, LEDS_CHASE_CYCLE_TIME);
    INIT_UNSET_PROPERTY(config.animationOptions, rainbowCycleTime, LEDS_RAINBOW_CYCLE_TIME);
    INIT_UNSET_PROPERTY(config.animationOptions, themeIndex, LEDS_THEME_INDEX);
    INIT_UNSET_PROPERTY(config.animationOptions, hasCustomTheme, false);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeUp, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeDown, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeLeft, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeRight, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB1, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB2, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB3, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB4, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL1, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR1, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL2, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR2, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeS1, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeS2, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL3, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR3, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeA1, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeA2, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeUpPressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeDownPressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeLeftPressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeRightPressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB1Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB2Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB3Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeB4Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL1Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR1Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL2Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR2Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeS1Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeS2Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeL3Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeR3Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeA1Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, customThemeA2Pressed, 0);
    INIT_UNSET_PROPERTY(config.animationOptions, buttonPressColorCooldownTimeInMs, LEDS_PRESS_COLOR_COOLDOWN_TIME);
    INIT_UNSET_PROPERTY(config.animationOptions, ambientLightEffectsCountIndex, AMBIENT_LIGHT_EFFECT);
    INIT_UNSET_PROPERTY(config.animationOptions, alStaticColorBrightnessCustomX, AMBIENT_STATIC_COLOR_BRIGHTNESS);
    INIT_UNSET_PROPERTY(config.animationOptions, alGradientBrightnessCustomX, AMBIENT_GRADIENT_COLOR_BRIGHTNESS);
    INIT_UNSET_PROPERTY(config.animationOptions, alChaseBrightnessCustomX, AMBIENT_CHASE_COLOR_BRIGHTNESS);
    INIT_UNSET_PROPERTY(config.animationOptions, alStaticBrightnessCustomThemeX, AMBIENT_CUSTOM_THEME_BRIGHTNESS);
    INIT_UNSET_PROPERTY(config.animationOptions, ambientLightGradientSpeed, AMBIENT_GRADIENT_SPEED);
    INIT_UNSET_PROPERTY(config.animationOptions, ambientLightChaseSpeed, AMBIENT_CHASE_SPEED);
    INIT_UNSET_PROPERTY(config.animationOptions, ambientLightBreathSpeed, AMBIENT_BREATH_SPEED);
    INIT_UNSET_PROPERTY(config.animationOptions, alCustomStaticThemeIndex, AMBIENT_CUSTOM_THEME);
    INIT_UNSET_PROPERTY(config.animationOptions, alCustomStaticColorIndex, AMBIENT_STATIC_COLOR);

    // addonOptions.bootselButtonOptions
    INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, enabled, !!BOOTSEL_BUTTON_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, buttonMap, BOOTSEL_BUTTON_MASK);

    // addonOptions.onBoardLedOptions
    INIT_UNSET_PROPERTY(config.addonOptions.onBoardLedOptions, enabled, !!BOARD_LED_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.onBoardLedOptions, mode, BOARD_LED_TYPE);

    // addonOptions.analogOptions
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, enabled, !!ANALOG_INPUT_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc1PinX, ANALOG_ADC_1_VRX);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc1PinY, ANALOG_ADC_1_VRY);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc1Mode, ANALOG_ADC_1_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc1Invert, ANALOG_ADC_1_INVERT);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc2PinX, ANALOG_ADC_2_VRX);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc2PinY, ANALOG_ADC_2_VRY);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc2Mode, ANALOG_ADC_2_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdc2Invert, ANALOG_ADC_2_INVERT);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, forced_circularity, !!FORCED_CIRCULARITY_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, inner_deadzone, DEFAULT_INNER_DEADZONE);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, outer_deadzone, DEFAULT_OUTER_DEADZONE);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, auto_calibrate, !!AUTO_CALIBRATE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analog_smoothing, !!ANALOG_SMOOTHING_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, smoothing_factor, !!SMOOTHING_FACTOR);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analog_error, ANALOG_ERROR);

    // addonOptions.turboOptions
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, enabled, !!TURBO_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, deprecatedButtonPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, ledPin, TURBO_LED_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shotCount, DEFAULT_SHOT_PER_SEC);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupDialPin, PIN_SHMUP_DIAL);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupModeEnabled, !!TURBO_SHMUP_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn1, SHMUP_ALWAYS_ON1);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn2, SHMUP_ALWAYS_ON2);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn3, SHMUP_ALWAYS_ON3);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn4, SHMUP_ALWAYS_ON4);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn1Pin, PIN_SHMUP_BUTTON1);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn2Pin, PIN_SHMUP_BUTTON2);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn3Pin, PIN_SHMUP_BUTTON3);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn4Pin, PIN_SHMUP_BUTTON4);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask1, SHMUP_BUTTON1);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask2, SHMUP_BUTTON2);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask3, SHMUP_BUTTON3);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask4, SHMUP_BUTTON4);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupMixMode, SHMUP_MIX_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, turboLedType, TURBO_LED_TYPE);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, turboLedIndex, TURBO_LED_INDEX);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, turboLedColor, static_cast<uint32_t>(TURBO_LED_COLOR.r) << 16 | static_cast<uint32_t>(TURBO_LED_COLOR.g) << 8 | static_cast<uint32_t>(TURBO_LED_COLOR.b));

    // addonOptions.reverseOptions
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, enabled, !!REVERSE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, buttonPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, ledPin, REVERSE_LED_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionUp, REVERSE_UP_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionDown, REVERSE_DOWN_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionLeft, REVERSE_LEFT_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionRight, REVERSE_RIGHT_DEFAULT);

    // addonOptions.socdSliderOptions
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, enabled, !!SLIDER_SOCD_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, deprecatedPinOne, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, deprecatedPinTwo, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, modeDefault, SLIDER_SOCD_SLOT_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, deprecatedModeOne, SLIDER_SOCD_SLOT_ONE);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, deprecatedModeTwo, SLIDER_SOCD_SLOT_TWO);

    // addonOptions.analogADS1219Options
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, enabled, !!I2C_ANALOG1219_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, deprecatedI2cBlock, (I2C_ANALOG1219_BLOCK == i2c0) ? 0 : 1)
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, deprecatedI2cSDAPin, -1);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, deprecatedI2cSCLPin, -1);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, deprecatedI2cAddress, I2C_ANALOG1219_ADDRESS);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, deprecatedI2cSpeed, I2C_ANALOG1219_SPEED);

    // addonOptions.analogADS1256Options
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, enabled, !!SPI_ANALOG1256_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, spiBlock, (SPI_ANALOG1256_BLOCK == spi0) ? 0 : 1)
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, csPin, SPI_ANALOG1256_CS_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, drdyPin, SPI_ANALOG1256_DRDY_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, avdd, ADS1256_MAX_3V);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1256Options, enableTriggers, false);

    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, enabled, !!DUAL_DIRECTIONAL_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, deprecatedUpPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, deprecatedDownPin, (Pin_t)-1)
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, deprecatedLeftPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, deprecatedRightPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, dpadMode, static_cast<DpadMode>(DUAL_DIRECTIONAL_STICK_MODE));
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, combineMode, DualDirectionalCombinationMode::MIXED_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, fourWayMode, false);

	// addonOptions.tiltOptions
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, enabled, !!TILT_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tilt1Pin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt1LeftX, TILT1_FACTOR_LEFT_X);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt1LeftY, TILT1_FACTOR_LEFT_Y);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt1RightX, TILT1_FACTOR_RIGHT_X);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt1RightY, TILT1_FACTOR_RIGHT_Y);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tilt2Pin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt2LeftX, TILT2_FACTOR_LEFT_X);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt2LeftY, TILT2_FACTOR_LEFT_Y);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt2RightX, TILT2_FACTOR_RIGHT_X);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, factorTilt2RightY, TILT2_FACTOR_RIGHT_Y);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogDownPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogUpPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogLeftPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogRightPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogDownPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogUpPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogLeftPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogRightPin, (Pin_t)-1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltSOCDMode, TILT_SOCD_MODE);

    // addonOptions.buzzerOptions
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, enabled, !!BUZZER_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, pin, BUZZER_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, volume, BUZZER_VOLUME);
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, enablePin, BUZZER_ENABLE_PIN);

    // addonOptions.ps4Options
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, serial, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, signature, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaN, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaE, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaD, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaP, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaQ, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaDP, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaDQ, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaQP, emptyByteArray);
    INIT_UNSET_PROPERTY_BYTES(config.addonOptions.ps4Options, rsaRN, emptyByteArray);

    // addonOptions.wiiOptions
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, enabled, WII_EXTENSION_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, deprecatedI2cBlock, (WII_EXTENSION_I2C_BLOCK == i2c0) ? 0 : 1);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, deprecatedI2cSDAPin, -1);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, deprecatedI2cSCLPin, -1);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, deprecatedI2cSpeed, WII_EXTENSION_I2C_SPEED);

    // addonOptions.snesOptions
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, enabled, !!SNES_PAD_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, clockPin, SNES_PAD_CLOCK_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, latchPin, SNES_PAD_LATCH_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, dataPin, SNES_PAD_DATA_PIN);

    // addonOptions.pcf8575Options
    INIT_UNSET_PROPERTY(config.addonOptions.pcf8575Options, enabled, I2C_PCF8575_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.pcf8575Options, deprecatedI2cBlock, (I2C_PCF8575_BLOCK == i2c0) ? 0 : 1);

    GpioAction pcf8575Actions[PCF8575_PIN_COUNT] = {
        PCF8575_PIN00_ACTION,PCF8575_PIN01_ACTION,PCF8575_PIN02_ACTION,PCF8575_PIN03_ACTION,
        PCF8575_PIN04_ACTION,PCF8575_PIN05_ACTION,PCF8575_PIN06_ACTION,PCF8575_PIN07_ACTION,
        PCF8575_PIN08_ACTION,PCF8575_PIN09_ACTION,PCF8575_PIN10_ACTION,PCF8575_PIN11_ACTION,
        PCF8575_PIN12_ACTION,PCF8575_PIN13_ACTION,PCF8575_PIN14_ACTION,PCF8575_PIN15_ACTION
    };

    GpioDirection pcf8575Directions[PCF8575_PIN_COUNT] = {
        PCF8575_PIN00_DIRECTION,PCF8575_PIN01_DIRECTION,PCF8575_PIN02_DIRECTION,PCF8575_PIN03_DIRECTION,
        PCF8575_PIN04_DIRECTION,PCF8575_PIN05_DIRECTION,PCF8575_PIN06_DIRECTION,PCF8575_PIN07_DIRECTION,
        PCF8575_PIN08_DIRECTION,PCF8575_PIN09_DIRECTION,PCF8575_PIN10_DIRECTION,PCF8575_PIN11_DIRECTION,
        PCF8575_PIN12_DIRECTION,PCF8575_PIN13_DIRECTION,PCF8575_PIN14_DIRECTION,PCF8575_PIN15_DIRECTION
    };

    for (uint16_t pin = 0; pin < PCF8575_PIN_COUNT; pin++) {
        INIT_UNSET_PROPERTY(config.addonOptions.pcf8575Options.pins[pin], action, pcf8575Actions[pin]);
        INIT_UNSET_PROPERTY(config.addonOptions.pcf8575Options.pins[pin], direction, pcf8575Directions[pin]);
    }
    // reminder that this must be set or else nanopb won't retain anything
    config.addonOptions.pcf8575Options.pins_count = PCF8575_PIN_COUNT;

    // addonOptions.drv8833RumbleOptions
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, enabled, !!DRV8833_RUMBLE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, leftMotorPin, DRV8833_RUMBLE_LEFT_MOTOR_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, rightMotorPin, DRV8833_RUMBLE_RIGHT_MOTOR_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, motorSleepPin, DRV8833_RUMBLE_MOTOR_SLEEP_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, pwmFrequency, DRV8833_RUMBLE_PWM_FREQUENCY);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, dutyMin, DRV8833_RUMBLE_DUTY_MIN);
    INIT_UNSET_PROPERTY(config.addonOptions.drv8833RumbleOptions, dutyMax, DRV8833_RUMBLE_DUTY_MAX);

    // addonOptions.rotaryOptions
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions, enabled, !!ROTARY_ENCODER_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, enabled, !!ENCODER_ONE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, pinA, (Pin_t)ENCODER_ONE_PIN_A);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, pinB, (Pin_t)ENCODER_ONE_PIN_B);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, mode, ENCODER_ONE_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, pulsesPerRevolution, ENCODER_ONE_PPR);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, resetAfter, ENCODER_ONE_RESET);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, allowWrapAround, ENCODER_ONE_WRAP);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderOne, multiplier, ENCODER_ONE_MULTIPLIER);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, enabled, !!ENCODER_TWO_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, pinA, (Pin_t)ENCODER_TWO_PIN_A);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, pinB, (Pin_t)ENCODER_TWO_PIN_B);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, mode, ENCODER_TWO_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, pulsesPerRevolution, ENCODER_TWO_PPR);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, resetAfter, ENCODER_TWO_RESET);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, allowWrapAround, ENCODER_TWO_WRAP);
    INIT_UNSET_PROPERTY(config.addonOptions.rotaryOptions.encoderTwo, multiplier, ENCODER_TWO_MULTIPLIER);

    // addonOptions.reactiveLEDOptions
    INIT_UNSET_PROPERTY(config.addonOptions.reactiveLEDOptions, enabled, !!REACTIVE_LED_ENABLED);
    for (uint16_t led = 0; led < REACTIVE_LED_COUNT; led++) {
        INIT_UNSET_PROPERTY(config.addonOptions.reactiveLEDOptions.leds[led], pin, -1);
        INIT_UNSET_PROPERTY(config.addonOptions.reactiveLEDOptions.leds[led], action, GpioAction::NONE);
        INIT_UNSET_PROPERTY(config.addonOptions.reactiveLEDOptions.leds[led], modeDown, REACTIVE_LED_STATIC_ON);
        INIT_UNSET_PROPERTY(config.addonOptions.reactiveLEDOptions.leds[led], modeUp, REACTIVE_LED_STATIC_OFF);
    }
    // reminder that this must be set or else nanopb won't retain anything
    config.addonOptions.reactiveLEDOptions.leds_count = REACTIVE_LED_COUNT;

    // keyboardMapping
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, enabled, KEYBOARD_HOST_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, deprecatedPinDplus, KEYBOARD_HOST_PIN_DPLUS);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, deprecatedPin5V, KEYBOARD_HOST_PIN_5V);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyDpadUp, KEY_DPAD_UP);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyDpadDown, KEY_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyDpadRight, KEY_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyDpadLeft, KEY_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonB1, KEY_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonB2, KEY_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonR2, KEY_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonL2, KEY_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonB3, KEY_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonB4, KEY_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonR1, KEY_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonL1, KEY_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonS1, KEY_BUTTON_S1);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonS2, KEY_BUTTON_S2);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonL3, KEY_BUTTON_L3);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonR3, KEY_BUTTON_R3);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonA1, KEY_BUTTON_A1);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions.mapping, keyButtonA2, KEY_BUTTON_A2);

    // addonOptions.focusModeOptions
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, enabled, !!FOCUS_MODE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, pin, -1);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, buttonLockMask, FOCUS_MODE_BUTTON_MASK);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, buttonLockEnabled, !!FOCUS_MODE_BUTTON_LOCK_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, macroLockEnabled, !!FOCUS_MODE_MACRO_LOCK_ENABLED);


    // addonOptions.gamepadUSBHostOptions
    INIT_UNSET_PROPERTY(config.addonOptions.gamepadUSBHostOptions, enabled, GAMEPAD_USB_HOST_ENABLED)

    // Macro options (always on)
    INIT_UNSET_PROPERTY(config.addonOptions.macroOptions, enabled, true);
    INIT_UNSET_PROPERTY(config.addonOptions.macroOptions, macroBoardLedEnabled, INPUT_MACRO_BOARD_LED_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.macroOptions, deprecatedPin, -1);

    // Set all macros
    for(int i = 0; i < MAX_MACRO_LIMIT; i++) {
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], enabled, 0);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], exclusive, 1);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], interruptible, 1);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], showFrames, 1);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], macroType, MacroType::ON_PRESS);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], useMacroTriggerButton, 0);
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], macroTriggerButton, 0);
        INIT_UNSET_PROPERTY_STR(config.addonOptions.macroOptions.macroList[i], macroLabel, "");
        INIT_UNSET_PROPERTY(config.addonOptions.macroOptions.macroList[i], deprecatedMacroTriggerPin, -1);
    }
}


// -----------------------------------------------------
// migrations
// used for when we might need to populate configs with
// something *other than* the board defaults
// -----------------------------------------------------

// convert core and addon pin mappings to GPIO mapping config
// NOTE: this also handles initializations for a blank config! if/when the deprecated
// pin mappings go away, the remainder of this code should go in there (there was no point
// in duplicating it right now)
void gpioMappingsMigrationCore(Config& config)
{
    PinMappings& deprecatedPinMappings = config.deprecatedPinMappings;
    ExtraButtonOptions& extraButtonOptions = config.addonOptions.deprecatedExtraButtonOptions;
    DualDirectionalOptions& ddiOptions = config.addonOptions.dualDirectionalOptions;
    SliderOptions& jsSliderOptions = config.addonOptions.deprecatedSliderOptions;
    SOCDSliderOptions& socdSliderOptions = config.addonOptions.socdSliderOptions;
    PeripheralOptions& peripheralOptions = config.peripheralOptions;
    KeyboardHostOptions& keyboardHostOptions = config.addonOptions.keyboardHostOptions;
    PSPassthroughOptions& psPassthroughOptions = config.addonOptions.psPassthroughOptions;
    TurboOptions& turboOptions = config.addonOptions.turboOptions;
    TiltOptions& tiltOptions = config.addonOptions.tiltOptions;
    FocusModeOptions& focusModeOptions = config.addonOptions.focusModeOptions;
    ReverseOptions& reverseOptions = config.addonOptions.reverseOptions;

    const auto gamepadMaskToGpioAction = [&](Mask_t gpMask) -> GpioAction
    {
        switch (gpMask)
        {
            case GAMEPAD_MASK_DU:
                return GpioAction::BUTTON_PRESS_UP;
            case GAMEPAD_MASK_DD:
                return GpioAction::BUTTON_PRESS_DOWN;
            case GAMEPAD_MASK_DL:
                return GpioAction::BUTTON_PRESS_LEFT;
            case GAMEPAD_MASK_DR:
                return GpioAction::BUTTON_PRESS_RIGHT;
            case GAMEPAD_MASK_B1:
                return GpioAction::BUTTON_PRESS_B1;
            case GAMEPAD_MASK_B2:
                return GpioAction::BUTTON_PRESS_B2;
            case GAMEPAD_MASK_B3:
                return GpioAction::BUTTON_PRESS_B3;
            case GAMEPAD_MASK_B4:
                return GpioAction::BUTTON_PRESS_B4;
            case GAMEPAD_MASK_L1:
                return GpioAction::BUTTON_PRESS_L1;
            case GAMEPAD_MASK_R1:
                return GpioAction::BUTTON_PRESS_R1;
            case GAMEPAD_MASK_L2:
                return GpioAction::BUTTON_PRESS_L2;
            case GAMEPAD_MASK_R2:
                return GpioAction::BUTTON_PRESS_R2;
            case GAMEPAD_MASK_S1:
                return GpioAction::BUTTON_PRESS_S1;
            case GAMEPAD_MASK_S2:
                return GpioAction::BUTTON_PRESS_S2;
            case GAMEPAD_MASK_L3:
                return GpioAction::BUTTON_PRESS_L3;
            case GAMEPAD_MASK_R3:
                return GpioAction::BUTTON_PRESS_R3;
            case GAMEPAD_MASK_A1:
                return GpioAction::BUTTON_PRESS_A1;
            case GAMEPAD_MASK_A2:
                return GpioAction::BUTTON_PRESS_A2;
            case AUX_MASK_FUNCTION:
                return GpioAction::BUTTON_PRESS_FN;
            default:
                return GpioAction::NONE;
        }
    };

    // create an array of the old
    GpioAction actions[NUM_BANK0_GPIOS] = {GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE,
                                           GpioAction::NONE, GpioAction::NONE, GpioAction::NONE};

    // flag additional pins as being used by an addon not managed here
    const auto markAddonPinIfUsed = [&](Pin_t gpPin) -> void {
        if (isValidPin(gpPin))
            actions[gpPin] = GpioAction::ASSIGNED_TO_ADDON;
    };

    // From Protobuf or Board Config
    const auto fromProtoBuf = [&](bool isInProtobuf, Pin_t *protobufEntry, GpioAction action) -> void {
        // get the core config value for a pin either from protobuf or, failing that, BoardConfig.h
        if (isInProtobuf) {
            if (*protobufEntry >= 0 && *protobufEntry < 30) {
                actions[*protobufEntry] = action;
                *protobufEntry = -1;
            }
        }
    };

    const auto fromBoardConfig = [&](Pin_t pinAssign, GpioAction action) -> void {
        if (actions[pinAssign] == GpioAction::NONE && isValidPin(pinAssign)) {
            actions[pinAssign] = action;
        }
    };

    fromProtoBuf(deprecatedPinMappings.has_pinDpadUp,    &deprecatedPinMappings.pinDpadUp,    GpioAction::BUTTON_PRESS_UP);
    fromProtoBuf(deprecatedPinMappings.has_pinDpadDown,  &deprecatedPinMappings.pinDpadDown,  GpioAction::BUTTON_PRESS_DOWN);
    fromProtoBuf(deprecatedPinMappings.has_pinDpadLeft,  &deprecatedPinMappings.pinDpadLeft,  GpioAction::BUTTON_PRESS_LEFT);
    fromProtoBuf(deprecatedPinMappings.has_pinDpadRight, &deprecatedPinMappings.pinDpadRight, GpioAction::BUTTON_PRESS_RIGHT);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonB1,  &deprecatedPinMappings.pinButtonB1,  GpioAction::BUTTON_PRESS_B1);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonB2,  &deprecatedPinMappings.pinButtonB2,  GpioAction::BUTTON_PRESS_B2);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonB3,  &deprecatedPinMappings.pinButtonB3,  GpioAction::BUTTON_PRESS_B3);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonB4,  &deprecatedPinMappings.pinButtonB4,  GpioAction::BUTTON_PRESS_B4);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonL1,  &deprecatedPinMappings.pinButtonL1,  GpioAction::BUTTON_PRESS_L1);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonR1,  &deprecatedPinMappings.pinButtonR1,  GpioAction::BUTTON_PRESS_R1);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonL2,  &deprecatedPinMappings.pinButtonL2,  GpioAction::BUTTON_PRESS_L2);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonR2,  &deprecatedPinMappings.pinButtonR2,  GpioAction::BUTTON_PRESS_R2);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonS1,  &deprecatedPinMappings.pinButtonS1,  GpioAction::BUTTON_PRESS_S1);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonS2,  &deprecatedPinMappings.pinButtonS2,  GpioAction::BUTTON_PRESS_S2);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonL3,  &deprecatedPinMappings.pinButtonL3,  GpioAction::BUTTON_PRESS_L3);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonR3,  &deprecatedPinMappings.pinButtonR3,  GpioAction::BUTTON_PRESS_R3);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonA1,  &deprecatedPinMappings.pinButtonA1,  GpioAction::BUTTON_PRESS_A1);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonA2,  &deprecatedPinMappings.pinButtonA2,  GpioAction::BUTTON_PRESS_A2);
    fromProtoBuf(deprecatedPinMappings.has_pinButtonFn,  &deprecatedPinMappings.pinButtonFn,  GpioAction::BUTTON_PRESS_FN);

    // convert extra pin mapping to GPIO mapping config
    if (extraButtonOptions.enabled && isValidPin(extraButtonOptions.pin)) {
        // previous config had a value we haven't migrated yet, it can/should apply in the new config
        actions[extraButtonOptions.pin] = gamepadMaskToGpioAction(extraButtonOptions.buttonMap);
        extraButtonOptions.pin = -1;
        extraButtonOptions.enabled = false;
    }

    // convert DDI direction pin mapping to GPIO mapping config
    if (ddiOptions.enabled) {
        fromProtoBuf(ddiOptions.has_deprecatedUpPin,    &ddiOptions.deprecatedUpPin, GpioAction::BUTTON_PRESS_DDI_UP);
        fromProtoBuf(ddiOptions.has_deprecatedDownPin,  &ddiOptions.deprecatedDownPin, GpioAction::BUTTON_PRESS_DDI_DOWN);
        fromProtoBuf(ddiOptions.has_deprecatedLeftPin,  &ddiOptions.deprecatedLeftPin, GpioAction::BUTTON_PRESS_DDI_LEFT);
        fromProtoBuf(ddiOptions.has_deprecatedRightPin, &ddiOptions.deprecatedRightPin, GpioAction::BUTTON_PRESS_DDI_RIGHT);
    }

    // convert JS slider pin mappings to GPIO mapping config
    if (jsSliderOptions.enabled && isValidPin(jsSliderOptions.deprecatedPinSliderOne)) {
        switch (jsSliderOptions.deprecatedModeOne) {
            case DpadMode::DPAD_MODE_DIGITAL: {
                actions[jsSliderOptions.deprecatedPinSliderOne] = GpioAction::SUSTAIN_DP_MODE_DP; break;
            }
            case DpadMode::DPAD_MODE_LEFT_ANALOG: {
                actions[jsSliderOptions.deprecatedPinSliderOne] = GpioAction::SUSTAIN_DP_MODE_LS; break;
            }
            case DpadMode::DPAD_MODE_RIGHT_ANALOG: {
                actions[jsSliderOptions.deprecatedPinSliderOne] = GpioAction::SUSTAIN_DP_MODE_RS; break;
            }
            default: break;
        }
        jsSliderOptions.deprecatedPinSliderOne = -1;
    }
    if (jsSliderOptions.enabled && isValidPin(jsSliderOptions.deprecatedPinSliderTwo)) {
        switch (jsSliderOptions.deprecatedModeTwo) {
            case DpadMode::DPAD_MODE_DIGITAL: {
                actions[jsSliderOptions.deprecatedPinSliderTwo] = GpioAction::SUSTAIN_DP_MODE_DP; break;
            }
            case DpadMode::DPAD_MODE_LEFT_ANALOG: {
                actions[jsSliderOptions.deprecatedPinSliderTwo] = GpioAction::SUSTAIN_DP_MODE_LS; break;
            }
            case DpadMode::DPAD_MODE_RIGHT_ANALOG: {
                actions[jsSliderOptions.deprecatedPinSliderTwo] = GpioAction::SUSTAIN_DP_MODE_RS; break;
            }
            default: break;
        }
        jsSliderOptions.deprecatedPinSliderTwo = -1;
    }
    // convert SOCD slider pin mappings to GPIO mapping config
    if (socdSliderOptions.enabled && isValidPin(socdSliderOptions.deprecatedPinOne)) {
        switch (socdSliderOptions.deprecatedModeOne) {
            case SOCDMode::SOCD_MODE_UP_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinOne] = GpioAction::SUSTAIN_SOCD_MODE_UP_PRIO; break;
            }
            case SOCDMode::SOCD_MODE_NEUTRAL: {
                actions[socdSliderOptions.deprecatedPinOne] = GpioAction::SUSTAIN_SOCD_MODE_NEUTRAL; break;
            }
            case SOCDMode::SOCD_MODE_SECOND_INPUT_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinOne] = GpioAction::SUSTAIN_SOCD_MODE_SECOND_WIN; break;
            }
            case SOCDMode::SOCD_MODE_FIRST_INPUT_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinOne] = GpioAction::SUSTAIN_SOCD_MODE_FIRST_WIN; break;
            }
            case SOCDMode::SOCD_MODE_BYPASS: {
                actions[socdSliderOptions.deprecatedPinOne] = GpioAction::SUSTAIN_SOCD_MODE_BYPASS; break;
            }
            default: break;
        }
        socdSliderOptions.deprecatedPinOne = -1;
    }

    if (socdSliderOptions.enabled && isValidPin(socdSliderOptions.deprecatedPinTwo)) {
        switch (socdSliderOptions.deprecatedModeTwo) {
            case SOCDMode::SOCD_MODE_UP_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinTwo] = GpioAction::SUSTAIN_SOCD_MODE_UP_PRIO; break;
            }
            case SOCDMode::SOCD_MODE_NEUTRAL: {
                actions[socdSliderOptions.deprecatedPinTwo] = GpioAction::SUSTAIN_SOCD_MODE_NEUTRAL; break;
            }
            case SOCDMode::SOCD_MODE_SECOND_INPUT_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinTwo] = GpioAction::SUSTAIN_SOCD_MODE_SECOND_WIN; break;
            }
            case SOCDMode::SOCD_MODE_FIRST_INPUT_PRIORITY: {
                actions[socdSliderOptions.deprecatedPinTwo] = GpioAction::SUSTAIN_SOCD_MODE_FIRST_WIN; break;
            }
            case SOCDMode::SOCD_MODE_BYPASS: {
                actions[socdSliderOptions.deprecatedPinTwo] = GpioAction::SUSTAIN_SOCD_MODE_BYPASS; break;
            }
            default: break;
        }
        socdSliderOptions.deprecatedPinTwo = -1;
    }

    if (focusModeOptions.enabled) {
        fromProtoBuf(focusModeOptions.has_pin, &focusModeOptions.pin, GpioAction::SUSTAIN_FOCUS_MODE);
    }

    if (reverseOptions.enabled) {
        fromProtoBuf(reverseOptions.has_buttonPin, &reverseOptions.buttonPin, GpioAction::BUTTON_PRESS_INPUT_REVERSE);
    }

    // verify that tilt factors are not set to -1
    if (tiltOptions.enabled) {
        if (tiltOptions.factorTilt1LeftX == -1) tiltOptions.factorTilt1LeftX = TILT1_FACTOR_LEFT_X;
        if (tiltOptions.factorTilt1LeftY == -1) tiltOptions.factorTilt1LeftY = TILT1_FACTOR_LEFT_Y;
        if (tiltOptions.factorTilt1RightX == -1) tiltOptions.factorTilt1RightX = TILT1_FACTOR_RIGHT_X;
        if (tiltOptions.factorTilt1RightY == -1) tiltOptions.factorTilt1RightY = TILT1_FACTOR_RIGHT_Y;
        if (tiltOptions.factorTilt2LeftX == -1) tiltOptions.factorTilt2LeftX = TILT2_FACTOR_LEFT_X;
        if (tiltOptions.factorTilt2LeftY == -1) tiltOptions.factorTilt2LeftY = TILT2_FACTOR_LEFT_Y;
        if (tiltOptions.factorTilt2RightX == -1) tiltOptions.factorTilt2RightX = TILT2_FACTOR_RIGHT_X;
        if (tiltOptions.factorTilt2RightY == -1) tiltOptions.factorTilt2RightY = TILT2_FACTOR_RIGHT_Y;

        fromProtoBuf(tiltOptions.has_tilt1Pin, &tiltOptions.tilt1Pin, GpioAction::ANALOG_DIRECTION_MOD_LOW);
        fromProtoBuf(tiltOptions.has_tilt2Pin, &tiltOptions.tilt2Pin, GpioAction::ANALOG_DIRECTION_MOD_HIGH);
        fromProtoBuf(tiltOptions.has_tiltLeftAnalogUpPin, &tiltOptions.tiltLeftAnalogUpPin, GpioAction::ANALOG_DIRECTION_LS_Y_NEG);
        fromProtoBuf(tiltOptions.has_tiltLeftAnalogDownPin, &tiltOptions.tiltLeftAnalogDownPin, GpioAction::ANALOG_DIRECTION_LS_Y_POS);
        fromProtoBuf(tiltOptions.has_tiltLeftAnalogLeftPin, &tiltOptions.tiltLeftAnalogLeftPin, GpioAction::ANALOG_DIRECTION_LS_X_NEG);
        fromProtoBuf(tiltOptions.has_tiltLeftAnalogRightPin, &tiltOptions.tiltLeftAnalogRightPin, GpioAction::ANALOG_DIRECTION_LS_X_POS);
        fromProtoBuf(tiltOptions.has_tiltRightAnalogUpPin, &tiltOptions.tiltRightAnalogUpPin, GpioAction::ANALOG_DIRECTION_RS_Y_NEG);
        fromProtoBuf(tiltOptions.has_tiltRightAnalogDownPin, &tiltOptions.tiltRightAnalogDownPin, GpioAction::ANALOG_DIRECTION_RS_Y_POS);
        fromProtoBuf(tiltOptions.has_tiltRightAnalogLeftPin, &tiltOptions.tiltRightAnalogLeftPin, GpioAction::ANALOG_DIRECTION_RS_X_NEG);
        fromProtoBuf(tiltOptions.has_tiltRightAnalogRightPin, &tiltOptions.tiltRightAnalogRightPin, GpioAction::ANALOG_DIRECTION_RS_X_POS);
    }

    // Assign all potential board config pins
    GpioAction boardConfig[NUM_BANK0_GPIOS] = {GPIO_PIN_00, GPIO_PIN_01, GPIO_PIN_02,
											   GPIO_PIN_03, GPIO_PIN_04, GPIO_PIN_05,
                                               GPIO_PIN_06, GPIO_PIN_07, GPIO_PIN_08,
                                               GPIO_PIN_09, GPIO_PIN_10, GPIO_PIN_11,
                                               GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14,
                                               GPIO_PIN_15, GPIO_PIN_16, GPIO_PIN_17,
                                               GPIO_PIN_18, GPIO_PIN_19, GPIO_PIN_20,
                                               GPIO_PIN_21, GPIO_PIN_22, GPIO_PIN_23,
                                               GPIO_PIN_24, GPIO_PIN_25, GPIO_PIN_26,
                                               GPIO_PIN_27, GPIO_PIN_28, GPIO_PIN_29};

    // If we didn't import from protobuf, import from boardconfig
    for(unsigned int i = 0; i < NUM_BANK0_GPIOS; i++) {
        fromBoardConfig(i, boardConfig[i]);
    }

    // migrate I2C addons to use peripheral manager
    if (!peripheralOptions.blockI2C0.enabled && (
            (config.displayOptions.enabled && (config.displayOptions.deprecatedI2cBlock == 0)) || 
            (config.addonOptions.analogADS1219Options.enabled && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 0)) || 
            (config.addonOptions.wiiOptions.enabled && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 0))
        )
    ) {
        peripheralOptions.blockI2C0.enabled = (
            (config.displayOptions.enabled && (config.displayOptions.deprecatedI2cBlock == 0)) | 
            (config.addonOptions.analogADS1219Options.enabled && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 0)) | 
            (config.addonOptions.wiiOptions.enabled && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 0)) | 
            (!!I2C0_ENABLED)
        );

        // pin configuration
        peripheralOptions.blockI2C0.sda = (
            isValidPin(config.displayOptions.deprecatedI2cSDAPin) && (config.displayOptions.deprecatedI2cBlock == 0) ? 
            config.displayOptions.deprecatedI2cSDAPin : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSDAPin) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 0) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSDAPin : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSDAPin) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 0) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSDAPin : 
                    I2C0_PIN_SDA
                )
            )
        );
        markAddonPinIfUsed(peripheralOptions.blockI2C0.sda);

        peripheralOptions.blockI2C0.scl = (
            isValidPin(config.displayOptions.deprecatedI2cSCLPin) && (config.displayOptions.deprecatedI2cBlock == 0) ? 
            config.displayOptions.deprecatedI2cSCLPin : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSCLPin) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 0) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSCLPin : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSCLPin) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 0) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSCLPin : 
                    I2C0_PIN_SCL
                )
            )
        );
        markAddonPinIfUsed(peripheralOptions.blockI2C0.scl);

        // option configuration
        peripheralOptions.blockI2C0.speed = (
            isValidPin(config.displayOptions.deprecatedI2cSpeed) && (config.displayOptions.deprecatedI2cBlock == 0) ? 
            config.displayOptions.deprecatedI2cSpeed : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSpeed) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 0) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSpeed : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSpeed) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 0) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSpeed : 
                    I2C0_SPEED
                )
            )
        );
    }

    if (!peripheralOptions.blockI2C1.enabled && (
            (config.displayOptions.enabled && (config.displayOptions.deprecatedI2cBlock == 1)) || 
            (config.addonOptions.analogADS1219Options.enabled && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 1)) || 
            (config.addonOptions.wiiOptions.enabled && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 1))
        )
    ) {
        peripheralOptions.blockI2C1.enabled = (
            (config.displayOptions.enabled && (config.displayOptions.deprecatedI2cBlock == 1)) | 
            (config.addonOptions.analogADS1219Options.enabled && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 1)) | 
            (config.addonOptions.wiiOptions.enabled && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 1)) | 
            (!!I2C1_ENABLED)
        );

        // pin configuration
        peripheralOptions.blockI2C1.sda = (
            isValidPin(config.displayOptions.deprecatedI2cSDAPin) && (config.displayOptions.deprecatedI2cBlock == 1) ? 
            config.displayOptions.deprecatedI2cSDAPin : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSDAPin) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 1) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSDAPin : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSDAPin) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 1) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSDAPin : 
                    I2C1_PIN_SDA
                )
            )
        );
        markAddonPinIfUsed(peripheralOptions.blockI2C1.sda);

        peripheralOptions.blockI2C1.scl = (
            isValidPin(config.displayOptions.deprecatedI2cSCLPin) && (config.displayOptions.deprecatedI2cBlock == 1) ? 
            config.displayOptions.deprecatedI2cSCLPin : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSCLPin) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 1) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSCLPin : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSCLPin) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 1) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSCLPin : 
                    I2C1_PIN_SCL
                )
            )
        );
        markAddonPinIfUsed(peripheralOptions.blockI2C1.scl);

        // option configuration
        peripheralOptions.blockI2C1.speed = (
            isValidPin(config.displayOptions.deprecatedI2cSpeed) && (config.displayOptions.deprecatedI2cBlock == 1) ? 
            config.displayOptions.deprecatedI2cSpeed : 
            (
                isValidPin(config.addonOptions.analogADS1219Options.deprecatedI2cSpeed) && (config.addonOptions.analogADS1219Options.deprecatedI2cBlock == 1) ? 
                config.addonOptions.analogADS1219Options.deprecatedI2cSpeed : 
                (
                    isValidPin(config.addonOptions.wiiOptions.deprecatedI2cSpeed) && (config.addonOptions.wiiOptions.deprecatedI2cBlock == 1) ? 
                    config.addonOptions.wiiOptions.deprecatedI2cSpeed : 
                    I2C1_SPEED
                )
            )
        );
    }

    // migrate USB addons to use peripheral manager
    if (!peripheralOptions.blockUSB0.enabled && (keyboardHostOptions.enabled || psPassthroughOptions.enabled)) {
        peripheralOptions.blockUSB0.enabled = keyboardHostOptions.enabled | psPassthroughOptions.enabled | false;

        if (peripheralOptions.blockUSB0.enabled) {
            peripheralOptions.blockUSB0.enable5v = (
                isValidPin(keyboardHostOptions.deprecatedPin5V) ?
                keyboardHostOptions.deprecatedPin5V :
                (
                    isValidPin(psPassthroughOptions.deprecatedPin5V) ?
                    psPassthroughOptions.deprecatedPin5V :
                    -1
                )
            );
            markAddonPinIfUsed(peripheralOptions.blockUSB0.enable5v);

            peripheralOptions.blockUSB0.dp = (
                isValidPin(keyboardHostOptions.deprecatedPinDplus) ?
                keyboardHostOptions.deprecatedPinDplus :
                (
                    isValidPin(psPassthroughOptions.deprecatedPinDplus) ?
                    psPassthroughOptions.deprecatedPinDplus :
                    -1
                )
            );
            markAddonPinIfUsed(peripheralOptions.blockUSB0.dp);
            if (isValidPin(peripheralOptions.blockUSB0.dp))
                actions[peripheralOptions.blockUSB0.dp+1] = GpioAction::ASSIGNED_TO_ADDON;
        }
    }

    markAddonPinIfUsed(config.ledOptions.dataPin);
    // check if PLED PINs are actually GPIOs or not
    // pledPin used to be used for RGB indexes, so we should only mark the GPIO
    // as assigned to addon if in PWM mode
    if (config.ledOptions.pledType == PLEDType::PLED_TYPE_PWM) {
        // fields are being used for PWM, so they are GPIOs; reserve them
        markAddonPinIfUsed(config.ledOptions.pledPin1);
        markAddonPinIfUsed(config.ledOptions.pledPin2);
        markAddonPinIfUsed(config.ledOptions.pledPin3);
        markAddonPinIfUsed(config.ledOptions.pledPin4);
    } else {
        // default init copied the values into the new fields, pledIndex1-4, so unset these
        config.ledOptions.pledPin1 = -1;
        config.ledOptions.pledPin2 = -1;
        config.ledOptions.pledPin3 = -1;
        config.ledOptions.pledPin4 = -1;
    }
    markAddonPinIfUsed(config.addonOptions.analogOptions.analogAdc1PinX);
    markAddonPinIfUsed(config.addonOptions.analogOptions.analogAdc1PinY);
    markAddonPinIfUsed(config.addonOptions.analogOptions.analogAdc2PinX);
    markAddonPinIfUsed(config.addonOptions.analogOptions.analogAdc2PinY);
    markAddonPinIfUsed(config.addonOptions.buzzerOptions.pin);
    markAddonPinIfUsed(config.addonOptions.buzzerOptions.enablePin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.ledPin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.shmupDialPin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.shmupBtn1Pin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.shmupBtn2Pin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.shmupBtn3Pin);
    markAddonPinIfUsed(config.addonOptions.turboOptions.shmupBtn4Pin);
    markAddonPinIfUsed(config.addonOptions.reverseOptions.ledPin);
    markAddonPinIfUsed(config.addonOptions.snesOptions.clockPin);
    markAddonPinIfUsed(config.addonOptions.snesOptions.latchPin);
    markAddonPinIfUsed(config.addonOptions.snesOptions.dataPin);

    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        config.gpioMappings.pins[pin].action = actions[pin];
    }
    // reminder that this must be set or else nanopb won't retain anything
    config.gpioMappings.pins_count = NUM_BANK0_GPIOS;

    config.migrations.gpioMappingsMigrated = true;
}

// if the user previously had the JS slider addon enabled, copy its default to the
// core gamepad setting, since the functionality is within the core now
void migrateJSliderToCore(Config& config)
{
    if (config.addonOptions.deprecatedSliderOptions.enabled) {
        config.gamepadOptions.dpadMode = config.addonOptions.deprecatedSliderOptions.deprecatedModeDefault;
        config.addonOptions.deprecatedSliderOptions.enabled = false;
    }
}

// populate the alternative gpio mapping sets, aka profiles, with
// the old values or whatever is in the core mappings
// NOTE: this also handles initializations for a blank config! if/when the deprecated
// pin mappings go away, the remainder of this code should go in there (there was no point
// in duplicating it right now)
void gpioMappingsMigrationProfiles(Config& config)
{
    AlternativePinMappings* deprecatedAlts = config.profileOptions.deprecatedAlternativePinMappings;

    const auto assignProfilePinIfUsed = [&](uint8_t profileNum, Pin_t profilePin, GpioAction action) -> void {
        if (isValidPin(profilePin)) {
            config.profileOptions.gpioMappingsSets[profileNum].pins[profilePin].action = action;
        }
    };

    for (uint8_t profileNum = 0; profileNum <= 2; profileNum++) {
        for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
            config.profileOptions.gpioMappingsSets[profileNum].pins[pin].action = config.gpioMappings.pins[pin].action;
        }
        // only check protobuf if profiles are defined
        if (profileNum < config.profileOptions.deprecatedAlternativePinMappings_count) {
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonB1,  GpioAction::BUTTON_PRESS_B1);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonB2,  GpioAction::BUTTON_PRESS_B2);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonB3,  GpioAction::BUTTON_PRESS_B3);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonB4,  GpioAction::BUTTON_PRESS_B4);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonL1,  GpioAction::BUTTON_PRESS_L1);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonR1,  GpioAction::BUTTON_PRESS_R1);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonL2,  GpioAction::BUTTON_PRESS_L2);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinButtonR2,  GpioAction::BUTTON_PRESS_R2);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinDpadUp,    GpioAction::BUTTON_PRESS_UP);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinDpadDown,  GpioAction::BUTTON_PRESS_DOWN);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinDpadLeft,  GpioAction::BUTTON_PRESS_LEFT);
            assignProfilePinIfUsed(profileNum, deprecatedAlts[profileNum].pinDpadRight, GpioAction::BUTTON_PRESS_RIGHT);
        }

        // reminder that this must be set or else nanopb won't retain anything
        config.profileOptions.gpioMappingsSets[profileNum].pins_count = NUM_BANK0_GPIOS;
    }
    // reminder that this must be set or else nanopb won't retain anything
    config.profileOptions.gpioMappingsSets_count = 3;

    config.migrations.buttonProfilesMigrated = true;
}

void migrateTurboPinToGpio(Config& config) {
    // Features converted here must set their previous deprecated pin/set value as well (pin = -1)
    TurboOptions & turboOptions = config.addonOptions.turboOptions;

    // Convert turbo pin mapping to GPIO mapping config
    if (turboOptions.enabled && isValidPin(turboOptions.deprecatedButtonPin)) {
        Pin_t pin = turboOptions.deprecatedButtonPin;
        // previous config had a value we haven't migrated yet, it can/should apply in the new config
        config.gpioMappings.pins[pin].action = GpioAction::BUTTON_PRESS_TURBO;
        for (uint8_t profileNum = 0; profileNum <= 2; profileNum++) {
            config.profileOptions.gpioMappingsSets[profileNum].pins[pin].action = GpioAction::BUTTON_PRESS_TURBO;
        }
        turboOptions.deprecatedButtonPin = -1; // set our turbo options to -1 for subsequent calls
    }
    
    // Make sure we set PWM mode if we are using led pin
    if ( turboOptions.turboLedType == PLED_TYPE_NONE && isValidPin(turboOptions.ledPin) ) {
        turboOptions.turboLedType = PLED_TYPE_PWM;
    }
}

void migrateAuthenticationMethods(Config& config) {
    // Auth migrations
    GamepadOptions & gamepadOptions = config.gamepadOptions;
    PS4Options & ps4Options = config.addonOptions.ps4Options;
    PSPassthroughOptions & psPassthroughOptions = config.addonOptions.psPassthroughOptions;
    XBOnePassthroughOptions & xbonePassthroughOptions = config.addonOptions.xbonePassthroughOptions;

    if ( ps4Options.enabled == true ) { // PS4-Mode "on", assume keys are loaded, do not change modes
        gamepadOptions.ps4AuthType = InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS;
        ps4Options.enabled = false; // disable PS4-Mode add-on permanently
    }

    if ( psPassthroughOptions.enabled == true ) { // PS5 add-on "on", USB pass through, update ps4->ps5 boot
        gamepadOptions.ps5AuthType = InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB;
        // If current mode is PS4, update to PS5
        if ( gamepadOptions.inputMode == INPUT_MODE_PS4 ) {
            gamepadOptions.inputMode = INPUT_MODE_PS5;
        }
        // Also update our boot mode from PS4 to PS5 if set
        int32_t * bootModes[8] = { &config.gamepadOptions.inputModeB1, &config.gamepadOptions.inputModeB2,
            &config.gamepadOptions.inputModeB3, &config.gamepadOptions.inputModeB4,
            &config.gamepadOptions.inputModeL1, &config.gamepadOptions.inputModeL2,
            &config.gamepadOptions.inputModeR1, &config.gamepadOptions.inputModeR2};
        for(int32_t i = 0; i < 8; i++ ) {
            if ( *bootModes[i] == INPUT_MODE_PS4 ) {
                *bootModes[i] = INPUT_MODE_PS5; // modify ps4 -> ps5
            }
        }
        psPassthroughOptions.enabled = false; // disable PS-Passthrough add-on permanently
    }

    if ( xbonePassthroughOptions.enabled == true ) { // Xbox One add-on "on", USB pass through is assumed
        xbonePassthroughOptions.enabled = false; // disable and go on our way
    }
}

// enable profiles that have real data in them (profile 1 is always enabled)
// note that profiles 2-4 are no longer populated with profile 1's data on a fresh
// config, and this is checking previous configs with non-copy mappings to enable them
void profileEnabledFlagsMigration(Config& config) {
    config.gpioMappings.enabled = true;
    for (uint8_t profileNum = 0; profileNum < config.profileOptions.gpioMappingsSets_count; profileNum++) {
        if (!config.profileOptions.gpioMappingsSets[profileNum].pins_count) {
            // uninitialized profile, skip it and leave it disabled
            config.profileOptions.gpioMappingsSets[profileNum].enabled = false;
            continue;
        }
        for (uint8_t pinNum = 0; pinNum < config.gpioMappings.pins_count; pinNum++) {
            // check each pin: if the alt. mapping pin is different than the base (profile 1)
            // mapping, enable the profile and check the next one
            if (config.gpioMappings.pins[pinNum].action !=
                        config.profileOptions.gpioMappingsSets[profileNum].pins[pinNum].action ||
                    config.gpioMappings.pins[pinNum].customButtonMask !=
                        config.profileOptions.gpioMappingsSets[profileNum].pins[pinNum].customButtonMask ||
                    config.gpioMappings.pins[pinNum].customDpadMask !=
                        config.profileOptions.gpioMappingsSets[profileNum].pins[pinNum].customDpadMask) {
                config.profileOptions.gpioMappingsSets[profileNum].enabled = true;
                break;
            }
        }
    }
    config.migrations.profileEnabledFlagsMigrated = true;
}

void migrateMacroPinsToGpio(Config& config) {
    // Convert Macro pin mapping to GPIO mapping configs
    MacroOptions & macroOptions = config.addonOptions.macroOptions;
    if (macroOptions.has_deprecatedPin && isValidPin(macroOptions.deprecatedPin) ) {
        Pin_t pin = macroOptions.deprecatedPin;
        config.gpioMappings.pins[pin].action = GpioAction::BUTTON_PRESS_MACRO;
        for (uint8_t profileNum = 0; profileNum <= 2; profileNum++) {
            config.profileOptions.gpioMappingsSets[profileNum].pins[pin].action = GpioAction::BUTTON_PRESS_MACRO;
        }
        macroOptions.deprecatedPin = -1; // set our turbo options to -1 for subsequent calls
        macroOptions.has_deprecatedPin = false;
    }

    if ( macroOptions.macroList_count == MAX_MACRO_LIMIT ) {
        const static GpioAction actionList[6] = { GpioAction::BUTTON_PRESS_MACRO_1, GpioAction::BUTTON_PRESS_MACRO_2,
                                                    GpioAction::BUTTON_PRESS_MACRO_3, GpioAction::BUTTON_PRESS_MACRO_4,
                                                    GpioAction::BUTTON_PRESS_MACRO_5, GpioAction::BUTTON_PRESS_MACRO_6 };
        for(int i = 0; i < MAX_MACRO_LIMIT; i++ ) {
            if ( macroOptions.macroList[i].has_deprecatedMacroTriggerPin &&
                    isValidPin(macroOptions.macroList[i].deprecatedMacroTriggerPin) ) {
                Pin_t pin = macroOptions.macroList[i].deprecatedMacroTriggerPin;
                config.gpioMappings.pins[pin].action = actionList[i];
                for (uint8_t profileNum = 0; profileNum <= 2; profileNum++) {
                    config.profileOptions.gpioMappingsSets[profileNum].pins[pin].action = actionList[i];
                }
                macroOptions.macroList[i].deprecatedMacroTriggerPin = -1; // set our turbo options to -1 for subsequent calls
                macroOptions.macroList[i].has_deprecatedMacroTriggerPin = false;
            }
        }
    }
}

// populate existing configurations' buttonsMask and auxMask to mirror behavior
// from the behavior before this code merged. totally new configs get their
// board defaults via initUnsetPropertiesWithDefaults
void hotkeysMigration(Config& config)
{
    HotkeyOptions& hotkeys = config.hotkeyOptions;

    // if dpadMask is defined and buttonsMask and auxMask aren't, then this
    // hotkey was saved at least once in the past when the button shortcut was
    // known as F1/F2, so they should be made to reflect the
    // previously-hardcoded values for those hotkeys

    // F1 == S1 | S2, no Fn
    if (hotkeys.hotkey01.has_dpadMask && (!hotkeys.hotkey01.has_auxMask || !hotkeys.hotkey01.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey01, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey01, buttonsMask, GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    }
    if (hotkeys.hotkey02.has_dpadMask && (!hotkeys.hotkey02.has_auxMask || !hotkeys.hotkey02.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey02, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey02, buttonsMask, GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    }
    if (hotkeys.hotkey03.has_dpadMask && (!hotkeys.hotkey03.has_auxMask || !hotkeys.hotkey03.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey03, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey03, buttonsMask, GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    }
    if (hotkeys.hotkey04.has_dpadMask && (!hotkeys.hotkey04.has_auxMask || !hotkeys.hotkey04.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey04, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey04, buttonsMask, GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2);
    }

    // F2 == S2 | A1, no Fn
    if (hotkeys.hotkey05.has_dpadMask && (!hotkeys.hotkey05.has_auxMask || !hotkeys.hotkey05.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey05, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey05, buttonsMask, GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1);
    }
    if (hotkeys.hotkey06.has_dpadMask && (!hotkeys.hotkey06.has_auxMask || !hotkeys.hotkey06.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey06, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey06, buttonsMask, GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1);
    }
    if (hotkeys.hotkey07.has_dpadMask && (!hotkeys.hotkey07.has_auxMask || !hotkeys.hotkey07.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey07, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey07, buttonsMask, GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1);
    }
    if (hotkeys.hotkey08.has_dpadMask && (!hotkeys.hotkey08.has_auxMask || !hotkeys.hotkey08.has_buttonsMask)) {
	INIT_UNSET_PROPERTY(hotkeys.hotkey08, auxMask, 0);
	INIT_UNSET_PROPERTY(hotkeys.hotkey08, buttonsMask, GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1);
    }

    config.migrations.hotkeysMigrated = true;
}

// -----------------------------------------------------
// Loading / Saving
// -----------------------------------------------------

// We put a ConfigFooter struct at the end of the flash area reserved for FlashPROM. It contains a magicvalue, the size
// of the serialized config data and a CRC of that data. This information allows us to both locate and verify the stored
// data. The serialized data is located directly before the footer:
//
//                       FlashPROM block
// ┌────────────────────────────┴─────────────────────────────┐
// ┌──────────────┬────────────────────────────────────┬──────┐
// │Unused memory │Protobuf data                       │Footer│
// └──────────────┴────────────────────────────────────┴──────┘
//
struct ConfigFooter
{
    uint32_t dataSize;
    uint32_t dataCrc;
    uint32_t magic;

    bool operator==(const ConfigFooter& other) const
    {
        return
            dataSize == other.dataSize &&
            dataCrc == other.dataCrc &&
            magic == other.magic;
    }
};

static const uint32_t FOOTER_MAGIC = 0xd2f1e365;

// Verify that the maximum size of the serialized Config object fits into the allocated flash block
#if defined(Config_size)
    static_assert(Config_size + sizeof(ConfigFooter) <= EEPROM_SIZE_BYTES, "Maximum size of Config exceeds the maximum size allocated for FlashPROM");
#else
    #error "Maximum size of Config cannot be determined statically, make sure that you do not use any dynamically sized arrays or strings"
#endif

static bool loadConfigInner(Config& config)
{
    config = Config Config_init_zero;

    const uint8_t* flashEnd = reinterpret_cast<const uint8_t*>(EEPROM_ADDRESS_START) + EEPROM_SIZE_BYTES;
    const ConfigFooter& footer = *reinterpret_cast<const ConfigFooter*>(flashEnd - sizeof(ConfigFooter));

    // Check for presence of magic value
    if (footer.magic != FOOTER_MAGIC)
    {
        return false;
    }

        // Check if dataSize exceeds the reserved space
    if (footer.dataSize + sizeof(ConfigFooter) > EEPROM_SIZE_BYTES)
    {
        return false;
    }

    const uint8_t* dataPtr = flashEnd - sizeof(ConfigFooter) - footer.dataSize;

    // Verify CRC32 hash
    if (CRC32::calculate(dataPtr, footer.dataSize) != footer.dataCrc)
    {
        return false;
    }

    // We are now sufficiently confident that the data is valid so we run the deserialization
    pb_istream_t inputStream = pb_istream_from_buffer(dataPtr, footer.dataSize);
    return pb_decode(&inputStream, Config_fields, &config);
}

void ConfigUtils::load(Config& config)
{
    // First try to load from Protobuf storage, if that fails fall back to legacy storage.
    const bool loaded = loadConfigInner(config) | fromLegacyStorage(config);

    if (!loaded)
    {
        // We could neither deserialize Protobuf config data nor legacy config data.
        // We are probably dealing with a new device and therefore initialize the config to default values.
        config = Config Config_init_default;
    }

    // run migrations
    if (!config.migrations.hotkeysMigrated)
        hotkeysMigration(config);

    // Make sure that fields that were not deserialized are properly initialized.
    // They were probably added with a newer version of the firmware.
    initUnsetPropertiesWithDefaults(config);

    // Run migrations that need to happen after initUnset...
    // ProtoBuf && Board Config settings are loaded here
    if (!config.migrations.gpioMappingsMigrated)
        gpioMappingsMigrationCore(config);

    // Run migration to enable or disable pre-existing profiles
    if (!config.migrations.profileEnabledFlagsMigrated)
        profileEnabledFlagsMigration(config);

    // following migrations are simple enough to not need a protobuf boolean to track
    // Migrate turbo into GpioMappings
    migrateTurboPinToGpio(config);
    // Migrate PS4/PS5/XBone authentication methods to new organization
    migrateAuthenticationMethods(config);
    // Macro pins to gpio
    migrateMacroPinsToGpio(config);
    // Migrate old JS slider add-on to core
    migrateJSliderToCore(config);

    // Update boardVersion, in case we migrated from an older version
    strncpy(config.boardVersion, GP2040VERSION, sizeof(config.boardVersion));
    config.boardVersion[sizeof(config.boardVersion) - 1] = '\0';
    config.has_boardVersion = true;

    // Save, to make sure we persist any performed migration steps
    save(config);
}

static void setHasFlags(const pb_msgdesc_t* fields, void* s)
{
    pb_field_iter_t iter;
    if (!pb_field_iter_begin(&iter, fields, s))
    {
        return;
    }

    do
    {
        // Not implemented for extension fields
        assert(PB_LTYPE(iter.type) != PB_LTYPE_EXTENSION);

        switch (PB_HTYPE(iter.type))
        {
            case PB_HTYPE_OPTIONAL:
            {
                *reinterpret_cast<bool*>(iter.pSize) = true;

                // Recurse into sub-messages
                if (PB_LTYPE(iter.type) == PB_LTYPE_SUBMESSAGE)
                {
                    assert(iter.submsg_desc);
                    assert(iter.pData);

                    setHasFlags(iter.submsg_desc, iter.pData);
                }
                break;
            }

            case PB_HTYPE_REPEATED:
            {
                // Recurse into sub-messages
                if (PB_LTYPE(iter.type) == PB_LTYPE_SUBMESSAGE)
                {
                    assert(iter.submsg_desc);
                    assert(iter.pData);
                    assert(iter.pSize);

                    const pb_size_t array_size = *reinterpret_cast<pb_size_t*>(iter.pSize);
                    pb_byte_t* item_ptr = reinterpret_cast<pb_byte_t*>(iter.pData);
                    for (pb_size_t index = 0; index < array_size; ++index)
                    {
                        setHasFlags(iter.submsg_desc, item_ptr);
                        item_ptr += iter.data_size;
                    }
                }
                break;
            }

            default:
                // We do not support any other htypes of fields
                assert(false);
                continue;
        }
    } while (pb_field_iter_next(&iter));
}

bool ConfigUtils::save(Config& config)
{
    // We only allow saves from core0. Saves from core1 have to be marshalled to core0.
    assert(get_core_num() == 0);
    if (get_core_num() != 0)
    {
        return false;
    }

    // Set all has_XXX flags to true, we want to save all fields.
    // If we didn't do this we would have to remember to set the has_XXX flag manually whenever we change a field from
    // its default value.
    setHasFlags(Config_fields, &config);

    // Encode the data directly into the cache of FlashPROM
    pb_ostream_t outputStream = pb_ostream_from_buffer(EEPROM.writeCache, EEPROM_SIZE_BYTES - sizeof(ConfigFooter));
    if (!pb_encode(&outputStream, Config_fields, &config))
    {
        return false;
    }

    // Create the new footer
    ConfigFooter newFooter;
    newFooter.dataSize = outputStream.bytes_written;
    newFooter.dataCrc = CRC32::calculate(EEPROM.writeCache, newFooter.dataSize);
    newFooter.magic = FOOTER_MAGIC;

    // The data has changed when the footer content has changed. Only then do we acutally need to save.
    const ConfigFooter& oldFooter = *reinterpret_cast<ConfigFooter*>(EEPROM.writeCache + EEPROM_SIZE_BYTES - sizeof(ConfigFooter));
    if (newFooter == oldFooter)
    {
        // The data has not changed, no saving neccessary.
        return true;
    }

    // Write the footer
    ConfigFooter* cacheFooter = reinterpret_cast<ConfigFooter*>(EEPROM.writeCache + EEPROM_SIZE_BYTES - sizeof(ConfigFooter));
    memcpy(cacheFooter, &newFooter, sizeof(ConfigFooter));

    // Move the encoded data in memory down to the footer
    memmove(EEPROM.writeCache + EEPROM_SIZE_BYTES - sizeof(ConfigFooter) - newFooter.dataSize, EEPROM.writeCache, newFooter.dataSize);
    memset(EEPROM.writeCache, 0, EEPROM_SIZE_BYTES - sizeof(ConfigFooter) - newFooter.dataSize);

    EEPROM.commit();

    return true;
}

// -----------------------------------------------------
// To JSON
// -----------------------------------------------------

static void writeIndentation(std::string& str, int level)
{
    str.append(static_cast<std::string::size_type>(level), '\t');
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) appendAsString(std::string& str, double value)
{
    str.append(std::to_string(value));
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) appendAsString(std::string& str, float value)
{
    str.append(std::to_string(value));
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) appendAsString(std::string& str, int32_t value)
{
    str.append(std::to_string(value));
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) appendAsString(std::string& str, uint32_t value)
{
    str.append(std::to_string(value));
}

#define TO_JSON_ENUM(fieldname, submessageType) appendAsString(str, static_cast<int32_t>(s.fieldname));
#define TO_JSON_UENUM(fieldname, submessageType) appendAsString(str, static_cast<uint32_t>(s.fieldname));
#define TO_JSON_DOUBLE(fieldname, submessageType) appendAsString(str, static_cast<double>(s.fieldname));
#define TO_JSON_FLOAT(fieldname, submessageType) appendAsString(str, static_cast<float>(s.fieldname));
#define TO_JSON_INT32(fieldname, submessageType) appendAsString(str, s.fieldname);
#define TO_JSON_UINT32(fieldname, submessageType) appendAsString(str, s.fieldname);
#define TO_JSON_BOOL(fieldname, submessageType) str.append((s.fieldname) ? "true" : "false");
#define TO_JSON_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname); str.push_back('"');
#define TO_JSON_BYTES(fieldname, submessageType) str.push_back('"'); str.append(Base64::Encode(reinterpret_cast<const char*>(s.fieldname.bytes), s.fieldname.size)); str.push_back('"');
#define TO_JSON_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname, indentLevel + 1);

#define TO_JSON_REPEATED_ENUM(fieldname, submessageType) appendAsString(str, static_cast<int32_t>(s.fieldname[i]));
#define TO_JSON_REPEATED_UENUM(fieldname, submessageType) appendAsString(str, static_cast<uint32_t>(s.fieldname[i]));
#define TO_JSON_REPEATED_DOUBLE(fieldname, submessageType) appendAsString(str, static_cast<double>(s.fieldname[i]));
#define TO_JSON_REPEATED_FLOAT(fieldname, submessageType) appendAsString(str, static_cast<float>(s.fieldname[i]));
#define TO_JSON_REPEATED_INT32(fieldname, submessageType) appendAsString(str, s.fieldname[i]);
#define TO_JSON_REPEATED_UINT32(fieldname, submessageType) appendAsString(str, s.fieldname[i]);
#define TO_JSON_REPEATED_BOOL(fieldname, submessageType) str.append((s.fieldname[i]) ? "true" : "false");
#define TO_JSON_REPEATED_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname[i]); str.push_back('"');
#define TO_JSON_REPEATED_BYTES(fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_REPEATED_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname[i], indentLevel + 1);

#define TO_JSON_REPEATED(ltype, fieldname, submessageType) \
    str.append("["); \
    for (int i = 0; i < s.PREPROCESSOR_JOIN(fieldname, _count); ++i) \
    { \
        if (i != 0) str.append(",");\
        str.append("\n"); \
        writeIndentation(str, indentLevel + 1); \
        PREPROCESSOR_JOIN(TO_JSON_REPEATED_, ltype)(fieldname, submessageType) \
    } \
    str.append("\n"); \
    writeIndentation(str, indentLevel); \
    str.append("]"); \

#define TO_JSON_REQUIRED(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_OPTIONAL(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_SINGULAR(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_FIXARRAY(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_ONEOF(ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_STATIC(htype, ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, htype)(ltype, fieldname, submessageType)
#define TO_JSON_POINTER(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_CALLBACK(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_FIELD(parenttype, atype, htype, ltype, fieldname, tag, disallow_export) \
    if (!disallow_export) \
    { \
        if (!firstField) str.append(",\n"); \
        firstField = false; \
        writeIndentation(str, indentLevel); \
        str.append("\"" #fieldname "\": "); \
        PREPROCESSOR_JOIN(TO_JSON_, atype)(htype, ltype, fieldname, parenttype ## _ ## fieldname ## _MSGTYPE) \
    }

#define GEN_TO_JSON_FUNCTION_DECL(structtype) static void toJSON ## structtype(std::string& str, const structtype& s, int indentLevel);

#define GEN_TO_JSON_FUNCTION(structtype) \
    static void toJSON ## structtype(std::string& str, const structtype& s, int indentLevel) \
    { \
        bool firstField = true; \
        str.append("{\n"); \
        structtype ## _FIELDLIST(TO_JSON_FIELD, structtype) \
        str.push_back('\n'); \
        writeIndentation(str, indentLevel - 1); \
        str.push_back('}'); \
    } \

#if defined(CONFIG_MESSAGES_GP2040)
    CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION_DECL)
    CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION)
#endif
#if defined(ENUM_MESSAGES_GP2040)
    ENUM_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION_DECL)
    ENUM_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION)
#endif

std::string ConfigUtils::toJSON(const Config& config)
{
    std::string str;
    str.reserve(1024 * 4);
    toJSONConfig(str, config, 1);
    str.push_back('\n');

    return str;
}

// -----------------------------------------------------
// From JSON
// -----------------------------------------------------

#define TEST_VALUE(name, value) if (v == value) return true;

#define GEN_IS_VALID_ENUM_VALUE_FUNCTION(enumtype) \
    static bool isValid ## enumtype(int v) \
    { \
        PREPROCESSOR_JOIN(enumtype, _VALUELIST)(TEST_VALUE) \
        return false; \
    }

#if defined(CONFIG_ENUMS_GP2040)
    CONFIG_ENUMS_GP2040(GEN_IS_VALID_ENUM_VALUE_FUNCTION)
#endif
#if defined(ENUMS_ENUMS_GP2040)
    ENUMS_ENUMS_GP2040(GEN_IS_VALID_ENUM_VALUE_FUNCTION)
#endif

#define FROM_JSON_ENUM(fieldname, enumType) \
    if (jsonObject.containsKey(#fieldname)) \
    { \
        JsonVariantConst value = jsonObject[#fieldname]; \
        if (value.is<int>()) \
        { \
            const int v = value.as<int>(); \
            if (PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(v)) \
            { \
                configStruct.fieldname = static_cast<decltype(configStruct.fieldname)>(v); \
                configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
            } \
            else \
            { \
                return false; \
            } \
        } \
        else \
        { \
            return false; \
        } \
    }

#define FROM_JSON_UENUM(fieldname, enumType) \
    if (jsonObject.containsKey(#fieldname)) \
    { \
        JsonVariantConst value = jsonObject[#fieldname]; \
        if (value.is<unsigned int>()) \
        { \
            const unsigned int v = value.as<unsigned int>(); \
            if (PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(v)) \
            { \
                configStruct.fieldname = static_cast<decltype(configStruct.fieldname)>(v); \
                configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
            } \
            else \
            { \
                return false; \
            } \
        } \
        else \
        { \
            return false; \
        } \
    }

static bool fromJsonDouble(JsonObjectConst jsonObject, const char* fieldname, double& value, bool& flag)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst jsonVariant = jsonObject[fieldname];
        if (jsonVariant.is<double>())
        {
            value = jsonVariant.as<double>();
            flag = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define FROM_JSON_DOUBLE(fieldname, submessageType) if (!fromJsonDouble(jsonObject, #fieldname, configStruct.fieldname, configStruct.PREPROCESSOR_JOIN(has_, fieldname))) { return false; }

static bool fromJsonFloat(JsonObjectConst jsonObject, const char* fieldname, float& value, bool& flag)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst jsonVariant = jsonObject[fieldname];
        if (jsonVariant.is<float>())
        {
            value = jsonVariant.as<float>();
            flag = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define FROM_JSON_FLOAT(fieldname, submessageType) if (!fromJsonFloat(jsonObject, #fieldname, configStruct.fieldname, configStruct.PREPROCESSOR_JOIN(has_, fieldname))) { return false; }

static bool fromJsonInt32(JsonObjectConst jsonObject, const char* fieldname, int32_t& value, bool& flag)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst jsonVariant = jsonObject[fieldname];
        if (jsonVariant.is<int>())
        {
            value = jsonVariant.as<int>();
            flag = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define FROM_JSON_INT32(fieldname, submessageType) if (!fromJsonInt32(jsonObject, #fieldname, configStruct.fieldname, configStruct.PREPROCESSOR_JOIN(has_, fieldname))) { return false; }

static bool fromJsonUint32(JsonObjectConst jsonObject, const char* fieldname, uint32_t& value, bool& flag)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst jsonVariant = jsonObject[fieldname];
        if (jsonVariant.is<unsigned int>())
        {
            value = jsonVariant.as<unsigned int>();
            flag = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define FROM_JSON_UINT32(fieldname, submessageType) if (!fromJsonUint32(jsonObject, #fieldname, configStruct.fieldname, configStruct.PREPROCESSOR_JOIN(has_, fieldname))) { return false; }

static bool fromJsonBool(JsonObjectConst jsonObject, const char* fieldname, bool& value, bool& flag)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst jsonVariant = jsonObject[fieldname];
        if (jsonVariant.is<bool>())
        {
            value = jsonVariant.as<bool>();
            flag = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define FROM_JSON_BOOL(fieldname, submessageType) if (!fromJsonBool(jsonObject, #fieldname, configStruct.fieldname, configStruct.PREPROCESSOR_JOIN(has_, fieldname))) { return false; }

#define FROM_JSON_STRING(fieldname, submessageType) \
    if (jsonObject.containsKey(#fieldname)) \
    { \
        JsonVariantConst value = jsonObject[#fieldname]; \
        if (value.is<const char*>() && strlen(value.as<const char*>()) < sizeof(configStruct.fieldname)) \
        { \
            strncpy(configStruct.fieldname, value.as<const char*>(), sizeof(configStruct.fieldname)); \
            configStruct.fieldname[sizeof(configStruct.fieldname) - 1] = '\0'; \
            configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
        } \
        else \
        { \
            return false; \
        } \
    }

static bool fromJsonBytes(JsonObjectConst jsonObject, const char* fieldname, uint8_t* bytes, uint16_t& size, size_t maxSize)
{
    if (jsonObject.containsKey(fieldname))
    {
        JsonVariantConst value = jsonObject[fieldname];
        if (!value.is<const char*>())
        {
            return false;
        }
        const char* str = value.as<const char*>();
        const size_t strLength = strlen(str);

        // Length of Base64 encoded data has to be divisible by 4
        if (strLength % 4 != 0)
        {
            return false;
        }

        size_t decodedLength = strLength / 4 * 3;
        if (strLength >= 1 && str[strLength - 1] == '=') --decodedLength;
        if (strLength >= 2 && str[strLength - 2] == '=') --decodedLength;
        if (decodedLength > maxSize)
        {
            return false;
        }

        std::string decoded;
        if (!Base64::Decode(str, strLength, decoded))
        {
            return false;
        }
        memcpy(bytes, decoded.data(), decoded.length());
        size = decoded.length();
    }

    return true;
}

#define FROM_JSON_BYTES(fieldname, submessageType) if (!fromJsonBytes(jsonObject, #fieldname, configStruct.fieldname.bytes, configStruct.fieldname.size, sizeof(configStruct.fieldname.bytes))) return false;

#define FROM_JSON_MESSAGE(fieldname, submessageType) \
    if (jsonObject.containsKey(#fieldname)) \
    { \
        JsonVariantConst value = jsonObject[#fieldname]; \
        if (!value.is<JsonObjectConst>() || !PREPROCESSOR_JOIN(fromJSON, PREPROCESSOR_JOIN(submessageType, _MSGTYPE))(value.as<JsonObjectConst>(), configStruct.fieldname)) \
        { \
            return false; \
        } \
    }

#define FROM_JSON_REPEATED_ENUM(fieldname, enumType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<int>() || !PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(array[index].as<int>())) \
        { \
            return false; \
        } \
        configStruct.fieldname[index] = static_cast<PREPROCESSOR_JOIN(enumType, _ENUMTYPE)>(array[index].as<int>()); \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_UENUM(fieldname, enumType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<unsigned int>() || !PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(array[index].as<unsigned int>())) \
        { \
            return false; \
        } \
        configStruct.fieldname[index] = static_cast<PREPROCESSOR_JOIN(enumType, _ENUMTYPE)>(array[index].as<unsigned int>()); \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_INT32(fieldname, submessageType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<int>()) \
        { \
            return false; \
        } \
        configStruct.fieldname[index] = array[index].as<int>(); \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_UINT32(fieldname, submessageType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<unsigned int>()) \
        { \
            return false; \
        } \
        configStruct.fieldname[index] = array[index].as<unsigned int>(); \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_BOOL(fieldname, submessageType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<bool>()) \
        { \
            return false; \
        } \
        configStruct.fieldname[index] = array[index].as<bool>(); \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_STRING(fieldname, submessageType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<const char*>() || strlen(array[index].as<const char*>()) >= sizeof(configStruct.fieldname[index])) \
        { \
            return false; \
        } \
        strncpy(configStruct.fieldname[index], array[index].as<const char*>(), sizeof(configStruct.fieldname[index])); \
        configStruct.fieldname[index][sizeof(configStruct.fieldname[index]) - 1] = '\0'; \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED_BYTES(fieldname, submessageType) static_assert(false, "not supported");

#define FROM_JSON_REPEATED_MESSAGE(fieldname, submessageType) \
    configStruct.fieldname ## _count = 0; \
    for (size_t index = 0; index < array.size(); ++index) \
    { \
        if (!array[index].is<JsonObjectConst>() || !PREPROCESSOR_JOIN(fromJSON, PREPROCESSOR_JOIN(submessageType, _MSGTYPE))(array[index].as<JsonObjectConst>(), configStruct.fieldname[index])) \
        { \
            return false; \
        } \
        ++configStruct.fieldname ## _count; \
    }

#define FROM_JSON_REPEATED(ltype, fieldname, submessageType) \
    if (jsonObject.containsKey(#fieldname)) \
    { \
        JsonVariantConst value = jsonObject[#fieldname]; \
        if (!value.is<JsonArrayConst>()) \
        { \
            return false; \
        } \
        JsonArrayConst array = value.as<JsonArrayConst>(); \
        if (array.size() > sizeof(configStruct.fieldname) / sizeof(configStruct.fieldname[0])) \
        { \
            return false; \
        } \
        PREPROCESSOR_JOIN(FROM_JSON_REPEATED_, ltype)(fieldname, submessageType) \
    }

#define FROM_JSON_REQUIRED(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, ltype)(fieldname, submessageType)
#define FROM_JSON_OPTIONAL(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, ltype)(fieldname, submessageType)
#define FROM_JSON_SINGULAR(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_FIXARRAY(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_ONEOF(ltype, fieldname, submessageType) static_assert(false, "not supported");

#define FROM_JSON_STATIC(htype, ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, htype)(ltype, fieldname, submessageType)
#define FROM_JSON_POINTER(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_CALLBACK(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");

#define FROM_JSON_FIELD(parenttype, atype, htype, ltype, fieldname, tag, disallow_export) \
    PREPROCESSOR_JOIN(FROM_JSON_, atype)(htype, ltype, fieldname, parenttype ## _ ## fieldname)

#define GEN_FROM_JSON_FUNCTION_DECL(structtype) static bool fromJSON ## structtype(JsonObjectConst jsonObject, structtype& configStruct);

#define GEN_FROM_JSON_FUNCTION(structtype) \
    static bool fromJSON ## structtype(JsonObjectConst jsonObject, structtype& configStruct) \
    { \
        structtype ## _FIELDLIST(FROM_JSON_FIELD, structtype) \
        return true; \
    }

#if defined(CONFIG_MESSAGES_GP2040)
    CONFIG_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION_DECL)
    CONFIG_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION)
#endif
#if defined(ENUM_MESSAGES_GP2040)
    ENUM_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION_DECL)
    ENUM_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION)
#endif

// Missing properties are ignored and initialized with default values
// Type mismatches, buffer overruns or illegal enum values cause an error
bool ConfigUtils::fromJSON(Config& config, const char* data, size_t dataLen)
{
    DynamicJsonDocument doc(1024 * 10);
    if (deserializeJson(doc, data, dataLen) != DeserializationError::Ok || !doc.is<JsonObject>())
    {
        return false;
    }

    // Store config struct on the heap to avoid stack overflow
    if (!fromJSONConfig(doc.as<JsonObjectConst>(), config))
    {
        return false;
    }

    initUnsetPropertiesWithDefaults(config);

    // we need to run migrations here too, in case the json document changed pins or things derived from pins
    gpioMappingsMigrationCore(config);
    migrateTurboPinToGpio(config);
    migrateAuthenticationMethods(config);
    migrateMacroPinsToGpio(config);

    return true;
}
