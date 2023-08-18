#include "config_utils.h"

#include "config.pb.h"
#include "enums.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"

#include "BoardConfig.h"
#include "GamepadConfig.h"
#include "helper.h"
#include "addons/analog.h"
#include "addons/board_led.h"
#include "addons/bootsel_button.h"
#include "addons/buzzerspeaker.h"
#include "addons/dualdirectional.h"
#include "addons/tilt.h"
#include "addons/extra_button.h"
#include "addons/focus_mode.h"
#include "addons/i2canalog1219.h"
#include "addons/i2cdisplay.h"
#include "addons/jslider.h"
#include "addons/keyboard_host.h"
#include "addons/neopicoleds.h"
#include "addons/playernum.h"
#include "addons/pleds.h"
#include "addons/ps4mode.h"
#include "addons/pspassthrough.h"
#include "addons/reverse.h"
#include "addons/slider_socd.h"
#include "addons/turbo.h"
#include "addons/wiiext.h"
#include "addons/snes_input.h"

#include "CRC32.h"
#include "FlashPROM.h"
#include "configs/base64.h"

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
#ifndef DEFAULT_DPAD_MODE
    #define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#endif
#ifndef DEFAULT_SOCD_MODE
    #define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#endif

void ConfigUtils::initUnsetPropertiesWithDefaults(Config& config)
{
    const uint8_t emptyByteArray[0] = {};

    INIT_UNSET_PROPERTY_STR(config, boardVersion, GP2040VERSION);

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

    // forcedSetupMode
    INIT_UNSET_PROPERTY(config.forcedSetupOptions, mode, DEFAULT_FORCED_SETUP_MODE);

    // pinMappings
    INIT_UNSET_PROPERTY(config.pinMappings, pinDpadUp, PIN_DPAD_UP);
    INIT_UNSET_PROPERTY(config.pinMappings, pinDpadDown, PIN_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.pinMappings, pinDpadLeft, PIN_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.pinMappings, pinDpadRight, PIN_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB1, PIN_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB2, PIN_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB3, PIN_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB4, PIN_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL1, PIN_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR1, PIN_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL2, PIN_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR2, PIN_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonS1, PIN_BUTTON_S1);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonS2, PIN_BUTTON_S2);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL3, PIN_BUTTON_L3);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR3, PIN_BUTTON_R3);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonA1, PIN_BUTTON_A1);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonA2, PIN_BUTTON_A2);
    INIT_UNSET_PROPERTY(config.pinMappings, pinButtonFn, PIN_BUTTON_FN);

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
    INIT_UNSET_PROPERTY(config.displayOptions, i2cBlock, (I2C_BLOCK == i2c0) ? 0 : 1);
    INIT_UNSET_PROPERTY(config.displayOptions, i2cSDAPin, I2C_SDA_PIN);
    INIT_UNSET_PROPERTY(config.displayOptions, i2cSCLPin, I2C_SCL_PIN);
    INIT_UNSET_PROPERTY(config.displayOptions, i2cAddress, DISPLAY_I2C_ADDR);
    INIT_UNSET_PROPERTY(config.displayOptions, i2cSpeed, I2C_SPEED);
    INIT_UNSET_PROPERTY(config.displayOptions, buttonLayout, BUTTON_LAYOUT);
    INIT_UNSET_PROPERTY(config.displayOptions, buttonLayoutRight, BUTTON_LAYOUT_RIGHT);

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

    // alternate pin mappings
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonB1, PIN_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonB2, PIN_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonB3, PIN_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonB4, PIN_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonL1, PIN_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonR1, PIN_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonL2, PIN_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinButtonR2, PIN_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinDpadUp, PIN_DPAD_UP);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinDpadDown, PIN_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinDpadLeft, PIN_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[0], pinDpadRight, PIN_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonB1, PIN_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonB2, PIN_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonB3, PIN_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonB4, PIN_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonL1, PIN_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonR1, PIN_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonL2, PIN_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinButtonR2, PIN_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinDpadUp, PIN_DPAD_UP);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinDpadDown, PIN_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinDpadLeft, PIN_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[1], pinDpadRight, PIN_DPAD_RIGHT);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonB1, PIN_BUTTON_B1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonB2, PIN_BUTTON_B2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonB3, PIN_BUTTON_B3);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonB4, PIN_BUTTON_B4);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonL1, PIN_BUTTON_L1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonR1, PIN_BUTTON_R1);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonL2, PIN_BUTTON_L2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinButtonR2, PIN_BUTTON_R2);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinDpadUp, PIN_DPAD_UP);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinDpadDown, PIN_DPAD_DOWN);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinDpadLeft, PIN_DPAD_LEFT);
    INIT_UNSET_PROPERTY(config.profileOptions.alternativePinMappings[2], pinDpadRight, PIN_DPAD_RIGHT);
    config.profileOptions.alternativePinMappings_count = 3;

    // ledOptions
    INIT_UNSET_PROPERTY(config.ledOptions, dataPin, BOARD_LEDS_PIN);
    INIT_UNSET_PROPERTY(config.ledOptions, ledFormat, static_cast<LEDFormat_Proto>(LED_FORMAT));
    INIT_UNSET_PROPERTY(config.ledOptions, ledLayout, BUTTON_LAYOUT);
    INIT_UNSET_PROPERTY(config.ledOptions, ledsPerButton, LEDS_PER_PIXEL);
    INIT_UNSET_PROPERTY(config.ledOptions, brightnessMaximum, LED_BRIGHTNESS_MAXIMUM);
    INIT_UNSET_PROPERTY(config.ledOptions, brightnessSteps, LED_BRIGHTNESS_STEPS);

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

    // addonOptions.bootselButtonOptions
    INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, enabled, !!BOOTSEL_BUTTON_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, buttonMap, BOOTSEL_BUTTON_MASK);

    // addonOptions.onBoardLedOptions
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
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analog_deadzone, DEFAULT_ANALOG_DEADZONE);
    INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, auto_calibrate, !!AUTO_CALIBRATE_ENABLED);

    // addonOptions.turboOptions
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, enabled, !!TURBO_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, buttonPin, PIN_BUTTON_TURBO);
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

    // addonOptions.sliderOptions
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, enabled, !!JSLIDER_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, pinSliderOne, PIN_SLIDER_ONE);
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, pinSliderTwo, PIN_SLIDER_TWO);
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, modeZero, SLIDER_MODE_ZERO);
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, modeOne, SLIDER_MODE_ONE);
    INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, modeTwo, SLIDER_MODE_TWO);

    // addonOptions.reverseOptions
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, enabled, !!REVERSE_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, buttonPin, PIN_REVERSE);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, ledPin, REVERSE_LED_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionUp, REVERSE_UP_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionDown, REVERSE_DOWN_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionLeft, REVERSE_LEFT_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionRight, REVERSE_RIGHT_DEFAULT);

    // addonOptions.socdSliderOptions
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, enabled, !!SLIDER_SOCD_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, pinOne, PIN_SLIDER_SOCD_ONE);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, pinTwo, PIN_SLIDER_SOCD_TWO);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, modeDefault, SLIDER_SOCD_SLOT_DEFAULT);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, modeOne, SLIDER_SOCD_SLOT_ONE);
    INIT_UNSET_PROPERTY(config.addonOptions.socdSliderOptions, modeTwo, SLIDER_SOCD_SLOT_TWO);

    // addonOptions.analogADS1219Options
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, enabled, !!I2C_ANALOG1219_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cBlock, (I2C_ANALOG1219_BLOCK == i2c0) ? 0 : 1)
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSDAPin, I2C_ANALOG1219_SDA_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSCLPin, I2C_ANALOG1219_SCL_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cAddress, I2C_ANALOG1219_ADDRESS);
    INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSpeed, I2C_ANALOG1219_SPEED);

    // addonOptions.dualDirectionalOptions
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, enabled, !!DUAL_DIRECTIONAL_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, upPin, PIN_DUAL_DIRECTIONAL_UP);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, downPin, PIN_DUAL_DIRECTIONAL_DOWN)
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, leftPin, PIN_DUAL_DIRECTIONAL_LEFT);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, rightPin, PIN_DUAL_DIRECTIONAL_RIGHT);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, dpadMode, static_cast<DpadMode>(DUAL_DIRECTIONAL_STICK_MODE));
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, combineMode, DUAL_DIRECTIONAL_COMBINE_MODE);
    INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, fourWayMode, false);

		// addonOptions.tiltOptions
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, enabled, !!TILT_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tilt1Pin, PIN_TILT_1);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tilt2Pin, PIN_TILT_2);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogDownPin, PIN_TILT_LEFT_ANALOG_DOWN);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogUpPin, PIN_TILT_LEFT_ANALOG_UP);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogLeftPin, PIN_TILT_LEFT_ANALOG_LEFT);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltLeftAnalogRightPin, PIN_TILT_LEFT_ANALOG_RIGHT);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogDownPin, PIN_TILT_RIGHT_ANALOG_DOWN);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogUpPin, PIN_TILT_RIGHT_ANALOG_UP);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogLeftPin, PIN_TILT_RIGHT_ANALOG_LEFT);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltRightAnalogRightPin, PIN_TILT_RIGHT_ANALOG_RIGHT);
    INIT_UNSET_PROPERTY(config.addonOptions.tiltOptions, tiltSOCDMode, TILT_SOCD_MODE);

    // addonOptions.buzzerOptions
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, enabled, !!BUZZER_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, pin, BUZZER_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, volume, BUZZER_VOLUME);

    // addonOptions.extraOptions
    INIT_UNSET_PROPERTY(config.addonOptions.extraButtonOptions, enabled, !!EXTRA_BUTTON_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.extraButtonOptions, pin, EXTRA_BUTTON_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.extraButtonOptions, buttonMap, EXTRA_BUTTON_MASK);

    // addonOptions.playerNumberOptions
    INIT_UNSET_PROPERTY(config.addonOptions.playerNumberOptions, enabled, !!PLAYERNUM_ADDON_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.playerNumberOptions, number, PLAYER_NUMBER);

    // addonOptions.ps4Options
    INIT_UNSET_PROPERTY(config.addonOptions.ps4Options, enabled, PS4MODE_ADDON_ENABLED);
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
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, i2cBlock, (WII_EXTENSION_I2C_BLOCK == i2c0) ? 0 : 1);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, i2cSDAPin, WII_EXTENSION_I2C_SDA_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, i2cSCLPin, WII_EXTENSION_I2C_SCL_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.wiiOptions, i2cSpeed, WII_EXTENSION_I2C_SPEED);

    // addonOptions.snesOptions
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, enabled, !!SNES_PAD_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, clockPin, SNES_PAD_CLOCK_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, latchPin, SNES_PAD_LATCH_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.snesOptions, dataPin, SNES_PAD_DATA_PIN);

    // keyboardMapping
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, enabled, KEYBOARD_HOST_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, pinDplus, KEYBOARD_HOST_PIN_DPLUS);
    INIT_UNSET_PROPERTY(config.addonOptions.keyboardHostOptions, pin5V, KEYBOARD_HOST_PIN_5V);
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
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, pin, FOCUS_MODE_PIN);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, buttonLockMask, FOCUS_MODE_BUTTON_MASK);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, oledLockEnabled, !!FOCUS_MODE_OLED_LOCK_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, rgbLockEnabled, !!FOCUS_MODE_RGB_LOCK_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.focusModeOptions, buttonLockEnabled, !!FOCUS_MODE_BUTTON_LOCK_ENABLED);

    // PS Passthrough
    INIT_UNSET_PROPERTY(config.addonOptions.psPassthroughOptions, enabled, PSPASSTHROUGH_ENABLED);
    INIT_UNSET_PROPERTY(config.addonOptions.psPassthroughOptions, pinDplus, PSPASSTHROUGH_PIN_DPLUS);
    INIT_UNSET_PROPERTY(config.addonOptions.psPassthroughOptions, pin5V, PSPASSTHROUGH_PIN_5V);
}


