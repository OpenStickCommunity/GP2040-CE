#include "config_legacy.h"
#include "config_utils.h"

#include "FlashPROM.h"
#include "CRC32.h"
#include "helper.h"

#include "config.pb.h"

const size_t GAMEPAD_STORAGE_INDEX      =    0; // 1024 bytes for gamepad options
const size_t BOARD_STORAGE_INDEX        = 1024; //  512 bytes for hardware options
const size_t LED_STORAGE_INDEX          = 1536; //  512 bytes for LED configuration
const size_t ANIMATION_STORAGE_INDEX    = 2048; // 1024 bytes for LED animations
const size_t ADDON_STORAGE_INDEX        = 3072; // 1024 bytes for Add-Ons
const size_t PS4_STORAGE_INDEX          = 4096; // 2048 bytes for PS4 options
const size_t SPLASH_IMAGE_STORAGE_INDEX = 6144; // 1032 bytes for Display Config

const uint32_t CHECKSUM_MAGIC   = 0;
const uint32_t NOCHECKSUM_MAGIC = 0xDEADBEEF;   // No checksum CRC;

namespace ConfigLegacy
{
    enum InputMode
    {
        INPUT_MODE_XINPUT,
        INPUT_MODE_SWITCH,
        INPUT_MODE_HID,
        INPUT_MODE_KEYBOARD,
        INPUT_MODE_PS4,
        INPUT_MODE_CONFIG = 255,
    };

    enum DpadMode
    {
        DPAD_MODE_DIGITAL,
        DPAD_MODE_LEFT_ANALOG,
        DPAD_MODE_RIGHT_ANALOG,
    };

    enum SOCDMode
    {
        SOCD_MODE_UP_PRIORITY,
        SOCD_MODE_NEUTRAL,
        SOCD_MODE_SECOND_INPUT_PRIORITY,
        SOCD_MODE_FIRST_INPUT_PRIORITY,
        SOCD_MODE_BYPASS,
    };

    enum DpadDirection
    {
        DIRECTION_NONE,
        DIRECTION_UP,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
        DIRECTION_RIGHT,
    };

    enum GamepadHotkey
    {
        HOTKEY_NONE,
        HOTKEY_DPAD_DIGITAL,
        HOTKEY_DPAD_LEFT_ANALOG,
        HOTKEY_DPAD_RIGHT_ANALOG,
        HOTKEY_HOME_BUTTON,
        HOTKEY_CAPTURE_BUTTON,
        HOTKEY_SOCD_UP_PRIORITY,
        HOTKEY_SOCD_NEUTRAL,
        HOTKEY_SOCD_LAST_INPUT,
        HOTKEY_INVERT_X_AXIS,
        HOTKEY_INVERT_Y_AXIS,
        HOTKEY_SOCD_FIRST_INPUT,
        HOTKEY_SOCD_BYPASS,
    };

    enum OnBoardLedMode
    {
        BOARD_LED_OFF,
        MODE_INDICATOR,
        INPUT_TEST,
    };

    struct LEDOptions
    {
        bool useUserDefinedLEDs;
        int dataPin;
        LEDFormat ledFormat;
        ButtonLayout ledLayout;
        uint8_t ledsPerButton;
        uint8_t brightnessMaximum;
        uint8_t brightnessSteps;
        int indexUp;
        int indexDown;
        int indexLeft;
        int indexRight;
        int indexB1;
        int indexB2;
        int indexB3;
        int indexB4;
        int indexL1;
        int indexR1;
        int indexL2;
        int indexR2;
        int indexS1;
        int indexS2;
        int indexL3;
        int indexR3;
        int indexA1;
        int indexA2;
        int pledType;
        int pledPin1;
        int pledPin2;
        int pledPin3;
        int pledPin4;
        RGB pledColor;
        uint32_t checksum;
    };

    struct PS4Options {
        uint8_t serial[16];
        uint8_t signature[256];
        mbedtls_mpi_uint rsa_n[64];
        mbedtls_mpi_uint rsa_e[1];
        mbedtls_mpi_uint rsa_d[64];
        mbedtls_mpi_uint rsa_p[32];
        mbedtls_mpi_uint rsa_q[32];
        mbedtls_mpi_uint rsa_dp[32];
        mbedtls_mpi_uint rsa_dq[32];
        mbedtls_mpi_uint rsa_qp[32];
        mbedtls_mpi_uint rsa_rn[64];
        uint32_t checksum;
    };

