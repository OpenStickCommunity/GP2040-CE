#include "xid_driver.h"

uint8_t *xremote_get_rom();

bool xremote_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request, xid_interface_t *p_xid)
{
    if (request->bmRequestType == 0xC1 && request->bRequest == 0x06 && request->wValue == 0x4200)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending REMOTE_DESC_XID\r\n");
            tud_control_xfer(rhport, request, (void *)REMOTE_DESC_XID, sizeof(REMOTE_DESC_XID));
        }
        return true;
    }
    //INFO PACKET (Interface 1)
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wIndex == 1 && request->wValue == 0x0000)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending XREMOTE INFO\r\n");
            uint8_t *rom = xremote_get_rom();
            if (rom == NULL)
            {
                return false; //STALL
            }
            tud_control_xfer(rhport, request, &rom[0], request->wLength);
        }
        return true;
    }
    //ROM DATA (Interface 1)
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x02 && request->wIndex == 1)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            uint8_t *rom = xremote_get_rom();
            if (rom == NULL)
            {
                return false; //STALL
            }
            tud_control_xfer(rhport, request, &rom[request->wValue * 1024], request->wLength);
        }
        return true;
    }

    return false;
}
