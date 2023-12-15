#ifndef XID_DUKE_H_
#define XID_DUKE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>

/* Digital Button Masks */
#define XID_DUP (1 << 0)
#define XID_DDOWN (1 << 1)
#define XID_DLEFT (1 << 2)
#define XID_DRIGHT (1 << 3)
#define XID_START (1 << 4)
#define XID_BACK (1 << 5)
#define XID_LS (1 << 6)
#define XID_RS (1 << 7)

typedef struct __attribute__((packed))
{
    uint8_t zero;
    uint8_t bLength;
    uint8_t dButtons;
    uint8_t reserved;
    uint8_t A;
    uint8_t B;
    uint8_t X;
    uint8_t Y;
    uint8_t BLACK;
    uint8_t WHITE;
    uint8_t L;
    uint8_t R;
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;
} USB_XboxGamepad_InReport_t;

typedef struct __attribute__((packed))
{
    uint8_t zero;
    uint8_t bLength;
    uint16_t lValue;
    uint16_t rValue;
} USB_XboxGamepad_OutReport_t;

#define TUD_XID_DUKE_DESC_LEN  (9+7+7)

#define TUD_XID_DUKE_DESCRIPTOR(_itfnum, _epout, _epin) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, XID_INTERFACE_CLASS, XID_INTERFACE_SUBCLASS, 0x00, 0x00,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(32), 4, \
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(32), 4

static const uint8_t DUKE_DESC_XID[] = {
    0x10,
    0x42,
    0x00, 0x01,
    0x01,
    0x02,
    0x14,
    0x06,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t DUKE_CAPABILITIES_IN[] = {
    0x00,
    0x14,
    0xFF,
    0x00,
    0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
};

static const uint8_t DUKE_CAPABILITIES_OUT[] = {
    0x00,
    0x06,
    0xFF, 0xFF, 0xFF, 0xFF
};

#ifdef __cplusplus
}
#endif

#endif