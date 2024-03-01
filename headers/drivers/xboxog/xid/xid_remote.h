#ifndef XID_REMOTE_H_
#define XID_REMOTE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>

#define XID_XREMOTE_ROM_CLASS 0x59

typedef enum
{
  XREMOTE_SELECT = 0x0A0B,
  XREMOTE_UP = 0x0AA6,
  XREMOTE_DOWN = 0x0AA7,
  XREMOTE_RIGHT = 0x0AA8,
  XREMOTE_LEFT = 0x0AA9,
  XREMOTE_INFO = 0x0AC3,
  XREMOTE_NINE = 0x0AC6,
  XREMOTE_EIGHT = 0x0AC7,
  XREMOTE_SEVEN = 0x0AC8,
  XREMOTE_SIX = 0x0AC9,
  XREMOTE_FIVE = 0x0ACA,
  XREMOTE_FOUR = 0x0ACB,
  XREMOTE_THREE = 0x0ACC,
  XREMOTE_TWO = 0x0ACD,
  XREMOTE_ONE = 0x0ACE,
  XREMOTE_ZERO = 0x0ACF,
  XREMOTE_DISPLAY = 0x0AD5,
  XREMOTE_BACK = 0x0AD8,
  XREMOTE_SKIP_MINUS = 0x0ADD,
  XREMOTE_SKIP_PLUS = 0x0ADF,
  XREMOTE_STOP = 0x0AE0,
  XREMOTE_REVERSE = 0x0AE2,
  XREMOTE_FORWARD = 0x0AE3,
  XREMOTE_TITLE = 0x0AE5,
  XREMOTE_PAUSE = 0x0AE6,
  XREMOTE_PLAY = 0x0AEA,
  XREMOTE_MENU = 0x0AF7,
} xremote_buttoncode_t;

typedef struct __attribute__((packed))
{
    uint8_t zero;
    uint8_t bLength;
    xremote_buttoncode_t buttonCode;
    uint16_t timeElapsed; //ms since last button press
} USB_XboxRemote_InReport_t;

#define TUD_XID_XREMOTE_DESC_LEN  (9+7+9)

#define TUD_XID_XREMOTE_DESCRIPTOR(_itfnum, _epin) \
  /* Interface 0 (HID DATA)*/\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, XID_INTERFACE_CLASS, XID_INTERFACE_SUBCLASS, 0x00, 0x00,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(8), 16, \
  /* Interface 1 (ROM DATA)*/\
  9, TUSB_DESC_INTERFACE, _itfnum + 1, 0, 0, XID_XREMOTE_ROM_CLASS, 0x00, 0x00, 0x00

static const uint8_t REMOTE_DESC_XID[] = {
    0x08,
    0x42,
    0x00, 0x01,
    0x03,
    0x00,
    0x06,
    0x00
};

#ifdef __cplusplus
}
#endif

#endif