    struct AddonOptions {
        uint8_t pinButtonTurbo;
        uint8_t pinButtonReverse;
        uint8_t pinSliderLS;
        uint8_t pinSliderRS;
        uint8_t pinSliderSOCDOne;
        uint8_t pinSliderSOCDTwo;
        uint8_t turboShotCount; // Turbo
        uint8_t pinTurboLED;    // Turbo LED
        uint8_t pinReverseLED;    // Reverse LED
        uint8_t reverseActionUp;
        uint8_t reverseActionDown;
        uint8_t reverseActionLeft;
        uint8_t reverseActionRight;
        uint8_t i2cAnalog1219SDAPin;
        uint8_t i2cAnalog1219SCLPin;
        int i2cAnalog1219Block;
        uint32_t i2cAnalog1219Speed;
        uint8_t i2cAnalog1219Address;
        uint8_t pinDualDirUp;    // Pins for Dual Directional Input
        uint8_t pinDualDirDown;
        uint8_t pinDualDirLeft;
        uint8_t pinDualDirRight;
        DpadMode dualDirDpadMode;    // LS/DP/RS
        uint8_t dualDirCombineMode; // Mix/Gamepad/Dual/None
        OnBoardLedMode onBoardLedMode;
        uint8_t analogAdcPinX;
        uint8_t analogAdcPinY;
        uint16_t bootselButtonMap;
        uint8_t extraButtonPin;
        uint32_t extraButtonMap;
        uint8_t buzzerPin;
        uint8_t buzzerVolume;
        uint8_t playerNumber;
        uint8_t shmupMode; // Turbo SHMUP Mode
        uint8_t shmupMixMode; // How we mix turbo and non-turbo buttons
        uint16_t shmupAlwaysOn1;
        uint16_t shmupAlwaysOn2;
        uint16_t shmupAlwaysOn3;
        uint16_t shmupAlwaysOn4;
        uint8_t pinShmupBtn1;
        uint8_t pinShmupBtn2;
        uint8_t pinShmupBtn3;
        uint8_t pinShmupBtn4;
        uint16_t shmupBtnMask1;
        uint16_t shmupBtnMask2;
        uint16_t shmupBtnMask3;
        uint16_t shmupBtnMask4;
        uint8_t pinShmupDial;
        SOCDMode sliderSOCDModeOne;
        SOCDMode sliderSOCDModeTwo;
        SOCDMode sliderSOCDModeDefault;
        uint8_t wiiExtensionSDAPin;
        uint8_t wiiExtensionSCLPin;
        int wiiExtensionBlock;
        uint32_t wiiExtensionSpeed;
        uint8_t AnalogInputEnabled;
        uint8_t BoardLedAddonEnabled;
        uint8_t BootselButtonAddonEnabled;
        uint8_t BuzzerSpeakerAddonEnabled;
        uint8_t DualDirectionalInputEnabled;
        uint8_t ExtraButtonAddonEnabled;
        uint8_t I2CAnalog1219InputEnabled;
        //bool I2CDisplayAddonEnabled; // I2C is special case
        uint8_t JSliderInputEnabled;
        //bool NeoPicoLEDAddonEnabled; // NeoPico is special case
        //bool PlayerLEDAddonEnabled; // PlayerLED is special case
        uint8_t PlayerNumAddonEnabled;
        uint8_t PS4ModeAddonEnabled;
        uint8_t ReverseInputEnabled;
        uint8_t TurboInputEnabled;
        uint8_t SliderSOCDInputEnabled;
        uint8_t WiiExtensionAddonEnabled;
        uint32_t checksum;
    };

    struct SplashImage {
        uint8_t data[16*64];
        uint32_t checksum;
    };

    struct GamepadHotkeyEntry
    {
        uint8_t dpadMask;
        GamepadHotkey action;
    };

    struct GamepadOptions
    {
        InputMode inputMode {InputMode::INPUT_MODE_XINPUT}; 
        DpadMode dpadMode {DpadMode::DPAD_MODE_DIGITAL};
        SOCDMode socdMode {SOCDMode::SOCD_MODE_NEUTRAL};
        bool invertXAxis;
        bool invertYAxis;

        uint8_t keyDpadUp;
        uint8_t keyDpadDown;
        uint8_t keyDpadLeft;
        uint8_t keyDpadRight;
        uint8_t keyButtonB1;
        uint8_t keyButtonB2;
        uint8_t keyButtonB3;
        uint8_t keyButtonB4;
        uint8_t keyButtonL1;
        uint8_t keyButtonR1;
        uint8_t keyButtonL2;
        uint8_t keyButtonR2;
        uint8_t keyButtonS1;
        uint8_t keyButtonS2;
        uint8_t keyButtonL3;
        uint8_t keyButtonR3;
        uint8_t keyButtonA1;
        uint8_t keyButtonA2;

        GamepadHotkeyEntry hotkeyF1Up;
        GamepadHotkeyEntry hotkeyF1Down;
        GamepadHotkeyEntry hotkeyF1Left;
        GamepadHotkeyEntry hotkeyF1Right;
        GamepadHotkeyEntry hotkeyF2Up;
        GamepadHotkeyEntry hotkeyF2Down;
        GamepadHotkeyEntry hotkeyF2Left;
        GamepadHotkeyEntry hotkeyF2Right;

        bool switchTpShareForDs4;

        uint32_t checksum;
    };

