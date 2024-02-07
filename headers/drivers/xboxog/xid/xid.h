#ifndef XID_H_
#define XID_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "drivers/xboxog/xid/xid_gamepad.h"
#include "drivers/xboxog/xid/xid_remote.h"
#include "drivers/xboxog/xid/xid_steelbattalion.h"

#define XID_DUKE 1
#define XID_STEELBATTALION 0
#define XID_XREMOTE 0
#define MSC_XMU 0

#define XID_INTERFACE_CLASS 0x58
#define XID_INTERFACE_SUBCLASS 0x42
#define XID_MAX_PACKET_SIZE 32

typedef enum
{
  XID_TYPE_GAMECONTROLLER,
  XID_TYPE_STEELBATTALION,
  XID_TYPE_XREMOTE,
} xid_type_t;

typedef struct
{
    uint8_t itf_num;
    xid_type_t type;
    uint8_t ep_in;
    uint8_t ep_out;
    CFG_TUSB_MEM_ALIGN uint8_t ep_out_buff[XID_MAX_PACKET_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t in[XID_MAX_PACKET_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t out[XID_MAX_PACKET_SIZE];
} xid_interface_t;

int8_t xid_get_index_by_type(uint8_t type_index, xid_type_t type);
bool xid_get_report(uint8_t index, void *report, uint16_t len);
bool xid_send_report_ready(uint8_t index);
bool xid_send_report(uint8_t index, void *report, uint16_t len);
const usbd_class_driver_t *xid_get_driver();

#ifdef __cplusplus
}
#endif

#endif //XID_H_