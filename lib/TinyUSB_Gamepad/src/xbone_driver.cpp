/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "xbone_driver.h"
#include "gamepad/descriptors/XBOneDescriptors.h"

#define ENDPOINT_SIZE 64
#define VENDOR_EPSIZE 64

#define CFG_TUD_XINPUT 8
#define CFG_TUD_VENDOR_TX_BUFSIZE VENDOR_EPSIZE
#define CFG_TUD_VENDOR_RX_BUFSIZE VENDOR_EPSIZE
#define CFG_TUD_XINPUT_TX_BUFSIZE 64
#define CFG_TUD_XINPUT_RX_BUFSIZE 64

static bool sending=false;

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;         // optional Out endpoint
    uint8_t boot_protocol;  // Boot mouse or keyboard
    bool boot_mode;         // default = false (Report)

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_XINPUT_TX_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_XINPUT_RX_BUFSIZE];
} xinputd_interface_t;

CFG_TUSB_MEM_SECTION static xinputd_interface_t _xinputd_itf[CFG_TUD_XINPUT];
static inline uint8_t get_index_by_itfnum(uint8_t itf_num) {
    for (uint8_t i = 0; i < CFG_TUD_XINPUT; i++) {
        if (itf_num == _xinputd_itf[i].itf_num) return i;
    }

    return 0xFF;
}

// Announce with random micros() call
const uint8_t announce[] = {
    0x7e, 0xed, 0x8d, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x5e, 0x04, 0xea, 0x02, 0x05, 0x00, 0x11, 0x00,
	0x82, 0x0c, 0x00, 0x00, 0x04, 0x05, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00};

// Starting Chunk looks like:
// [CMD: 8 bits] [Flags: 8 bits] [Seq: 8 bits] [Chunks Length: 8 bits] [Total Chunk Size**: 16 bits] [Chunk of Length Data]

// Middle Chunk looks like:
// [CMD: 8 bits] [Flags: 8 bits] [Seq: 8 bits] [Less than 0x100 received?: 1 bit Len: 7 bits (Chunk Length)] [Packets Received 16 bits, Little-Endian if <0x100, Big-Endian if =>0x100] [Chunk of Length Data]

// End Chunk looks like:
// [CMD: 8 bits] [Flags: 8 bits] [Seq: 8 bits] [Final Length: 8 bits] [Packets Received 16 bits, Little-Endian if <0x100, Big-Endian if =>0x100] [Final Length Data]

// Total Chunk Size** = Chunks Length + (0xBA or 0x3A) * middle-chunks + Final Length
// Packets Received Example:   0x3A received, bytes are  [0x00 0x3A]      0x01AE received,  bytes are [0xAE 0x01]

// [Chunk Size] works like this:
// If chunks received > 0x100
//    chunk size = length of packet
// Else reverse size received and:
//    chunk size = chunk size | 0x80

// Magic-X Descriptor
const uint8_t xboxOneDescriptor[] = {
	0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0x00,
	0x8B, 0x00, 0x16, 0x00, 0x1F, 0x00, 0x20, 0x00,
	0x27, 0x00, 0x2D, 0x00, 0x4A, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
	0x06, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x05,
	0x01, 0x04, 0x05, 0x06, 0x0A, 0x01, 0x1A, 0x00,
	0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2E,
	0x58, 0x62, 0x6F, 0x78, 0x2E, 0x49, 0x6E, 0x70,
	0x75, 0x74, 0x2E, 0x47, 0x61, 0x6D, 0x65, 0x70,
	0x61, 0x64, 0x04, 0x56, 0xFF, 0x76, 0x97, 0xFD,
	0x9B, 0x81, 0x45, 0xAD, 0x45, 0xB6, 0x45, 0xBB,
	0xA5, 0x26, 0xD6, 0x2C, 0x40, 0x2E, 0x08, 0xDF,
	0x07, 0xE1, 0x45, 0xA5, 0xAB, 0xA3, 0x12, 0x7A,
	0xF1, 0x97, 0xB5, 0xE7, 0x1F, 0xF3, 0xB8, 0x86,
	0x73, 0xE9, 0x40, 0xA9, 0xF8, 0x2F, 0x21, 0x26,
	0x3A, 0xCF, 0xB7, 0xFE, 0xD2, 0xDD, 0xEC, 0x87,
	0xD3, 0x94, 0x42, 0xBD, 0x96, 0x1A, 0x71, 0x2E,
	0x3D, 0xC7, 0x7D, 0x02, 0x17, 0x00, 0x20, 0x20,
	0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x17, 0x00, 0x09, 0x3C, 0x00,
	0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00};

