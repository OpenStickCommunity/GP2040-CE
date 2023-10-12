#ifndef _WIIExtensionAddon_H
#define _WIIExtensionAddon_H

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <stdint.h>
#include <hardware/i2c.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "WiiExtension.h"

// WiiExtension Module Name
#define WiiExtensionName "WiiExtension"

#ifndef WII_EXTENSION_ENABLED
#define WII_EXTENSION_ENABLED 0
#endif

#ifndef WII_EXTENSION_I2C_ADDR
#define WII_EXTENSION_I2C_ADDR 0x52
#endif

#ifndef WII_EXTENSION_I2C_SDA_PIN
#define WII_EXTENSION_I2C_SDA_PIN -1
#endif

#ifndef WII_EXTENSION_I2C_SCL_PIN
#define WII_EXTENSION_I2C_SCL_PIN -1
#endif

#ifndef WII_EXTENSION_I2C_BLOCK
#define WII_EXTENSION_I2C_BLOCK i2c0
#endif

#ifndef WII_EXTENSION_I2C_SPEED
#define WII_EXTENSION_I2C_SPEED 400000
#endif

#define WII_SET_MASK(bits, check, val) ((check) ? ((bits) |= (val)) : ((bits) &= ~(val)))

typedef enum {
    WII_ANALOG_TYPE_NONE,
    WII_ANALOG_TYPE_LEFT_STICK_X,
    WII_ANALOG_TYPE_LEFT_STICK_Y,
    WII_ANALOG_TYPE_RIGHT_STICK_X,
    WII_ANALOG_TYPE_RIGHT_STICK_Y,
    WII_ANALOG_TYPE_DPAD_X,
    WII_ANALOG_TYPE_DPAD_Y,
    WII_ANALOG_TYPE_LEFT_TRIGGER,
    WII_ANALOG_TYPE_RIGHT_TRIGGER,
    WII_ANALOG_TYPE_LEFT_STICK_X_PLUS,
    WII_ANALOG_TYPE_LEFT_STICK_X_MINUS,
    WII_ANALOG_TYPE_LEFT_STICK_Y_PLUS,
    WII_ANALOG_TYPE_LEFT_STICK_Y_MINUS,
    WII_ANALOG_TYPE_RIGHT_STICK_X_PLUS,
    WII_ANALOG_TYPE_RIGHT_STICK_X_MINUS,
    WII_ANALOG_TYPE_RIGHT_STICK_Y_PLUS,
    WII_ANALOG_TYPE_RIGHT_STICK_Y_MINUS,
    WII_ANALOG_TYPE_COUNT
} WiiAnalogType;

typedef struct {
    uint16_t axisType;
    uint16_t minRange;
    uint16_t maxRange;
} WiiAnalogAxis;

typedef struct {
    // button ID = gamepad mask value
    std::unordered_map<uint16_t, uint32_t> buttonMap;
    std::unordered_map<uint16_t, WiiAnalogAxis> analogMap;
} WiiExtensionConfig;

typedef struct {
    uint16_t analogInput;
    uint16_t analogValue;
} WiiAnalogChange;

class WiiExtensionInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // WiiExtension Setup
    virtual void process();     // WiiExtension Process
    virtual void preprocess() {}
    virtual std::string name() { return WiiExtensionName; }