    struct __attribute__ ((__packed__)) AnimationOptions
    {
        uint32_t checksum;
        uint8_t baseAnimationIndex;
        uint8_t brightness;
        uint8_t staticColorIndex;
        uint8_t buttonColorIndex;
        int16_t chaseCycleTime;
        int16_t rainbowCycleTime;
        uint8_t themeIndex;
        bool hasCustomTheme;
        uint32_t customThemeUp;
        uint32_t customThemeDown;
        uint32_t customThemeLeft;
        uint32_t customThemeRight;
        uint32_t customThemeB1;
        uint32_t customThemeB2;
        uint32_t customThemeB3;
        uint32_t customThemeB4;
        uint32_t customThemeL1;
        uint32_t customThemeR1;
        uint32_t customThemeL2;
        uint32_t customThemeR2;
        uint32_t customThemeS1;
        uint32_t customThemeS2;
        uint32_t customThemeL3;
        uint32_t customThemeR3;
        uint32_t customThemeA1;
        uint32_t customThemeA2;
        uint32_t customThemeUpPressed;
        uint32_t customThemeDownPressed;
        uint32_t customThemeLeftPressed;
        uint32_t customThemeRightPressed;
        uint32_t customThemeB1Pressed;
        uint32_t customThemeB2Pressed;
        uint32_t customThemeB3Pressed;
        uint32_t customThemeB4Pressed;
        uint32_t customThemeL1Pressed;
        uint32_t customThemeR1Pressed;
        uint32_t customThemeL2Pressed;
        uint32_t customThemeR2Pressed;
        uint32_t customThemeS1Pressed;
        uint32_t customThemeS2Pressed;
        uint32_t customThemeL3Pressed;
        uint32_t customThemeR3Pressed;
        uint32_t customThemeA1Pressed;
        uint32_t customThemeA2Pressed;
    };
}

#define PREPROCESSOR_JOIN2(x, y) x ## y
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN2(x, y)

#define SET_PROPERTY_BYTES(parent, property, byteArray) \
    parent.property.size = sizeof(byteArray); \
    memcpy(parent.property.bytes, byteArray, std::min(sizeof(byteArray), sizeof(parent.property.bytes))); \
    parent.PREPROCESSOR_JOIN(has_, property) = true;

#define SET_PROPERTY(parent, property, value) \
    parent.property = value; \
    parent.PREPROCESSOR_JOIN(has_, property) = true;

static uint32_t computeChecksum(const char* obj, size_t size, size_t checksumOffset)
{
    CRC32 crc32;
    if (checksumOffset + sizeof(uint32_t) > size)
    {
        crc32.update(obj, size);
    }
    else
    {
        crc32.update(obj, checksumOffset);
        const uint32_t checksumMagic = CHECKSUM_MAGIC;
        crc32.update(&checksumMagic, 1);
        crc32.update(obj + checksumOffset + sizeof(uint32_t), size - checksumOffset - sizeof(uint32_t));
    }
    return crc32.finalize();
}

