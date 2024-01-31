#include "drivers/xboxog/xid/xid.h"

bool duke_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request, xid_interface_t *p_xid);
bool steelbattalion_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request, xid_interface_t *p_xid);
bool xremote_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request, xid_interface_t *p_xid);

#define MAX_XIDS (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)

CFG_TUSB_MEM_SECTION static xid_interface_t _xid_itf[MAX_XIDS];

static inline int8_t get_index_by_itfnum(uint8_t itf_num)
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (itf_num == _xid_itf[i].itf_num)
            return i;
        //Xremote has two interfaces. Handle is separately.
        if (_xid_itf[i].type == XID_TYPE_XREMOTE)
        {
           if (itf_num == _xid_itf[i].itf_num + 1)
            return i; 
        }
    }
    return -1;
}

static inline int8_t get_index_by_ep_addr(uint8_t ep_addr)
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (ep_addr == _xid_itf[i].ep_in)
            return i;

        if (ep_addr == _xid_itf[i].ep_out)
            return i;
    }
    return -1;
}

static inline xid_interface_t *find_available_interface()
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (_xid_itf[i].ep_in == 0)
            return &_xid_itf[i];
    }
    return NULL;
}

static void xid_init(void)
{

    //#define MAX_XIDS (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)
    tu_memclr(_xid_itf, sizeof(_xid_itf));
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        _xid_itf[i].type = (i < (XID_DUKE)) ? XID_TYPE_GAMECONTROLLER :
                           (i < (XID_DUKE + XID_STEELBATTALION)) ? XID_TYPE_STEELBATTALION :
                           (i < (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)) ? XID_TYPE_XREMOTE : 0xFF;
    }
}

static void xid_reset(uint8_t rhport)
{
    xid_init();
}

static uint16_t xid_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    TU_VERIFY(itf_desc->bInterfaceClass == XID_INTERFACE_CLASS, 0);
    TU_VERIFY(itf_desc->bInterfaceSubClass == XID_INTERFACE_SUBCLASS, 0);

    xid_interface_t *p_xid = find_available_interface();
    TU_ASSERT(p_xid != NULL, 0);

    uint16_t const drv_len = (p_xid->type == XID_TYPE_GAMECONTROLLER) ? TUD_XID_DUKE_DESC_LEN :
                             (p_xid->type == XID_TYPE_STEELBATTALION) ? TUD_XID_SB_DESC_LEN :
                             (p_xid->type == XID_TYPE_XREMOTE)        ? TUD_XID_XREMOTE_DESC_LEN : 0;
    TU_ASSERT(max_len >= drv_len, 0);

    p_xid->itf_num = itf_desc->bInterfaceNumber;

    tusb_desc_endpoint_t *ep_desc;
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(itf_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_xid->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_xid->ep_out = ep_desc->bEndpointAddress);
    }

    TU_VERIFY(itf_desc->bNumEndpoints >= 2, drv_len);
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(ep_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_xid->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_xid->ep_out = ep_desc->bEndpointAddress);
    }

    return drv_len;
}

int8_t xid_get_index_by_type(uint8_t type_index, xid_type_t type)
{
    uint8_t _type_index = 0;
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (_xid_itf[i].type == type)
        {
            if (_type_index == type_index)
                return i;
            _type_index++;
        }
    }
    return -1;
}

bool xid_get_report(uint8_t index, void *report, uint16_t len)
{
    TU_VERIFY(index < MAX_XIDS, false);
    TU_VERIFY(_xid_itf[index].ep_out != 0, false);
    TU_VERIFY(len < XID_MAX_PACKET_SIZE, false);

    memcpy(report, _xid_itf[index].out, len);

    //Queue request on out endpoint
    //Most games send to control pipe, but some send to out pipe. THPSX2 atleast
    if (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, _xid_itf[index].ep_out))
    {
        usbd_edpt_xfer(TUD_OPT_RHPORT, _xid_itf[index].ep_out, _xid_itf[index].ep_out_buff, len);
    }
    return true;
}

