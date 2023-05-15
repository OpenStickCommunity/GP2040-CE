// WiiNunchuk Library
// category=Signal Input/Output

#ifndef _WIIEXTENSION_H_
#define _WIIEXTENSION_H_

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define WII_EXTENSION_NONE          -1
#define WII_EXTENSION_NUNCHUCK      0
#define WII_EXTENSION_CLASSIC       1
#define WII_EXTENSION_CLASSIC_PRO   2
#define WII_EXTENSION_DRAWSOME      3
#define WII_EXTENSION_GUITAR        4
#define WII_EXTENSION_DRUMS         5
#define WII_EXTENSION_TURNTABLE     6
#define WII_EXTENSION_TAIKO         7
#define WII_EXTENSION_UDRAW         8
#define WII_EXTENSION_BALANCE_BOARD 9
#define WII_EXTENSION_MOTION_PLUS   10

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

#ifndef WII_EXTENSION_DELAY
#define WII_EXTENSION_DELAY 300
#endif

#ifndef WII_EXTENSION_TIMEOUT
#define WII_EXTENSION_TIMEOUT 2
#endif

#ifndef WII_EXTENSION_CALIBRATION
#define WII_EXTENSION_CALIBRATION false
#endif

#define WII_ALARM_NUM 0
#define WII_ALARM_IRQ TIMER_IRQ_0

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

class WiiExtension {
  protected:
	uint8_t address;
  public:
    int8_t extensionType = WII_EXTENSION_NONE;
    int8_t dataType = WII_DATA_TYPE_0;

    uint16_t joy1X        = 0;
    uint16_t joy1Y        = 0;
    uint16_t joy2X        = 0;
    uint16_t joy2Y        = 0;
    uint16_t accelX       = 0;
    uint16_t accelY       = 0;
    uint16_t accelZ       = 0;

    bool buttonZ         = false;
    bool buttonC         = false;
    bool buttonZR        = false;
    bool buttonZL        = false;
    bool buttonA         = false;
    bool buttonB         = false;
    bool buttonX         = false;
    bool buttonY         = false;
    bool buttonPlus      = false;
    bool buttonHome      = false;
    bool buttonMinus     = false;
    bool buttonLT        = false;
    bool buttonRT        = false;

    bool directionUp     = false;
    bool directionDown   = false;
    bool directionLeft   = false;
    bool directionRight  = false;

    uint16_t triggerLeft  = 0;
    uint16_t triggerRight = 0;

    bool fretGreen       = false;
    bool fretRed         = false;
    bool fretYellow      = false;
    bool fretBlue        = false;
    bool fretOrange      = false;
    bool pedalButton     = false;

    uint16_t whammyBar   = 0;
    int8_t touchBar      = 0;
    bool isTouched       = 0;

    bool rimLeft         = false;
    bool rimRight        = false;
    bool drumLeft        = false;
    bool drumRight       = false;

    bool isReady         = false;

    // Constructor 
	WiiExtension(int sda, int scl, i2c_inst_t *i2cCtl, int32_t speed, uint8_t addr);

    // Methods
    void begin();
	void reset();
  	void start();
  	void poll();
  private:
	
    uint8_t iSDA;
    uint8_t iSCL;
    uint8_t bWire;
    i2c_inst_t *picoI2C;

	int32_t iSpeed;

    uint16_t _minX1 = 0;
    uint16_t _maxX1 = 1;
    uint16_t _cenX1 = 0;

    uint16_t _minY1 = 0;
    uint16_t _maxY1 = 1;
    uint16_t _cenY1 = 0;

    uint16_t _minX2 = 0;
    uint16_t _maxX2 = 1;
    uint16_t _cenX2 = 0;

    uint16_t _minY2 = 0;
    uint16_t _maxY2 = 1;
    uint16_t _cenY2 = 0;

    uint16_t _accelX0G = 0;
    uint16_t _accelY0G = 0;
    uint16_t _accelZ0G = 0;
    uint16_t _accelX1G = 0;
    uint16_t _accelY1G = 0;
    uint16_t _accelZ1G = 0;

    //uint8_t _lastRead[16];

    uint16_t _calibrationPrecision1From = WII_ANALOG_PRECISION_0;
    uint16_t _calibrationPrecision1To = WII_ANALOG_PRECISION_0;
    uint16_t _calibrationPrecision2From = WII_ANALOG_PRECISION_0;
    uint16_t _calibrationPrecision2To = WII_ANALOG_PRECISION_0;

    uint16_t _analogPrecision1From = WII_ANALOG_PRECISION_0;
    uint16_t _analogPrecision1To = WII_ANALOG_PRECISION_0;
    uint16_t _analogPrecision2From = WII_ANALOG_PRECISION_0;
    uint16_t _analogPrecision2To = WII_ANALOG_PRECISION_0;

    uint16_t _triggerPrecision1From = WII_ANALOG_PRECISION_0;
    uint16_t _triggerPrecision1To = WII_ANALOG_PRECISION_0;
    uint16_t _triggerPrecision2From = WII_ANALOG_PRECISION_0;
    uint16_t _triggerPrecision2To = WII_ANALOG_PRECISION_0;

    uint8_t _guitarType   = WII_GUITAR_UNSET;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    uint16_t calibrate(uint16_t pos, uint16_t min, uint16_t max, uint16_t center);

    int doI2CWrite(uint8_t *pData, int iLen);
    int doI2CRead(uint8_t *pData, int iLen);
    uint8_t doI2CTest();
    void doI2CInit();

    void waitUntil_us(uint64_t us);
    static void alarmIRQ();
};

#endif
