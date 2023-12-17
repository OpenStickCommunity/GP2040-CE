/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 OpenStickCommunity (gp2040-ce.info)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb_option.h"

#if (CFG_TUH_ENABLED && CFG_TUH_XINPUT)

#include "hardware/structs/usb.h"

#include "host/usbh.h"
#include "host/usbh_pvt.h"
#include "xinput_host.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+

typedef struct
{
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t type;
    uint8_t subtype;

    uint16_t epin_size;
    uint16_t epout_size;

    uint8_t epin_buf[CFG_TUH_XINPUT_EPIN_BUFSIZE];
    uint8_t epout_buf[CFG_TUH_XINPUT_EPOUT_BUFSIZE];
} xinputh_interface_t;

typedef struct
{
    uint8_t inst_count;
    xinputh_interface_t instances[CFG_TUH_XINPUT];
} xinputh_device_t;
static xinputh_device_t _xinputh_dev[CFG_TUH_DEVICE_MAX];

//------------- Internal prototypes -------------//

// Get HID device & interface
TU_ATTR_ALWAYS_INLINE static inline xinputh_device_t *get_dev(uint8_t dev_addr);
TU_ATTR_ALWAYS_INLINE static inline xinputh_interface_t *get_instance(uint8_t dev_addr, uint8_t instance);
static uint8_t get_instance_id_by_itfnum(uint8_t dev_addr, uint8_t itf);
static uint8_t get_instance_id_by_epaddr(uint8_t dev_addr, uint8_t ep_addr);

//--------------------------------------------------------------------+
// Interface API
//--------------------------------------------------------------------+

uint8_t tuh_xinput_instance_count(uint8_t dev_addr) {
    return get_dev(dev_addr)->inst_count;
}

bool tuh_xinput_mounted(uint8_t dev_addr, uint8_t instance) {
    if (get_dev(dev_addr)->inst_count < instance) return false;
    xinputh_interface_t *hid_itf = get_instance(dev_addr, instance);
    return (hid_itf->ep_in != 0) || (hid_itf->ep_out != 0);
}

//--------------------------------------------------------------------+
// Interrupt Endpoint API
//--------------------------------------------------------------------+

bool tuh_xinput_receive_report(uint8_t dev_addr, uint8_t instance) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);

    // claim endpoint
    TU_VERIFY(usbh_edpt_claim(dev_addr, xid_itf->ep_in));

    if (!usbh_edpt_xfer(dev_addr, xid_itf->ep_in, xid_itf->epin_buf, xid_itf->epin_size)) {
        usbh_edpt_release(dev_addr, xid_itf->ep_in);
        return false;
    }

    return true;
}

bool tuh_xinput_send_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);

    bool ret = false;

    // claim endpoint
    TU_ASSERT(len <= xid_itf->epout_size);
    bool tuh_rdy = tuh_ready(dev_addr);
    bool edpt_busy = usbh_edpt_busy(dev_addr, xid_itf->ep_out);
    if (tuh_rdy &&
        (xid_itf->ep_out != 0) && (!edpt_busy)) {
        TU_VERIFY(usbh_edpt_claim(dev_addr, xid_itf->ep_out));
        memcpy(xid_itf->epout_buf, report, len);
        if (!usbh_edpt_xfer(dev_addr, xid_itf->ep_out, xid_itf->epout_buf, len)) {
            usbh_edpt_release(dev_addr, xid_itf->ep_out);
            ret = false;
        }
        ret = true;

    } 

    return ret;
}

bool tuh_xinput_ready(uint8_t dev_addr, uint8_t instance) {
    TU_VERIFY(tuh_xinput_mounted(dev_addr, instance));

    xinputh_interface_t *hid_itf = get_instance(dev_addr, instance);
    return !usbh_edpt_busy(dev_addr, hid_itf->ep_in);
}

//--------------------------------------------------------------------+
// USBH API
//--------------------------------------------------------------------+
void xinputh_init(void) {
    tu_memclr(_xinputh_dev, sizeof(_xinputh_dev));
}

bool xinputh_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void)result;

    uint8_t const dir = tu_edpt_dir(ep_addr);
    uint8_t const instance = get_instance_id_by_epaddr(dev_addr, ep_addr);
    xinputh_interface_t *xinput_itf = get_instance(dev_addr, instance);

    if (dir == TUSB_DIR_IN) {
        tuh_xinput_report_received_cb(dev_addr, instance, xinput_itf->epin_buf, (uint16_t)xferred_bytes);
        
        // Is this double sending?
        usbh_edpt_xfer(dev_addr, xinput_itf->ep_in, xinput_itf->epin_buf, xinput_itf->epin_size);
    } else {
        if (tuh_xinput_report_sent_cb)
            tuh_xinput_report_sent_cb(dev_addr, instance, xinput_itf->epout_buf, xferred_bytes);
    }

    return true;
}