private:
    WiiExtension * wii;
    uint32_t uIntervalMS;
    uint32_t nextTimer;

    // controller ID = config
    // defaults if no defined config
    std::unordered_map<uint16_t, WiiExtensionConfig> extensionConfigs = {
        {
            WiiExtensionController::WII_EXTENSION_NUNCHUCK,
            {
                {
                    {WiiButtons::WII_BUTTON_C,GAMEPAD_MASK_B1},
                    {WiiButtons::WII_BUTTON_Z,GAMEPAD_MASK_B2},
                },
                {
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_Y, 0, 0 }
                    },
                }
            }
        },
        {
            WiiExtensionController::WII_EXTENSION_CLASSIC,
            {
                {
                    {WiiButtons::WII_BUTTON_B, GAMEPAD_MASK_B1},
                    {WiiButtons::WII_BUTTON_A, GAMEPAD_MASK_B2},
                    {WiiButtons::WII_BUTTON_X, GAMEPAD_MASK_B4},
                    {WiiButtons::WII_BUTTON_Y, GAMEPAD_MASK_B3},
                    {WiiButtons::WII_BUTTON_L, GAMEPAD_MASK_L2},
                    {WiiButtons::WII_BUTTON_ZL, GAMEPAD_MASK_L1},
                    {WiiButtons::WII_BUTTON_R, GAMEPAD_MASK_R2},
                    {WiiButtons::WII_BUTTON_ZR, GAMEPAD_MASK_R1},
                    {WiiButtons::WII_BUTTON_MINUS, GAMEPAD_MASK_S1},
                    {WiiButtons::WII_BUTTON_PLUS, GAMEPAD_MASK_S2},
                    {WiiButtons::WII_BUTTON_HOME, GAMEPAD_MASK_A1},
                    {WiiButtons::WII_BUTTON_UP, GAMEPAD_MASK_DU},
                    {WiiButtons::WII_BUTTON_DOWN, GAMEPAD_MASK_DD},
                    {WiiButtons::WII_BUTTON_LEFT, GAMEPAD_MASK_DL},
                    {WiiButtons::WII_BUTTON_RIGHT, GAMEPAD_MASK_DR},
                },
                {
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_Y, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_Y, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_TRIGGER,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_TRIGGER, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_TRIGGER, 0, 0 }
                    },
                }
            }
        },
        {
            WiiExtensionController::WII_EXTENSION_TAIKO,
            {
                {
                    {TaikoButtons::TATA_KAT_LEFT, GAMEPAD_MASK_L2},
                    {TaikoButtons::TATA_KAT_RIGHT, GAMEPAD_MASK_R2},
                    {TaikoButtons::TATA_DON_RIGHT, GAMEPAD_MASK_B1},
                    {TaikoButtons::TATA_DON_LEFT, GAMEPAD_MASK_DL},
                },
                {}
            }
        },
        {
            WiiExtensionController::WII_EXTENSION_GUITAR,
            {
                {
                    {GuitarButtons::GUITAR_RED, GAMEPAD_MASK_B2},
                    {GuitarButtons::GUITAR_GREEN, GAMEPAD_MASK_B1},
                    {GuitarButtons::GUITAR_YELLOW, GAMEPAD_MASK_B4},
                    {GuitarButtons::GUITAR_BLUE, GAMEPAD_MASK_B3},
                    {GuitarButtons::GUITAR_ORANGE, GAMEPAD_MASK_L2},
                    {GuitarButtons::GUITAR_PEDAL, GAMEPAD_MASK_R2},
                    {WiiButtons::WII_BUTTON_MINUS, GAMEPAD_MASK_S1},
                    {WiiButtons::WII_BUTTON_PLUS, GAMEPAD_MASK_S2},
                    {WiiButtons::WII_BUTTON_UP, GAMEPAD_MASK_DU},
                    {WiiButtons::WII_BUTTON_DOWN, GAMEPAD_MASK_DD},
                },
                {
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_Y, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_X, 0, 0 }
                    },
                }
            }
        },
        {
            WiiExtensionController::WII_EXTENSION_DRUMS,
            {
                {
                    {DrumButtons::DRUM_RED, GAMEPAD_MASK_B2},
                    {DrumButtons::DRUM_GREEN, GAMEPAD_MASK_B1},
                    {DrumButtons::DRUM_BLUE, GAMEPAD_MASK_B4},
                    {DrumButtons::DRUM_YELLOW, GAMEPAD_MASK_B3},
                    {DrumButtons::DRUM_ORANGE, GAMEPAD_MASK_L2},
                    {DrumButtons::DRUM_PEDAL, GAMEPAD_MASK_R2},
                    {WiiButtons::WII_BUTTON_MINUS, GAMEPAD_MASK_S1},
                    {WiiButtons::WII_BUTTON_PLUS, GAMEPAD_MASK_S2},
                },
                {
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_Y, 0, 0 }
                    },
                }
            }
        },
        {
            WiiExtensionController::WII_EXTENSION_TURNTABLE,
            {
                {
                    {TurntableButtons::TURNTABLE_RIGHT_GREEN, GAMEPAD_MASK_B3},
                    {TurntableButtons::TURNTABLE_RIGHT_RED, GAMEPAD_MASK_B4},
                    {TurntableButtons::TURNTABLE_RIGHT_BLUE, GAMEPAD_MASK_B2},
                    {TurntableButtons::TURNTABLE_EUPHORIA, GAMEPAD_MASK_R1},
                    {TurntableButtons::TURNTABLE_LEFT_GREEN, GAMEPAD_MASK_DL},
                    {TurntableButtons::TURNTABLE_LEFT_RED, GAMEPAD_MASK_DU},
                    {TurntableButtons::TURNTABLE_LEFT_BLUE, GAMEPAD_MASK_DR},
                    {WiiButtons::WII_BUTTON_MINUS, GAMEPAD_MASK_S1},
                    {WiiButtons::WII_BUTTON_PLUS, GAMEPAD_MASK_S2},
                },
                {
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_LEFT_STICK_Y, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_X,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_Y,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_Y, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_LEFT_TRIGGER,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_X, 0, 0 }
                    },
                    {
                        WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER,
                        { WiiAnalogType::WII_ANALOG_TYPE_RIGHT_STICK_X, 0, 0 }
                    },
                }
            }
        },
    };
    WiiExtensionConfig* currentConfig = NULL;

    bool buttonC = false;
    bool buttonZ = false;

    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool buttonL = false;
    bool buttonZL = false;
    bool buttonR = false;
    bool buttonZR = false;

    bool buttonSelect = false;
    bool buttonStart = false;
    bool buttonHome = false;

    bool dpadUp     = false;
    bool dpadDown   = false;
    bool dpadLeft   = false;
    bool dpadRight  = false;

    bool isAnalogTriggers = false;

    uint16_t triggerLeft  = 0;
    uint16_t triggerRight = 0;
    uint16_t lastTriggerLeft  = 0;
    uint16_t lastTriggerRight = 0;
    uint16_t whammyBar    = 0;

    uint16_t leftX = 0;
    uint16_t lastLeftX = 0;

    uint16_t leftY = 0;
    uint16_t lastLeftY = 0;

    uint16_t rightX = 0;
    uint16_t lastRightX = 0;

    uint16_t rightY = 0;
    uint16_t lastRightY = 0;

    std::map<uint16_t, std::vector<WiiAnalogChange>> analogChanges = {
        {WII_ANALOG_TYPE_LEFT_STICK_X,{}},
        {WII_ANALOG_TYPE_LEFT_STICK_Y,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_X,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_Y,{}},
        {WII_ANALOG_TYPE_DPAD_X,{}},
        {WII_ANALOG_TYPE_DPAD_Y,{}},
        {WII_ANALOG_TYPE_LEFT_TRIGGER,{}},
        {WII_ANALOG_TYPE_RIGHT_TRIGGER,{}},

        {WII_ANALOG_TYPE_LEFT_STICK_X_PLUS,{}},
        {WII_ANALOG_TYPE_LEFT_STICK_X_MINUS,{}},
        {WII_ANALOG_TYPE_LEFT_STICK_Y_PLUS,{}},
        {WII_ANALOG_TYPE_LEFT_STICK_Y_MINUS,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_X_PLUS,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_X_MINUS,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_Y_PLUS,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_Y_MINUS,{}},
    };

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    uint16_t bounds(uint16_t x, uint16_t out_min, uint16_t out_max);

    void update();
    void setControllerButton(uint16_t controllerID, uint16_t buttonID, uint32_t buttonMask);
    void setControllerAnalog(uint16_t controllerID, uint16_t analogID, uint32_t axisType);
    void setControllerStickMode(uint16_t controllerID, uint16_t analogID, uint32_t axisType);
    void setButtonState(bool buttonState, uint16_t buttonMask);
    void queueAnalogChange(uint16_t analogInput, uint16_t analogValue, uint16_t lastAnalogValue);
    void updateAnalogState();
    void reloadConfig();

    uint16_t getAverage(std::vector<WiiAnalogChange> const& changes);
    uint16_t getDelta(std::vector<uint16_t> const& changes, uint16_t baseValue);
};

#endif  // _WIIExtensionAddon_H