bool xid_send_report_ready(uint8_t index)
{
    TU_VERIFY(index < MAX_XIDS, false);
    TU_VERIFY(_xid_itf[index].ep_in != 0, false);
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, _xid_itf[index].ep_in));
}
bool xid_send_report(uint8_t index, void *report, uint16_t len)
{
    TU_VERIFY(len < XID_MAX_PACKET_SIZE, false);
    TU_VERIFY(index < MAX_XIDS, false);
    TU_VERIFY(_xid_itf[index].ep_in != 0, false);
    TU_VERIFY(xid_send_report_ready(index), false);

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a local copy of the report
    memcpy(_xid_itf[index].in, report, len);

    //Send it to the host
    return usbd_edpt_xfer(TUD_OPT_RHPORT, _xid_itf[index].ep_in, _xid_itf[index].in, len);
}

static bool xid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    uint8_t index = get_index_by_ep_addr(ep_addr);

    TU_VERIFY(result == XFER_RESULT_SUCCESS, true);
    TU_VERIFY(index != -1, true);
    TU_VERIFY(xferred_bytes < XID_MAX_PACKET_SIZE, true);

    if (ep_addr == _xid_itf[index].ep_out)
    {
        memcpy(_xid_itf[index].out, _xid_itf[index].ep_out_buff, MIN(xferred_bytes, sizeof( _xid_itf[index].ep_out_buff)));
    }

    return true;
}

bool xid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    TU_VERIFY(request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);

    uint8_t index = get_index_by_itfnum((uint8_t)request->wIndex);
    TU_VERIFY(index != -1, false);

    bool ret = false;

     //Get HID Report
    if (request->bmRequestType == 0xA1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending HID report on control pipe for index %02x\n", request->wIndex);
            tud_control_xfer(rhport, request, _xid_itf[index].in, MIN(request->wLength, sizeof(_xid_itf[index].in)));
        }
        return true;
    }

    //Set HID Report
    if (request->bmRequestType == 0x21 && request->bRequest == 0x09 && request->wValue == 0x0200 && request->wLength == 0x06)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            //Host is sending a rumble command to control pipe. Queue receipt.
            tud_control_xfer(rhport, request, _xid_itf[index].ep_out_buff, MIN(request->wLength, sizeof(_xid_itf[index].ep_out_buff)));
        }
        else if (stage == CONTROL_STAGE_ACK)
        {
            //Receipt complete. Copy data to rumble struct
            TU_LOG1("Got HID report from control pipe for index %02x\n", request->wIndex);
            memcpy(_xid_itf[index].out, _xid_itf[index].ep_out_buff, MIN(request->wLength, sizeof(_xid_itf[index].out)));
        }
        return true;
    }

    switch (_xid_itf[index].type)
    {
    case XID_TYPE_GAMECONTROLLER:
        ret = duke_control_xfer(rhport, stage, request, &_xid_itf[index]);
        break;
    case XID_TYPE_STEELBATTALION:
        ret = steelbattalion_control_xfer(rhport, stage, request, &_xid_itf[index]);
        break;
    case XID_TYPE_XREMOTE:
        ret = xremote_control_xfer(rhport, stage, request, &_xid_itf[index]);
        break;
    default:
        break;
    }

    if (ret == false)
    {
        TU_LOG1("STALL: wIndex: %02x bmRequestType: %02x, bRequest: %02x, wValue: %04x\n",
                request->wIndex,
                request->bmRequestType,
                request->bRequest,
                request->wValue);
        return false;
    }

    return true;
}

static const usbd_class_driver_t xid_driver =
{
#if CFG_TUSB_DEBUG >= 2
    .name = "XID DRIVER (DUKE,SB OR XREMOTE)",
#endif
    .init = xid_init,
    .reset = xid_reset,
    .open = xid_open,
    .control_xfer_cb = xid_control_xfer_cb,
    .xfer_cb = xid_xfer_cb,
    .sof = NULL
};

const usbd_class_driver_t *xid_get_driver()
{
    return &xid_driver;
}
