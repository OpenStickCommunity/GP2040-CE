#ifndef XID_DRIVER_H_
#define XID_DRIVER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "drivers/xboxog/xid/xid.h"

static const tusb_desc_device_t XID_DESC_DEVICE =
{
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0110,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x045E,
    .idProduct = 0x0289,
    .bcdDevice = 0x0121,

    .iManufacturer = 0x00,
    .iProduct = 0x00,
    .iSerialNumber = 0x00,

    .bNumConfigurations = 0x01
};

enum
{
#if (XID_DUKE >= 1)
  ITF_NUM_XID_DUKE,
#endif
#if (XID_STEELBATTALION >= 1)
  ITF_NUM_XID_STEELBATTALION,
#endif
#if (XID_XREMOTE >= 1)
  ITF_NUM_XID_XREMOTE,
  ITF_NUM_XID_XREMOTE_ROM,
#endif
#if (MSC_XMU >= 1)
  ITF_NUM_MSC,
#endif
  XID_ITF_NUM_TOTAL
};

#define XID_CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN) + \
    (TUD_XID_DUKE_DESC_LEN * XID_DUKE) + \
    (TUD_XID_SB_DESC_LEN * XID_STEELBATTALION) + \
    (TUD_XID_XREMOTE_DESC_LEN * XID_XREMOTE) + \
    (TUD_MSC_DESC_LEN * MSC_XMU)

static uint8_t const XID_DESC_CONFIGURATION[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, XID_ITF_NUM_TOTAL, 0, XID_CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

#if (XID_DUKE >= 1)
    TUD_XID_DUKE_DESCRIPTOR(ITF_NUM_XID_DUKE, ITF_NUM_XID_DUKE + 1, 0x80 | (ITF_NUM_XID_DUKE + 1)),
#endif

#if (XID_STEELBATTALION >= 1)
    TUD_XID_SB_DESCRIPTOR(ITF_NUM_XID_STEELBATTALION, ITF_NUM_XID_STEELBATTALION + 1, 0x80 | (ITF_NUM_XID_STEELBATTALION + 1)),
#endif

#if (XID_XREMOTE >= 1)
    TUD_XID_XREMOTE_DESCRIPTOR(ITF_NUM_XID_XREMOTE, 0x80 | (ITF_NUM_XID_XREMOTE + 1)),
#endif

#if (CFG_TUD_MSC >= 1)
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, ITF_NUM_MSC + 1, 0x80 | (ITF_NUM_MSC + 1), 64),
#endif

};

#ifdef __cplusplus
}
#endif

#endif //XID_DRIVER_H_
