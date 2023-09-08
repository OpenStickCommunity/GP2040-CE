// WiiNunchuk Library
// category=Signal Input/Output

#ifndef _WIIEXTENSION_H_
#define _WIIEXTENSION_H_

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "extensions/Extensions.h"

#define WII_EXTENSION_NONE -1

typedef enum {
    WII_EXTENSION_NUNCHUCK,
    WII_EXTENSION_CLASSIC,
    WII_EXTENSION_CLASSIC_PRO,
    WII_EXTENSION_GUITAR,
    WII_EXTENSION_DRUMS,
    WII_EXTENSION_TURNTABLE,
    WII_EXTENSION_TAIKO,
    WII_EXTENSION_TRAIN,
    WII_EXTENSION_DRAWSOME,
    WII_EXTENSION_UDRAW,
    WII_EXTENSION_MOTION_PLUS,
    WII_EXTENSION_COUNT
} WiiExtensionController;

#define WII_DATA_TYPE_0             0
#define WII_DATA_TYPE_1             1
#define WII_DATA_TYPE_2             2
#define WII_DATA_TYPE_3             3

#define WII_ANALOG_PRECISION_0      32
#define WII_ANALOG_PRECISION_1      64
#define WII_ANALOG_PRECISION_2      256
#define WII_ANALOG_PRECISION_3      1024

#define WII_GUITAR_UNSET            0
#define WII_GUITAR_GH3              1
#define WII_GUITAR_GHWT             2
#define WII_GUITAR_UNDEFINED        3

#define WII_GUITAR_TOUCHPAD_GREEN   0x03
#define WII_GUITAR_TOUCHPAD_RED     0x09
#define WII_GUITAR_TOUCHPAD_YELLOW  0x0E
#define WII_GUITAR_TOUCHPAD_BLUE    0x15
#define WII_GUITAR_TOUCHPAD_ORANGE  0x1B
#define WII_GUITAR_TOUCHPAD_MAX     0x1F
#define WII_GUITAR_TOUCHPAD_NONE    0x0F
#define WII_GUITAR_TOUCHPAD_OVERLAP 0x03

#ifndef HAS_WII_EXTENSION
#define HAS_WII_EXTENSION 1
#endif

#ifndef WII_EXTENSION_DEBUG
#define WII_EXTENSION_DEBUG false
#endif

#ifndef WII_EXTENSION_ENCRYPTION
// if a device acts funny with "unencrypted" mode, enable this. used in very rare cases.
#define WII_EXTENSION_ENCRYPTION false
#endif

#ifndef WII_EXTENSION_DELAY
#define WII_EXTENSION_DELAY 300
#endif

#ifndef WII_EXTENSION_TIMEOUT
#define WII_EXTENSION_TIMEOUT 2
#endif

#ifndef WII_EXTENSION_CALIBRATION
#define WII_EXTENSION_CALIBRATION true
#endif

#define WII_ALARM_NUM 0
#define WII_ALARM_IRQ TIMER_IRQ_0

#define WII_CHECKSUM_MAGIC 0x55
#define WII_CALIBRATION_SIZE 0x10
#define WII_CALIBRATION_CHECKSUM_SIZE 0x02

static volatile bool WiiExtension_alarmFired;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#define TOUCH_BETWEEN_RANGE(val,beg,end) (((val) >= ((beg)-WII_GUITAR_TOUCHPAD_OVERLAP)) && ((val) < (end)))
#define WII_DECRYPT_BYTE(x) (((x) ^ 0x17) + 0x17)

class WiiExtension {
  protected:
    uint8_t address;
  public:
    int8_t extensionType = WII_EXTENSION_NONE;
    int8_t dataType = WII_DATA_TYPE_0;

    bool isReady         = false;

    // Constructor 
    WiiExtension(int sda, int scl, i2c_inst_t *i2cCtl, int32_t speed, uint8_t addr);

    // Methods
    void begin();
    void reset();
    void start();
    void poll();

    ExtensionBase* getController() { return extensionController; };
  private:
    ExtensionBase *extensionController = NULL;

    uint8_t iSDA;
    uint8_t iSCL;
    i2c_inst_t *picoI2C;
    int32_t iSpeed;

#if WII_EXTENSION_DEBUG==true
    uint8_t _lastRead[16] = {0xFF};
#endif

    int doI2CWrite(uint8_t *pData, int iLen);
    int doI2CRead(uint8_t *pData, int iLen);
    uint8_t doI2CTest();
    void doI2CInit();

    void waitUntil_us(uint64_t us);
    static void alarmIRQ();
};

#endif