// -----------------------------------------------------
// migrations
// used for when we might need to populate configs with
// something *other than* the board defaults
// -----------------------------------------------------

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
    hotkeysMigration(config);

    // Make sure that fields that were not deserialized are properly initialized.
    // They were probably added with a newer version of the firmware.
    initUnsetPropertiesWithDefaults(config);

    // ----------------------------------------
    // Further migrations can be performed here
    // ----------------------------------------

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

        if (PB_HTYPE(iter.type) == PB_HTYPE_OPTIONAL && iter.pSize)
        {
            *reinterpret_cast<char*>(iter.pSize) = true;
        }

        // Recurse into sub-messages
        if (PB_LTYPE(iter.type) == PB_LTYPE_SUBMESSAGE)
        {
            assert(iter.submsg_desc);
            assert(iter.pData);

            setHasFlags(iter.submsg_desc, iter.pData);
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
#define TO_JSON_INT32(fieldname, submessageType) appendAsString(str, s.fieldname);
#define TO_JSON_UINT32(fieldname, submessageType) appendAsString(str, s.fieldname);
#define TO_JSON_BOOL(fieldname, submessageType) str.append((s.fieldname) ? "true" : "false");
#define TO_JSON_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname); str.push_back('"');
#define TO_JSON_BYTES(fieldname, submessageType) str.push_back('"'); str.append(Base64::Encode(reinterpret_cast<const char*>(s.fieldname.bytes), s.fieldname.size)); str.push_back('"');
#define TO_JSON_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname, indentLevel + 1);

#define TO_JSON_REPEATED_RENUM(fieldname, submessageType) appendAsString(str, static_cast<int32_t>(s.fieldname[i]));
#define TO_JSON_REPEATED_UENUM(fieldname, submessageType) appendAsString(str, static_cast<uint32_t>(s.fieldname[i]));
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

    return true;
}