void xinputh_close(uint8_t dev_addr) {
    TU_VERIFY(dev_addr <= CFG_TUH_DEVICE_MAX, );
    xinputh_device_t *hid_dev = get_dev(dev_addr);
    if (tuh_xinput_umount_cb) {
        for (uint8_t inst = 0; inst < hid_dev->inst_count; inst++) tuh_xinput_umount_cb(dev_addr, inst);
    }

    tu_memclr(hid_dev, sizeof(xinputh_device_t));
}

//--------------------------------------------------------------------+
// Enumeration
//--------------------------------------------------------------------+
typedef enum
{
    UNKNOWN = 0,
    XBOX360,
    XBOXONE,
} xinput_type_t;

bool xinputh_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    (void)rhport;
    (void)max_len;
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == desc_itf->bInterfaceClass || TUSB_CLASS_HID == desc_itf->bInterfaceClass, 0);
    xinputh_interface_t *p_xinput = NULL;
    for (uint8_t i = 0; i < CFG_TUH_XINPUT; i++) {
        xinputh_interface_t *xid_itf = get_instance(dev_addr, i);
        if (xid_itf->ep_in == 0 && xid_itf->ep_out == 0) {
            p_xinput = xid_itf;
            break;
        }
    }
    TU_VERIFY(p_xinput, 0);
    uint8_t const *p_desc = (uint8_t const *)desc_itf;
    if (desc_itf->bInterfaceSubClass == 0x47 &&
               desc_itf->bInterfaceProtocol == 0xD0 && desc_itf->bNumEndpoints) {
        uint8_t endpoints = desc_itf->bNumEndpoints;
        while (endpoints--) {
            p_desc = tu_desc_next(p_desc);
            tusb_desc_endpoint_t const *desc_ep =
                (tusb_desc_endpoint_t const *)p_desc;
            TU_ASSERT(TUSB_DESC_ENDPOINT == desc_ep->bDescriptorType);
            if (desc_ep->bEndpointAddress & 0x80) {
                p_xinput->ep_in = desc_ep->bEndpointAddress;
                p_xinput->epin_size = tu_edpt_packet_size(desc_ep);
                TU_ASSERT(tuh_edpt_open(dev_addr, desc_ep));
            } else {
                p_xinput->ep_out = desc_ep->bEndpointAddress;
                p_xinput->epout_size = tu_edpt_packet_size(desc_ep);
                TU_ASSERT(tuh_edpt_open(dev_addr, desc_ep));
            }
        }
        p_xinput->itf_num = desc_itf->bInterfaceNumber;
        p_xinput->type = XBOXONE;

        _xinputh_dev->inst_count++;
        usbh_edpt_xfer(dev_addr, p_xinput->ep_in, p_xinput->epin_buf, p_xinput->epin_size);
        return true;
    } 
    return false;
}

//--------------------------------------------------------------------+
// Set Configure
//--------------------------------------------------------------------+
static void config_driver_mount_complete(uint8_t dev_addr, uint8_t instance);
static void process_set_config(tuh_xfer_t *xfer);

bool xinputh_set_config(uint8_t dev_addr, uint8_t itf_num) {
    uint8_t instance = get_instance_id_by_itfnum(dev_addr, itf_num);
    config_driver_mount_complete(dev_addr, instance);
    return true;
}

static void config_driver_mount_complete(uint8_t dev_addr, uint8_t instance) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);
    
    // enumeration is complete
    tuh_xinput_mount_cb(dev_addr, instance, xid_itf->type, xid_itf->subtype);

    // notify usbh that driver enumeration is complete
    usbh_driver_set_config_complete(dev_addr, xid_itf->itf_num);
}

//--------------------------------------------------------------------+
// Helper
//--------------------------------------------------------------------+

// Get Device by address
TU_ATTR_ALWAYS_INLINE static inline xinputh_device_t *get_dev(uint8_t dev_addr) {
    return &_xinputh_dev[dev_addr - 1];
}

// Get Interface by instance number
TU_ATTR_ALWAYS_INLINE static inline xinputh_interface_t *get_instance(uint8_t dev_addr, uint8_t instance) {
    return &_xinputh_dev[dev_addr - 1].instances[instance];
}

// Get instance ID by interface number
static uint8_t get_instance_id_by_itfnum(uint8_t dev_addr, uint8_t itf) {
    for (uint8_t inst = 0; inst < CFG_TUH_XINPUT; inst++) {
        xinputh_interface_t *hid = get_instance(dev_addr, inst);
        if ((hid->itf_num == itf)) return inst;
    }
    return 0xff;
}

// Get instance ID by endpoint address
static uint8_t get_instance_id_by_epaddr(uint8_t dev_addr, uint8_t ep_addr) {
    for (uint8_t inst = 0; inst < CFG_TUH_XINPUT; inst++) {
        xinputh_interface_t *hid = get_instance(dev_addr, inst);
        if ((ep_addr == hid->ep_in) || (ep_addr == hid->ep_out)) return inst;
    }
    return 0xff;
}

#endif