// Santroller Gamepad Descriptor
/*
const uint8_t xboxOneDescriptor[] = {
    0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x00,
	0x9d, 0x00, 0x16, 0x00, 0x1b, 0x00, 0x1c, 0x00,
	0x26, 0x00, 0x2f, 0x00, 0x4c, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05,
	0x00, 0x11, 0x00, 0x00, 0x09, 0x01, 0x02, 0x03,
	0x04, 0x06, 0x07, 0x0c, 0x0d, 0x1e, 0x08, 0x01,
	0x04, 0x05, 0x06, 0x0a, 0x0c, 0x0d, 0x1e, 0x01,
	0x1a, 0x00, 0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77,
	0x73, 0x2e, 0x58, 0x62, 0x6f, 0x78, 0x2e, 0x49,
	0x6e, 0x70, 0x75, 0x74, 0x2e, 0x47, 0x61, 0x6d,
	0x65, 0x70, 0x61, 0x64, 0x05, 0x56, 0xff, 0x76,
	0x97, 0xfd, 0x9b, 0x81, 0x45, 0xad, 0x45, 0xb6,
	0x45, 0xbb, 0xa5, 0x26, 0xd6, 0x2c, 0x40, 0x2e,
	0x08, 0xdf, 0x07, 0xe1, 0x45, 0xa5, 0xab, 0xa3,
	0x12, 0x7a, 0xf1, 0x97, 0xb5, 0xe7, 0x1f, 0xf3,
	0xb8, 0x86, 0x73, 0xe9, 0x40, 0xa9, 0xf8, 0x2f,
	0x21, 0x26, 0x3a, 0xcf, 0xb7, 0x6b, 0xe5, 0xf2,
	0x87, 0xbb, 0xc3, 0xb1, 0x49, 0x82, 0x65, 0xff,
	0xff, 0xf3, 0x77, 0x99, 0xee, 0x1e, 0x9b, 0xad,
	0x34, 0xad, 0x36, 0xb5, 0x4f, 0x8a, 0xc7, 0x17,
	0x23, 0x4c, 0x9f, 0x54, 0x6f, 0x02, 0x17, 0x00,
	0x20, 0x2c, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x09,
	0x3c, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00};
*/

static void xbone_reset(uint8_t rhport) {
	(void)rhport;
	tu_memclr(&_xinputd_itf, sizeof(_xinputd_itf));
}

static void xbone_init(void) {
	// DEBUG our announce just inputs our ticks
	// uint32_serial = micros();
	// memcpy(&combined_report, announce, sizeof(announce));
    // memcpy(&combined_report.raw[7], &serial, 3);   serial = micros??
	xbone_reset(TUD_OPT_RHPORT);
}


static uint16_t xbone_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
	 uint16_t drv_len;
    if (TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass) {
        TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass, 0);
		
		drv_len = sizeof(tusb_desc_interface_t) +
                  (itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t)) + 16;
        TU_VERIFY(max_len >= drv_len, 0);

        // Find available interface
        xinputd_interface_t *p_xinput = NULL;
        for (uint8_t i = 0; i < CFG_TUD_XINPUT; i++) {
            if (_xinputd_itf[i].ep_in == 0 && _xinputd_itf[i].ep_out == 0) {
                p_xinput = &_xinputd_itf[i];
                break;
            }
        }

        TU_VERIFY(p_xinput, 0);
        if (itf_desc->bInterfaceSubClass == 0x47 &&
                   itf_desc->bInterfaceProtocol == 0xD0) {
			uint8_t const *current_desc = tu_desc_next(itf_desc);
			uint8_t found_endpoints = 0;
			while ((found_endpoints < itf_desc->bNumEndpoints) && (drv_len <= max_len)) {
				tusb_desc_endpoint_t const *endpoint_descriptor = (tusb_desc_endpoint_t const *)current_desc;
				if (TUSB_DESC_ENDPOINT == tu_desc_type(endpoint_descriptor))
				{
					TU_ASSERT(usbd_edpt_open(rhport, endpoint_descriptor));
					if (tu_edpt_dir(endpoint_descriptor->bEndpointAddress) == TUSB_DIR_IN)
						p_xinput->ep_in = endpoint_descriptor->bEndpointAddress;
					else
						p_xinput->ep_out = endpoint_descriptor->bEndpointAddress;
					++found_endpoints;
				} else {
					printf("Not Endpoint Descriptor\r\n");
				}
				current_desc = tu_desc_next(current_desc);
			}
		}
    } else {
		printf("TUSB_CLASS_HID ??? xbone_open driver\r\n");

        TU_VERIFY(TUSB_CLASS_HID == itf_desc->bInterfaceClass, 0);

        // len = interface + hid + n*endpoints
        drv_len = sizeof(tusb_desc_interface_t) + sizeof(tusb_hid_descriptor_hid_t) + itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t);
        TU_ASSERT(max_len >= drv_len, 0);

        // Find available interface
        xinputd_interface_t *p_hid = NULL;
        uint8_t hid_id;
        for (hid_id = 0; hid_id < CFG_TUD_XINPUT; hid_id++) {
            if (_xinputd_itf[hid_id].ep_in == 0) {
                p_hid = &_xinputd_itf[hid_id];
                break;
            }
        }
        TU_ASSERT(p_hid, 0);

        uint8_t const *p_desc = (uint8_t const *)itf_desc;

        //------------- HID descriptor -------------//
        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(HID_DESC_TYPE_HID == ((tusb_hid_descriptor_hid_t const *)p_desc)->bDescriptorType, 0);

        //------------- Endpoint Descriptor -------------//
        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, itf_desc->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_hid->ep_out, &p_hid->ep_in), 0);

        p_hid->itf_num = itf_desc->bInterfaceNumber;

        // Prepare for output endpoint
        if (p_hid->ep_out) {
            if (!usbd_edpt_xfer(rhport, p_hid->ep_out, p_hid->epout_buf, sizeof(p_hid->epout_buf))) {
                TU_LOG_FAILED();
                TU_BREAKPOINT();
            }
        }
    }
    //------------- Endpoint Descriptor -------------//

    // Config endpoint
	printf("Setting Xbox State as ready to announce!");
	XBOneData::getInstance().xboneState = XboxOneState::ready_to_announce;

    return drv_len;
}

