// WiiNunchuk Library
// category=Signal Input/Output

#ifndef _WIIEXTENSION_H_
#define _WIIEXTENSION_H_

#include <BitBang_I2C.h>

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

#ifndef HAS_WII_EXTENSION
#define HAS_WII_EXTENSION 1
#endif

#ifndef WII_EXTENSION_DEBUG
#define WII_EXTENSION_DEBUG false
#endif

#ifndef WII_EXTENSION_DELAY
#define WII_EXTENSION_DELAY 100
#endif

#ifndef WII_EXTENSION_I2C_ADDR
#define WII_EXTENSION_I2C_ADDR 0x52
#endif

#ifndef WII_EXTENSION_I2C_SDA_PIN
#define WII_EXTENSION_I2C_SDA_PIN 16
#endif

#ifndef WII_EXTENSION_I2C_SCL_PIN
#define WII_EXTENSION_I2C_SCL_PIN 17
#endif

#ifndef WII_EXTENSION_I2C_BLOCK
#define WII_EXTENSION_I2C_BLOCK i2c0
#endif

#ifndef WII_EXTENSION_I2C_SPEED
#define WII_EXTENSION_I2C_SPEED 400000
#endif

#ifndef WII_EXTENSION_CALIBRATION
#define WII_EXTENSION_CALIBRATION false
#endif

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

    int16_t whammyBar    = 0;

    bool isReady         = false;

    // Constructor 
	WiiExtension(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t iSpeed, uint8_t addr = WII_EXTENSION_I2C_ADDR);

    // Methods
    void begin();
	void reset();
  	void start();
  	void poll();
  private:
	
	BBI2C bbi2c;
	int32_t iSpeed;
	uint8_t config;
	bool singleShot;
	int data_ready;
	unsigned char uc[128];

    int8_t doWait = 0;

    uint16_t _minX1;
    uint16_t _maxX1;
    uint16_t _cenX1;

    uint16_t _minY1;
    uint16_t _maxY1;
    uint16_t _cenY1;

    uint16_t _minX2;
    uint16_t _maxX2;
    uint16_t _cenX2;

    uint16_t _minY2;
    uint16_t _maxY2;
    uint16_t _cenY2;

    uint16_t _accelX0G;
    uint16_t _accelY0G;
    uint16_t _accelZ0G;
    uint16_t _accelX1G;
    uint16_t _accelY1G;
    uint16_t _accelZ1G;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    uint16_t calibrate(uint16_t pos, uint16_t min, uint16_t max, uint16_t center);
};

#endif