bool ConfigUtils::fromLegacyStorage(Config& config)
{
    bool legacyConfigFound = false;

    const auto bytePinToIntPin = [](uint8_t pin) -> int32_t { return pin == 0xFF ? -1 : pin; };

    const ConfigLegacy::GamepadOptions& legacyGamepadOptions = *reinterpret_cast<ConfigLegacy::GamepadOptions*>(EEPROM_ADDRESS_START + GAMEPAD_STORAGE_INDEX);
    if (legacyGamepadOptions.checksum == computeChecksum(reinterpret_cast<const char*>(&legacyGamepadOptions), sizeof(ConfigLegacy::GamepadOptions), offsetof(ConfigLegacy::GamepadOptions, checksum)))
    {
        legacyConfigFound = true;

        GamepadOptions& gamepadOptions = config.gamepadOptions;
        config.has_gamepadOptions = true;
        SET_PROPERTY(gamepadOptions, inputMode, static_cast<InputMode>(legacyGamepadOptions.inputMode));
        SET_PROPERTY(gamepadOptions, dpadMode, static_cast<DpadMode>(legacyGamepadOptions.dpadMode));
        SET_PROPERTY(gamepadOptions, socdMode, static_cast<SOCDMode>(legacyGamepadOptions.socdMode));
        SET_PROPERTY(gamepadOptions, invertXAxis, legacyGamepadOptions.invertXAxis);
        SET_PROPERTY(gamepadOptions, invertYAxis, legacyGamepadOptions.invertYAxis);
        SET_PROPERTY(gamepadOptions, switchTpShareForDs4, legacyGamepadOptions.switchTpShareForDs4);

        KeyboardMapping& keyboardMapping = config.keyboardMapping;
        config.has_keyboardMapping = true;
        SET_PROPERTY(keyboardMapping, keyDpadUp, legacyGamepadOptions.keyDpadUp);
        SET_PROPERTY(keyboardMapping, keyDpadDown, legacyGamepadOptions.keyDpadDown);
        SET_PROPERTY(keyboardMapping, keyDpadLeft, legacyGamepadOptions.keyDpadLeft);
        SET_PROPERTY(keyboardMapping, keyDpadRight, legacyGamepadOptions.keyDpadRight);
        SET_PROPERTY(keyboardMapping, keyButtonB1, legacyGamepadOptions.keyButtonB1);
        SET_PROPERTY(keyboardMapping, keyButtonB2, legacyGamepadOptions.keyButtonB2);
        SET_PROPERTY(keyboardMapping, keyButtonB3, legacyGamepadOptions.keyButtonB3);
        SET_PROPERTY(keyboardMapping, keyButtonB4, legacyGamepadOptions.keyButtonB4);
        SET_PROPERTY(keyboardMapping, keyButtonL1, legacyGamepadOptions.keyButtonL1);
        SET_PROPERTY(keyboardMapping, keyButtonR1, legacyGamepadOptions.keyButtonR1);
        SET_PROPERTY(keyboardMapping, keyButtonL2, legacyGamepadOptions.keyButtonL2);
        SET_PROPERTY(keyboardMapping, keyButtonR2, legacyGamepadOptions.keyButtonR2);
        SET_PROPERTY(keyboardMapping, keyButtonS1, legacyGamepadOptions.keyButtonS1);
        SET_PROPERTY(keyboardMapping, keyButtonS2, legacyGamepadOptions.keyButtonS2);
        SET_PROPERTY(keyboardMapping, keyButtonL3, legacyGamepadOptions.keyButtonL3);
        SET_PROPERTY(keyboardMapping, keyButtonR3, legacyGamepadOptions.keyButtonR3);
        SET_PROPERTY(keyboardMapping, keyButtonA1, legacyGamepadOptions.keyButtonA1);
        SET_PROPERTY(keyboardMapping, keyButtonA2, legacyGamepadOptions.keyButtonA2);

        HotkeyOptions& hotkeyOptions = config.hotkeyOptions;
        config.has_hotkeyOptions = true;
        SET_PROPERTY(hotkeyOptions.hotkeyF1Up, dpadMask, legacyGamepadOptions.hotkeyF1Up.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF1Up, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF1Up.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF1Down, dpadMask, legacyGamepadOptions.hotkeyF1Down.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF1Down, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF1Down.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF1Left, dpadMask, legacyGamepadOptions.hotkeyF1Left.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF1Left, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF1Left.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF1Right, dpadMask, legacyGamepadOptions.hotkeyF1Right.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF1Right, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF1Right.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF2Up, dpadMask, legacyGamepadOptions.hotkeyF2Up.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF2Up, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF2Up.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF2Down, dpadMask, legacyGamepadOptions.hotkeyF2Down.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF2Down, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF2Down.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF2Left, dpadMask, legacyGamepadOptions.hotkeyF2Left.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF2Left, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF2Left.action));
        SET_PROPERTY(hotkeyOptions.hotkeyF2Right, dpadMask, legacyGamepadOptions.hotkeyF2Right.dpadMask);
        SET_PROPERTY(hotkeyOptions.hotkeyF2Right, action, static_cast<GamepadHotkey>(legacyGamepadOptions.hotkeyF2Right.action));
    }

    const ConfigLegacy::LEDOptions& legacyLEDOptions = *reinterpret_cast<ConfigLegacy::LEDOptions*>(EEPROM_ADDRESS_START + LED_STORAGE_INDEX);
    if (legacyLEDOptions.checksum == computeChecksum(reinterpret_cast<const char*>(&legacyLEDOptions), sizeof(ConfigLegacy::LEDOptions), offsetof(ConfigLegacy::LEDOptions, checksum)) &&
        legacyLEDOptions.useUserDefinedLEDs)
    {
        legacyConfigFound = true;

        LEDOptions& ledOptions = config.ledOptions;
        config.has_ledOptions = true;
        SET_PROPERTY(ledOptions, dataPin, isValidPin(legacyLEDOptions.dataPin) ? legacyLEDOptions.dataPin : -1);
        SET_PROPERTY(ledOptions, ledFormat, static_cast<LEDFormat_Proto>(legacyLEDOptions.ledFormat));
        SET_PROPERTY(ledOptions, ledLayout, static_cast<ButtonLayout>(legacyLEDOptions.ledLayout));
        SET_PROPERTY(ledOptions, ledsPerButton, legacyLEDOptions.ledsPerButton);
        SET_PROPERTY(ledOptions, brightnessMaximum, legacyLEDOptions.brightnessMaximum);
        SET_PROPERTY(ledOptions, brightnessSteps, legacyLEDOptions.brightnessSteps);
        SET_PROPERTY(ledOptions, indexUp, legacyLEDOptions.indexUp);
        SET_PROPERTY(ledOptions, indexDown, legacyLEDOptions.indexDown);
        SET_PROPERTY(ledOptions, indexLeft, legacyLEDOptions.indexLeft);
        SET_PROPERTY(ledOptions, indexRight, legacyLEDOptions.indexRight);
        SET_PROPERTY(ledOptions, indexB1, legacyLEDOptions.indexB1);
        SET_PROPERTY(ledOptions, indexB2, legacyLEDOptions.indexB2);
        SET_PROPERTY(ledOptions, indexB3, legacyLEDOptions.indexB3);
        SET_PROPERTY(ledOptions, indexB4, legacyLEDOptions.indexB4);
        SET_PROPERTY(ledOptions, indexL1, legacyLEDOptions.indexL1);
        SET_PROPERTY(ledOptions, indexR1, legacyLEDOptions.indexR1);
        SET_PROPERTY(ledOptions, indexL2, legacyLEDOptions.indexL2);
        SET_PROPERTY(ledOptions, indexR2, legacyLEDOptions.indexR2);
        SET_PROPERTY(ledOptions, indexS1, legacyLEDOptions.indexS1);
        SET_PROPERTY(ledOptions, indexS2, legacyLEDOptions.indexS2);
        SET_PROPERTY(ledOptions, indexL3, legacyLEDOptions.indexL3);
        SET_PROPERTY(ledOptions, indexR3, legacyLEDOptions.indexR3);
        SET_PROPERTY(ledOptions, indexA1, legacyLEDOptions.indexA1);
        SET_PROPERTY(ledOptions, indexA2, legacyLEDOptions.indexA2);
        switch (legacyLEDOptions.pledType)
        {
            case PLED_TYPE_NONE:
            case PLED_TYPE_PWM:
            case PLED_TYPE_RGB:
                SET_PROPERTY(ledOptions, pledType, static_cast<PLEDType>(legacyLEDOptions.pledType));
                break;
        }
        SET_PROPERTY(ledOptions, pledPin1, legacyLEDOptions.pledPin1);
        SET_PROPERTY(ledOptions, pledPin2, legacyLEDOptions.pledPin2);
        SET_PROPERTY(ledOptions, pledPin3, legacyLEDOptions.pledPin3);
        SET_PROPERTY(ledOptions, pledPin4, legacyLEDOptions.pledPin4);
        SET_PROPERTY(ledOptions, pledColor, legacyLEDOptions.pledColor.value(LED_FORMAT_RGB));
    }

    const ConfigLegacy::AnimationOptions& legacyAnimationOptions = *reinterpret_cast<ConfigLegacy::AnimationOptions*>(EEPROM_ADDRESS_START + ANIMATION_STORAGE_INDEX);
    if (legacyAnimationOptions.checksum == computeChecksum(reinterpret_cast<const char*>(&legacyAnimationOptions), sizeof(ConfigLegacy::AnimationOptions), offsetof(ConfigLegacy::AnimationOptions, checksum)))
    {
        legacyConfigFound = true;

        AnimationOptions_Proto& animationOptions = config.animationOptions;
        config.has_animationOptions = true;
        SET_PROPERTY(animationOptions, baseAnimationIndex, legacyAnimationOptions.baseAnimationIndex);
        SET_PROPERTY(animationOptions, brightness, legacyAnimationOptions.brightness);
        SET_PROPERTY(animationOptions, staticColorIndex, legacyAnimationOptions.staticColorIndex);
        SET_PROPERTY(animationOptions, buttonColorIndex, legacyAnimationOptions.buttonColorIndex);
        SET_PROPERTY(animationOptions, chaseCycleTime, legacyAnimationOptions.chaseCycleTime);
        SET_PROPERTY(animationOptions, rainbowCycleTime, legacyAnimationOptions.rainbowCycleTime);
        SET_PROPERTY(animationOptions, themeIndex, legacyAnimationOptions.themeIndex);
        SET_PROPERTY(animationOptions, hasCustomTheme, legacyAnimationOptions.hasCustomTheme);
        SET_PROPERTY(animationOptions, customThemeUp, legacyAnimationOptions.customThemeUp);
        SET_PROPERTY(animationOptions, customThemeDown, legacyAnimationOptions.customThemeDown);
        SET_PROPERTY(animationOptions, customThemeLeft, legacyAnimationOptions.customThemeLeft);
        SET_PROPERTY(animationOptions, customThemeRight, legacyAnimationOptions.customThemeRight);
        SET_PROPERTY(animationOptions, customThemeB1, legacyAnimationOptions.customThemeB1);
        SET_PROPERTY(animationOptions, customThemeB2, legacyAnimationOptions.customThemeB2);
        SET_PROPERTY(animationOptions, customThemeB3, legacyAnimationOptions.customThemeB3);
        SET_PROPERTY(animationOptions, customThemeB4, legacyAnimationOptions.customThemeB4);
        SET_PROPERTY(animationOptions, customThemeL1, legacyAnimationOptions.customThemeL1);
        SET_PROPERTY(animationOptions, customThemeR1, legacyAnimationOptions.customThemeR1);
        SET_PROPERTY(animationOptions, customThemeL2, legacyAnimationOptions.customThemeL2);
        SET_PROPERTY(animationOptions, customThemeR2, legacyAnimationOptions.customThemeR2);
        SET_PROPERTY(animationOptions, customThemeS1, legacyAnimationOptions.customThemeS1);
        SET_PROPERTY(animationOptions, customThemeS2, legacyAnimationOptions.customThemeS2);
        SET_PROPERTY(animationOptions, customThemeL3, legacyAnimationOptions.customThemeL3);
        SET_PROPERTY(animationOptions, customThemeR3, legacyAnimationOptions.customThemeR3);
        SET_PROPERTY(animationOptions, customThemeA1, legacyAnimationOptions.customThemeA1);
        SET_PROPERTY(animationOptions, customThemeA2, legacyAnimationOptions.customThemeA2);
        SET_PROPERTY(animationOptions, customThemeUpPressed, legacyAnimationOptions.customThemeUpPressed);
        SET_PROPERTY(animationOptions, customThemeDownPressed, legacyAnimationOptions.customThemeDownPressed);
        SET_PROPERTY(animationOptions, customThemeLeftPressed, legacyAnimationOptions.customThemeLeftPressed);
        SET_PROPERTY(animationOptions, customThemeRightPressed, legacyAnimationOptions.customThemeRightPressed);
        SET_PROPERTY(animationOptions, customThemeB1Pressed, legacyAnimationOptions.customThemeB1Pressed);
        SET_PROPERTY(animationOptions, customThemeB2Pressed, legacyAnimationOptions.customThemeB2Pressed);
        SET_PROPERTY(animationOptions, customThemeB3Pressed, legacyAnimationOptions.customThemeB3Pressed);
        SET_PROPERTY(animationOptions, customThemeB4Pressed, legacyAnimationOptions.customThemeB4Pressed);
        SET_PROPERTY(animationOptions, customThemeL1Pressed, legacyAnimationOptions.customThemeL1Pressed);
        SET_PROPERTY(animationOptions, customThemeR1Pressed, legacyAnimationOptions.customThemeR1Pressed);
        SET_PROPERTY(animationOptions, customThemeL2Pressed, legacyAnimationOptions.customThemeL2Pressed);
        SET_PROPERTY(animationOptions, customThemeR2Pressed, legacyAnimationOptions.customThemeR2Pressed);
        SET_PROPERTY(animationOptions, customThemeS1Pressed, legacyAnimationOptions.customThemeS1Pressed);
        SET_PROPERTY(animationOptions, customThemeS2Pressed, legacyAnimationOptions.customThemeS2Pressed);
        SET_PROPERTY(animationOptions, customThemeL3Pressed, legacyAnimationOptions.customThemeL3Pressed);
        SET_PROPERTY(animationOptions, customThemeR3Pressed, legacyAnimationOptions.customThemeR3Pressed);
        SET_PROPERTY(animationOptions, customThemeA1Pressed, legacyAnimationOptions.customThemeA1Pressed);
        SET_PROPERTY(animationOptions, customThemeA2Pressed, legacyAnimationOptions.customThemeA2Pressed);
    }

    const ConfigLegacy::AddonOptions& legacyAddonOptions = *reinterpret_cast<ConfigLegacy::AddonOptions*>(EEPROM_ADDRESS_START + ADDON_STORAGE_INDEX);
    if (legacyAddonOptions.checksum == computeChecksum(reinterpret_cast<const char*>(&legacyAddonOptions), sizeof(ConfigLegacy::AddonOptions), offsetof(ConfigLegacy::AddonOptions, checksum)))
    {
        legacyConfigFound = true;

        config.has_addonOptions = true;

        AnalogOptions& analogOptions = config.addonOptions.analogOptions;
        config.addonOptions.has_analogOptions = true;
        SET_PROPERTY(analogOptions, enabled, legacyAddonOptions.AnalogInputEnabled);
        SET_PROPERTY(analogOptions, analogAdcPinX, bytePinToIntPin(legacyAddonOptions.analogAdcPinX));
        SET_PROPERTY(analogOptions, analogAdcPinY, bytePinToIntPin(legacyAddonOptions.analogAdcPinY));

        BootselButtonOptions& bootselButtonOptions = config.addonOptions.bootselButtonOptions;
        config.addonOptions.has_bootselButtonOptions = true;
        SET_PROPERTY(bootselButtonOptions, enabled, legacyAddonOptions.BootselButtonAddonEnabled);
        SET_PROPERTY(bootselButtonOptions, buttonMap, legacyAddonOptions.bootselButtonMap);

        BuzzerOptions& buzzerOptions = config.addonOptions.buzzerOptions;
        config.addonOptions.has_buzzerOptions = true;
        SET_PROPERTY(buzzerOptions, enabled, legacyAddonOptions.BuzzerSpeakerAddonEnabled);
        SET_PROPERTY(buzzerOptions, pin, bytePinToIntPin(legacyAddonOptions.buzzerPin));
        SET_PROPERTY(buzzerOptions, volume, legacyAddonOptions.buzzerVolume);

        DualDirectionalOptions& dualDirectionalOptions = config.addonOptions.dualDirectionalOptions;
        config.addonOptions.has_dualDirectionalOptions = true;
        SET_PROPERTY(dualDirectionalOptions, enabled, legacyAddonOptions.DualDirectionalInputEnabled);
        SET_PROPERTY(dualDirectionalOptions, upPin, bytePinToIntPin(legacyAddonOptions.pinDualDirUp));
        SET_PROPERTY(dualDirectionalOptions, downPin, bytePinToIntPin(legacyAddonOptions.pinDualDirDown));
        SET_PROPERTY(dualDirectionalOptions, leftPin, bytePinToIntPin(legacyAddonOptions.pinDualDirLeft));
        SET_PROPERTY(dualDirectionalOptions, rightPin, bytePinToIntPin(legacyAddonOptions.pinDualDirRight));
        SET_PROPERTY(dualDirectionalOptions, dpadMode, static_cast<DpadMode>(legacyAddonOptions.dualDirDpadMode));
        SET_PROPERTY(dualDirectionalOptions, combineMode, legacyAddonOptions.dualDirCombineMode);

        ExtraButtonOptions& extraButtonOptions = config.addonOptions.extraButtonOptions;
        config.addonOptions.has_extraButtonOptions = true;
        SET_PROPERTY(extraButtonOptions, enabled, legacyAddonOptions.ExtraButtonAddonEnabled);
        SET_PROPERTY(extraButtonOptions, pin, bytePinToIntPin(legacyAddonOptions.extraButtonPin));
        SET_PROPERTY(extraButtonOptions, buttonMap, legacyAddonOptions.extraButtonMap);

        AnalogADS1219Options& analogADS1219Options = config.addonOptions.analogADS1219Options;
        config.addonOptions.has_analogADS1219Options = true;
        SET_PROPERTY(analogADS1219Options, enabled, legacyAddonOptions.I2CAnalog1219InputEnabled);
        SET_PROPERTY(analogADS1219Options, i2cBlock, legacyAddonOptions.i2cAnalog1219Block);
        SET_PROPERTY(analogADS1219Options, i2cSDAPin, bytePinToIntPin(legacyAddonOptions.i2cAnalog1219SDAPin));
        SET_PROPERTY(analogADS1219Options, i2cSCLPin, bytePinToIntPin(legacyAddonOptions.i2cAnalog1219SCLPin));
        SET_PROPERTY(analogADS1219Options, i2cAddress, legacyAddonOptions.i2cAnalog1219Address);
        SET_PROPERTY(analogADS1219Options, i2cSpeed, legacyAddonOptions.i2cAnalog1219Speed);

        SliderOptions& sliderOptions = config.addonOptions.sliderOptions;
        config.addonOptions.has_sliderOptions = true;
        SET_PROPERTY(sliderOptions, enabled, legacyAddonOptions.JSliderInputEnabled);
        SET_PROPERTY(sliderOptions, pinLS, bytePinToIntPin(legacyAddonOptions.pinSliderLS));
        SET_PROPERTY(sliderOptions, pinRS, bytePinToIntPin(legacyAddonOptions.pinSliderRS));

        PlayerNumberOptions& playerNumberOptions = config.addonOptions.playerNumberOptions;
        config.addonOptions.has_playerNumberOptions = true;
        SET_PROPERTY(playerNumberOptions, enabled, legacyAddonOptions.PlayerNumAddonEnabled);
        SET_PROPERTY(playerNumberOptions, number, legacyAddonOptions.playerNumber);

        ReverseOptions& reverseOptions = config.addonOptions.reverseOptions;
        config.addonOptions.has_reverseOptions = true;
        SET_PROPERTY(reverseOptions, enabled, legacyAddonOptions.ReverseInputEnabled);
        SET_PROPERTY(reverseOptions, buttonPin, bytePinToIntPin(legacyAddonOptions.pinButtonReverse));
        SET_PROPERTY(reverseOptions, ledPin, bytePinToIntPin(legacyAddonOptions.pinReverseLED));
        SET_PROPERTY(reverseOptions, actionUp, legacyAddonOptions.reverseActionUp);
        SET_PROPERTY(reverseOptions, actionDown, legacyAddonOptions.reverseActionDown);
        SET_PROPERTY(reverseOptions, actionLeft, legacyAddonOptions.reverseActionLeft);
        SET_PROPERTY(reverseOptions, actionRight, legacyAddonOptions.reverseActionRight);

        SOCDSliderOptions& socdSliderOptions = config.addonOptions.socdSliderOptions;
        config.addonOptions.has_socdSliderOptions = true;
        SET_PROPERTY(socdSliderOptions, enabled, legacyAddonOptions.SliderSOCDInputEnabled);
        SET_PROPERTY(socdSliderOptions, pinOne, bytePinToIntPin(legacyAddonOptions.pinSliderSOCDOne));
        SET_PROPERTY(socdSliderOptions, pinTwo, bytePinToIntPin(legacyAddonOptions.pinSliderSOCDTwo));
        SET_PROPERTY(socdSliderOptions, modeDefault, static_cast<SOCDMode>(legacyAddonOptions.sliderSOCDModeDefault));
        SET_PROPERTY(socdSliderOptions, modeOne, static_cast<SOCDMode>(legacyAddonOptions.sliderSOCDModeOne));
        SET_PROPERTY(socdSliderOptions, modeTwo, static_cast<SOCDMode>(legacyAddonOptions.sliderSOCDModeTwo));

        OnBoardLedOptions& onBoardLedOptions = config.addonOptions.onBoardLedOptions;
        config.addonOptions.has_onBoardLedOptions = true;
        SET_PROPERTY(onBoardLedOptions, enabled, legacyAddonOptions.BoardLedAddonEnabled);
        SET_PROPERTY(onBoardLedOptions, mode, static_cast<OnBoardLedMode>(legacyAddonOptions.onBoardLedMode));

        TurboOptions& turboOptions = config.addonOptions.turboOptions;
        config.addonOptions.has_turboOptions = true;
        SET_PROPERTY(turboOptions, enabled, legacyAddonOptions.TurboInputEnabled);
        SET_PROPERTY(turboOptions, buttonPin, bytePinToIntPin(legacyAddonOptions.pinButtonTurbo));
        SET_PROPERTY(turboOptions, ledPin, bytePinToIntPin(legacyAddonOptions.pinTurboLED));
        SET_PROPERTY(turboOptions, shotCount, legacyAddonOptions.turboShotCount);
        SET_PROPERTY(turboOptions, shmupDialPin, bytePinToIntPin(legacyAddonOptions.pinShmupDial));
        SET_PROPERTY(turboOptions, shmupModeEnabled, legacyAddonOptions.shmupMode);
        SET_PROPERTY(turboOptions, shmupAlwaysOn1, legacyAddonOptions.shmupAlwaysOn1);
        SET_PROPERTY(turboOptions, shmupAlwaysOn2, legacyAddonOptions.shmupAlwaysOn2);
        SET_PROPERTY(turboOptions, shmupAlwaysOn3, legacyAddonOptions.shmupAlwaysOn3);
        SET_PROPERTY(turboOptions, shmupAlwaysOn4, legacyAddonOptions.shmupAlwaysOn4);
        SET_PROPERTY(turboOptions, shmupBtn1Pin, bytePinToIntPin(legacyAddonOptions.pinShmupBtn1));
        SET_PROPERTY(turboOptions, shmupBtn2Pin, bytePinToIntPin(legacyAddonOptions.pinShmupBtn2));
        SET_PROPERTY(turboOptions, shmupBtn3Pin, bytePinToIntPin(legacyAddonOptions.pinShmupBtn3));
        SET_PROPERTY(turboOptions, shmupBtn4Pin, bytePinToIntPin(legacyAddonOptions.pinShmupBtn4));
        SET_PROPERTY(turboOptions, shmupBtnMask1, legacyAddonOptions.shmupBtnMask1);
        SET_PROPERTY(turboOptions, shmupBtnMask2, legacyAddonOptions.shmupBtnMask2);
        SET_PROPERTY(turboOptions, shmupBtnMask3, legacyAddonOptions.shmupBtnMask3);
        SET_PROPERTY(turboOptions, shmupBtnMask4, legacyAddonOptions.shmupBtnMask4);
        if (legacyAddonOptions.shmupMixMode == 0 || legacyAddonOptions.shmupMixMode == 1)
        {
            SET_PROPERTY(turboOptions, shmupMixMode, static_cast<ShmupMixMode>(legacyAddonOptions.shmupMixMode));
        }

        WiiOptions& wiiOptions = config.addonOptions.wiiOptions;
        config.addonOptions.has_wiiOptions = true;
        SET_PROPERTY(wiiOptions, enabled, legacyAddonOptions.WiiExtensionAddonEnabled);
        SET_PROPERTY(wiiOptions, i2cBlock, legacyAddonOptions.wiiExtensionBlock);
        SET_PROPERTY(wiiOptions, i2cSDAPin, bytePinToIntPin(legacyAddonOptions.wiiExtensionSDAPin));
        SET_PROPERTY(wiiOptions, i2cSCLPin, bytePinToIntPin(legacyAddonOptions.wiiExtensionSCLPin));
        SET_PROPERTY(wiiOptions, i2cSpeed, legacyAddonOptions.wiiExtensionSpeed);
        
        PS4Options& ps4Options = config.addonOptions.ps4Options;
        config.addonOptions.has_ps4Options = true;
        SET_PROPERTY(ps4Options, enabled, legacyAddonOptions.PS4ModeAddonEnabled);
    }

    const ConfigLegacy::PS4Options& legacyPS4Options = *reinterpret_cast<ConfigLegacy::PS4Options*>(EEPROM_ADDRESS_START + PS4_STORAGE_INDEX);
    if (legacyPS4Options.checksum == NOCHECKSUM_MAGIC)
    {
        legacyConfigFound = true;

        config.has_addonOptions = true;

        PS4Options& ps4Options = config.addonOptions.ps4Options;
        config.addonOptions.has_ps4Options = true;
        SET_PROPERTY_BYTES(ps4Options, serial, legacyPS4Options.serial);
        SET_PROPERTY_BYTES(ps4Options, signature, legacyPS4Options.signature);
        SET_PROPERTY_BYTES(ps4Options, rsaN, legacyPS4Options.rsa_n);
        SET_PROPERTY_BYTES(ps4Options, rsaE, legacyPS4Options.rsa_e);
        SET_PROPERTY_BYTES(ps4Options, rsaD, legacyPS4Options.rsa_d);
        SET_PROPERTY_BYTES(ps4Options, rsaP, legacyPS4Options.rsa_p);
        SET_PROPERTY_BYTES(ps4Options, rsaQ, legacyPS4Options.rsa_q);
        SET_PROPERTY_BYTES(ps4Options, rsaDP, legacyPS4Options.rsa_dp);
        SET_PROPERTY_BYTES(ps4Options, rsaDQ, legacyPS4Options.rsa_dq);
        SET_PROPERTY_BYTES(ps4Options, rsaQP, legacyPS4Options.rsa_qp);
        SET_PROPERTY_BYTES(ps4Options, rsaRN, legacyPS4Options.rsa_rn);
    }

    const ConfigLegacy::SplashImage& legacySplashImage = *reinterpret_cast<ConfigLegacy::SplashImage*>(EEPROM_ADDRESS_START + SPLASH_IMAGE_STORAGE_INDEX);
    if (legacySplashImage.checksum == computeChecksum(reinterpret_cast<const char*>(&legacySplashImage), sizeof(ConfigLegacy::SplashImage), offsetof(ConfigLegacy::SplashImage, checksum)))
    {
        legacyConfigFound = true;

        DisplayOptions& displayOptions = config.displayOptions;
        config.has_displayOptions = true;
        SET_PROPERTY_BYTES(displayOptions, splashImage, legacySplashImage.data);
    }

    return legacyConfigFound;
}
