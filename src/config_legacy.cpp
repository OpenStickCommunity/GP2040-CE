#include "config_legacy.h"
#include "config_utils.h"

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
    enum OnBoardLedMode
    {
        BOARD_LED_OFF,
        MODE_INDICATOR,
        INPUT_TEST,
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
}

bool ConfigUtils::fromLegacyStorage(Config& config)
{
    return false;
}