static bool xbone_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
	(void)rhport;
	(void)stage;
	(void)request;

	printf("xbone_device_control_request?\r\n");

	return true;
}

static bool xbone_control_complete(uint8_t rhport, tusb_control_request_t const *request)
{
	(void)rhport;
	(void)request;

	return true;
}

bool xbone_xfer_callback(uint8_t rhport, uint8_t ep_addr, xfer_result_t result,
                     uint32_t xferred_bytes) {
    (void)result;

    uint8_t itf = 0;
    xinputd_interface_t *p_xinput = _xinputd_itf;

	printf("xbone_xfer_callback\r\n");

	for (;; itf++, p_xinput++) {
        if (itf >= TU_ARRAY_SIZE(_xinputd_itf))
			return false;

        if (ep_addr == p_xinput->ep_out || ep_addr == p_xinput->ep_in)
			break;
    }

	if (ep_addr == p_xinput->ep_out) {
		printf("hid_set_report triggered!\r\n");
        //hid_set_report(p_xinput->epout_buf, xferred_bytes, 0x00, INTERRUPT_ID);
        //TU_ASSERT(usbd_edpt_xfer(rhport, p_xinput->ep_out, p_xinput->epout_buf,
        //                         sizeof(p_xinput->epout_buf)));
    } else if (ep_addr == p_xinput->ep_in) {
        printf("ep_addr == p_xinput->ep_in :: xbone_xfer_callback not sending anymore!\r\n");
		sending = false;
    }
    return true;
}

bool send_xbone_report(void *report, uint8_t report_size) {
	bool sent = false;

    uint8_t itf = 0;
    xinputd_interface_t *p_xinput = _xinputd_itf;

	for (;; itf++, p_xinput++) {
        if (itf >= TU_ARRAY_SIZE(_xinputd_itf)) {
			//printf("Could not find an input endpoint\r\n");
			return false;
		}
        if (p_xinput->ep_in)
			break;
    }

	if (
		tud_ready() &&											// Is the device ready?
		(p_xinput->ep_in != 0) && (!usbd_edpt_busy(0, p_xinput->ep_in)) // Is the IN endpoint available?
	)
	{
		usbd_edpt_claim(0, p_xinput->ep_in);								// Take control of IN endpoint
		usbd_edpt_xfer(0, p_xinput->ep_in, (uint8_t *)report, report_size); // Send report buffer
		usbd_edpt_release(0, p_xinput->ep_in);								// Release control of IN endpoint
		sent = true;
	}

	return sent;
}

const usbd_class_driver_t xbone_driver =
	{
#if CFG_TUSB_DEBUG >= 2
		.name = "XBONE",
#endif
		.init = xbone_init,
		.reset = xbone_reset,
		.open = xbone_open,
		.control_xfer_cb = xbone_device_control_request,
		.xfer_cb = xbone_xfer_callback,
		.sof = NULL};

void tick_xbone_usb() {
	if (XBOneData::getInstance().xboneState == ready_to_announce && sending == false ) {
		printf("Sending ready to announce!");
		GipAnnounce_t announcePacket;
		GIP_HEADER((&announcePacket), GIP_ANNOUNCE, 1, 1);
		memcpy(&announcePacket.unk1[0], announce, sizeof(announce));
		uint64_t micro = to_us_since_boot(get_absolute_time());
		memcpy(&announcePacket.serial[0], &micro, sizeof(uint8_t)*3);
		send_xbone_report(&announcePacket, sizeof(announcePacket));
		sending = true;
	}
}
