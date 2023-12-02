/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "xbone_driver.h"
#include "gamepad/descriptors/XBOneDescriptors.h"

#include "system.h"

#define ENDPOINT_SIZE 64
#define VENDOR_EPSIZE 64

#define CFG_TUD_XBONE 8
#define CFG_TUD_VENDOR_TX_BUFSIZE VENDOR_EPSIZE
#define CFG_TUD_VENDOR_RX_BUFSIZE VENDOR_EPSIZE
#define CFG_TUD_XINPUT_TX_BUFSIZE 64
#define CFG_TUD_XINPUT_RX_BUFSIZE 64

#define USB_SETUP_DEVICE_TO_HOST 0x80
#define USB_SETUP_HOST_TO_DEVICE 0x00
#define USB_SETUP_TYPE_VENDOR    0x40
#define USB_SETUP_TYPE_CLASS     0x20
#define USB_SETUP_TYPE_STANDARD  0x00
#define USB_SETUP_RECIPIENT_INTERFACE   0x01
#define USB_SETUP_RECIPIENT_DEVICE      0x00
#define USB_SETUP_RECIPIENT_ENDPOINT    0x02
#define USB_SETUP_RECIPIENT_OTHER       0x03

#define REQ_GET_OS_FEATURE_DESCRIPTOR 0x20
#define DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR 0x0004
#define DESC_EXTENDED_PROPERTIES_DESCRIPTOR 0x0005
#define REQ_GET_XGIP_HEADER 0x90

static bool sending=false;
static bool waiting_ack=false;
uint8_t xbone_out_buffer[XBONE_OUT_SIZE] = {};
uint32_t timer_wait_for_auth = 0;
uint32_t xbox_one_powered_on = false;
uint32_t keep_alive_timer = 0;
uint8_t keep_alive_sequence = 0;

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;         // optional Out endpoint
    uint8_t boot_protocol;  // Boot mouse or keyboard
    bool boot_mode;         // default = false (Report)

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_XINPUT_TX_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_XINPUT_RX_BUFSIZE];
} xinputd_interface_t;

CFG_TUSB_MEM_SECTION static xinputd_interface_t _xinputd_itf[CFG_TUD_XBONE];
static inline uint8_t get_index_by_itfnum(uint8_t itf_num) {
    for (uint8_t i = 0; i < CFG_TUD_XBONE; i++) {
        if (itf_num == _xinputd_itf[i].itf_num) return i;
    }

    return 0xFF;
}

typedef struct {
	uint8_t idx;
	uint8_t seq;
	uint16_t sent;
} descriptor_handler;

static descriptor_handler descriptor;

// Announce with random micros() call
/*
// Santroller
const uint8_t announce[] = {
    0x7e, 0xed, 0x8d, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x5e, 0x04, 0xea, 0x02, 0x05, 0x00, 0x11, 0x00,
	0x82, 0x0c, 0x00, 0x00, 0x04, 0x05, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00};*/

/*
// Magic-X
const uint8_t announce[] = {
    0x00, 0x2a, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 
	0xdf, 0x33, 0x14, 0x00, 0x01, 0x00, 0x01, 0x00, 
	0x17, 0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 
	0x01, 0x00, 0x01, 0x00};
*/

// MagicBoots XBOne
const uint8_t announce[] = {
	0x19, 0x17, 0x23, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x79, 0x00, 0x94, 0x18, 0x01, 0x00, 0x01, 0x00,
	0x17, 0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00,
};

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

/*
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
*/

// MagicBootS Descriptor:
const uint8_t xboxOneDescriptor[] = {
	0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0x00,
	0x7b, 0x00, 0x16, 0x00, 0x1f, 0x00, 0x20, 0x00,
	0x27, 0x00, 0x2d, 0x00, 0x4a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
	0x06, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x05,
	0x01, 0x04, 0x05, 0x06, 0x0a, 0x01, 0x1a, 0x00,
	0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x73, 0x2e,
	0x58, 0x62, 0x6f, 0x78, 0x2e, 0x49, 0x6e, 0x70,
	0x75, 0x74, 0x2e, 0x47, 0x61, 0x6d, 0x65, 0x70,
	0x61, 0x64, 0x03, 0x56, 0xff, 0x76, 0x97, 0xfd,
	0x9b, 0x81, 0x45, 0xad, 0x45, 0xb6, 0x45, 0xbb,
	0xa5, 0x26, 0xd6, 0x2c, 0x40, 0x2e, 0x08, 0xdf,
	0x07, 0xe1, 0x45, 0xa5, 0xab, 0xa3, 0x12, 0x7a,
	0xf1, 0x97, 0xb5, 0xe7, 0x1f, 0xf3, 0xb8, 0x86,
	0x73, 0xe9, 0x40, 0xa9, 0xf8, 0x2f, 0x21, 0x26,
	0x3a, 0xcf, 0xb7, 0x02, 0x17, 0x00, 0x20, 0x0e,
	0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x17, 0x00, 0x09, 0x3c, 0x00,
	0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

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


typedef struct {
    uint32_t TotalLength;
    uint16_t Version;
    uint16_t Index;
    uint8_t TotalSections;
    uint8_t Reserved[7];
    uint8_t FirstInterfaceNumber;
    uint8_t Reserved2;
    uint8_t CompatibleID[8];
    uint8_t SubCompatibleID[8];
    uint8_t Reserved3[6];
} __attribute__((packed)) OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE;

const OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE DevCompatIDsOne = {
    TotalLength : sizeof(OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE),
    Version : 0x0100,
    Index : DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR,
    TotalSections : 1,
    Reserved : {0},
    Reserved2 : 0x01,
    CompatibleID : {'X','G','I','P','1','0',0,0},
    SubCompatibleID : {0},
    Reserved3 : {0}
};

void changeState(XboxOneState newState) {
	printf("Changing State from %u to %u\r\n", XBOneData::getInstance().xboneState, newState);
	XBOneData::getInstance().xboneState = newState;
}

static void xbone_reset(uint8_t rhport) {
	printf("xboxone_reset\r\n");
	(void)rhport;

	sending = false;
	memset(&descriptor, 0, sizeof(descriptor_handler));
	changeState(XboxOneState::reset_state);

	timer_wait_for_auth = to_ms_since_boot(get_absolute_time());
	keep_alive_timer = to_ms_since_boot(get_absolute_time());
	xbox_one_powered_on = false;

	// close any endpoints that are open
	tu_memclr(&_xinputd_itf, sizeof(_xinputd_itf));
}

static void xbone_init(void) {
	printf("xbone_init\r\n");
	// DEBUG our announce just inputs our ticks
	// uint32_serial = micros();
	// memcpy(&combined_report, announce, sizeof(announce));
    // memcpy(&combined_report.raw[7], &serial, 3);   serial = micros??
	xbone_reset(TUD_OPT_RHPORT);
}

static uint16_t xbone_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
	printf("Xboxone_Open %u\r\n", max_len);
	uint16_t drv_len = 0;
    if (TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass) {
        TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass, 0);
		
		drv_len = sizeof(tusb_desc_interface_t) +
                  (itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_len >= drv_len, 0);

        // Find available interface
        xinputd_interface_t *p_xinput = NULL;
        for (uint8_t i = 0; i < CFG_TUD_XBONE; i++) {
            if (_xinputd_itf[i].ep_in == 0 && _xinputd_itf[i].ep_out == 0) {
				p_xinput = &_xinputd_itf[i];
                break;
            }
        }

        TU_VERIFY(p_xinput, 0);
		uint8_t const *p_desc = (uint8_t const *)itf_desc;

        if (itf_desc->bInterfaceSubClass == 0x47 &&
                   itf_desc->bInterfaceProtocol == 0xD0) {
			p_desc = tu_desc_next(p_desc);
            TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, itf_desc->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_xinput->ep_out, &p_xinput->ep_in), 0);

            p_xinput->itf_num = itf_desc->bInterfaceNumber;

            // Prepare for output endpoint
            if (p_xinput->ep_out) {
                if (!usbd_edpt_xfer(rhport, p_xinput->ep_out, p_xinput->epout_buf, sizeof(p_xinput->epout_buf))) {
                    TU_LOG_FAILED();
                    TU_BREAKPOINT();
                }
            }
		}
    }

    return drv_len;
}

static bool xbone_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    return true;
}

static bool xbone_control_complete(uint8_t rhport, tusb_control_request_t const *request)
{
	(void)rhport;
	(void)request;

	return true;
}

bool xbone_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result,
                     uint32_t xferred_bytes) {
    (void)result;

	printf("xbone_xfer_cb called:\r\n");

    uint8_t itf = 0;
    xinputd_interface_t *p_xinput = _xinputd_itf;

    for (;; itf++, p_xinput++) {
        if (itf >= TU_ARRAY_SIZE(_xinputd_itf)) return false;

        if (ep_addr == p_xinput->ep_out || ep_addr == p_xinput->ep_in) break;
    }

    if (ep_addr == p_xinput->ep_out) {
		for(uint32_t i=0; i < xferred_bytes; i++)
		{
			if (i%16 == 0) printf("\r\n  ");
			printf("%02X ", p_xinput->epout_buf[i]);
		}
		printf("\r\n");

		// This is how Santroller moves data between console & controller
		GipHeader_t header;
		memcpy(&header, p_xinput->epout_buf, sizeof(GipHeader_t));
		if ( header.command == GIP_DEVICE_DESCRIPTOR && header.length == 0) {
			printf("RESET: Xbox One asking for GIP Descriptors!!!\r\n");
			changeState(XboxOneState::send_descriptor);
			descriptor.idx = 0;
			descriptor.seq = header.sequence;
			descriptor.sent = 0;
			sending = false;
			waiting_ack = false;
		} else if ( header.command == GIP_ACK_REQUEST ) {
			printf("Got an ack! header.sequence %u   descriptor.seq %u\r\n", header.sequence, descriptor.seq);
			if ( header.sequence == descriptor.seq ) { 
				waiting_ack = false;
			}
			sending = false;
		} else if ( header.command == GIP_POWER_MODE_DEVICE_CONFIG ) {
			printf("Got a power-up request!\r\n");
			xbox_one_powered_on = true;
		} else {
			printf("Got some other request\r\n");
		}
        //hid_set_report(p_xinput->epout_buf, xferred_bytes, 0x00, INTERRUPT_ID);
        TU_ASSERT(usbd_edpt_xfer(rhport, p_xinput->ep_out, p_xinput->epout_buf,
                                 sizeof(p_xinput->epout_buf)));
    } else if (ep_addr == p_xinput->ep_in) {
		printf("Finished an Ep_in request\r\n");
        sending = false;
    }
    return true;
}

bool xbone_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request) {
	uint8_t buf[255];

  	// nothing to with DATA & ACK stage
  	if (stage != CONTROL_STAGE_SETUP)
		return true;

	if (request->bmRequestType_bit.direction == TUSB_DIR_IN) { // This is where we should be
		uint16_t len = request->wLength;
		if ( request->bmRequestType == (USB_SETUP_DEVICE_TO_HOST | USB_SETUP_RECIPIENT_DEVICE | USB_SETUP_TYPE_VENDOR) && request->bRequest == REQ_GET_OS_FEATURE_DESCRIPTOR &&
			request->wIndex == DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR) {
			memcpy(buf, &DevCompatIDsOne, len);
			// DEBUG
			printf("Sizeof DevCompatIDsOne: %u\r\n", sizeof(DevCompatIDsOne));
			printf("XGIP10 Request: ");
			for(uint32_t i=0; i < len; i++)
			{
				if (i%16 == 0) printf("\r\n  ");
				printf("%02X ", buf[i]);
			}
			printf("\r\n");
		}
		tud_control_xfer(rhport, request, (void*)buf, len);
	} else {
		tud_control_xfer(rhport, request, (void*)buf, request->wLength);
	}
	return true;
}

bool send_xbone_report(void *report, uint8_t report_size) {
	bool sent = false;

    uint8_t itf = 0;
    xinputd_interface_t *p_xinput = _xinputd_itf;

	for (;; itf++, p_xinput++) {
        if (itf >= TU_ARRAY_SIZE(_xinputd_itf)) {
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

void receive_xbone_report(void) {
    uint8_t itf = 0;
    xinputd_interface_t *p_xinput = _xinputd_itf;

	for (;; itf++, p_xinput++) {
        if (itf >= TU_ARRAY_SIZE(_xinputd_itf)) {
			return;
		}
        if (p_xinput->ep_out)
			break;
    }

	if (
		tud_ready() &&											// Is the device ready?
		(p_xinput->ep_out != 0) && (!usbd_edpt_busy(0, p_xinput->ep_out)) // Is the IN endpoint available?
	)
	{
		usbd_edpt_claim(0, p_xinput->ep_out);								// Take control of IN endpoint
		usbd_edpt_xfer(0, p_xinput->ep_out, (uint8_t *)xbone_out_buffer, XBONE_OUT_SIZE); // Send report buffer
		usbd_edpt_release(0, p_xinput->ep_out);								// Release control of IN endpoint
	}
}

const usbd_class_driver_t xbone_driver =
	{
#if CFG_TUSB_DEBUG >= 2
		.name = "XBONE",
#endif
		.init = xbone_init,
		.reset = xbone_reset,
		.open = xbone_open,
		.control_xfer_cb = tud_vendor_control_xfer_cb,
		.xfer_cb = xbone_xfer_cb,
		.sof = NULL};

static uint8_t generateXGIPChunk(uint8_t * buf, const uint8_t * from, uint8_t cmd, uint16_t chunkIdx, uint16_t totalSize, uint8_t seq) {
	printf("Generating XIP Chunk ID: %u\r\n", chunkIdx);
	uint16_t totalSent = chunkIdx*0x3A;
	uint16_t chunkLeft = (totalSize - totalSent);
	bool isStart = (chunkIdx == 0);
	printf("Total Sent = %u\r\nChunk Left = %u\r\nisStart = %u\r\n", totalSent,chunkLeft, isStart);
	GipHeader_t device_desc;
	device_desc.command = cmd;
	device_desc.chunkStart = isStart;
	device_desc.chunked = 1;
	device_desc.internal = 1;
	device_desc.client = 0;
	device_desc.sequence = seq;
	uint16_t chunkSize = tu_min16(chunkLeft, 0x3A);
	device_desc.length = chunkSize;
	if (isStart || chunkLeft <= 0x3A) {
		device_desc.needsAck = 1; // ACK on start and last chunk
	} else if ( chunkLeft > 0x3A && totalSent < 0x100 ) {
		device_desc.length = device_desc.length | 0x80; // Set length to length | 0x80
	}
	if ( device_desc.needsAck == 1 ){
		printf("Setting global ack wait\r\n");
		waiting_ack = 1; // set global waiting for ack
	}
	memcpy(buf, &device_desc, sizeof(GipHeader_t));
	// Calculate total chunk sent which is not actual bytes sent
	
	uint16_t * chunkSizePtr = (uint16_t*)&buf[4];
	if (isStart) {
		printf("First Packet, Sending Total Chunk Size!\r\n");
		uint16_t calculatedChunkTotal = 0;
		if ( totalSize > 174 ) { // 0x3A*3
			calculatedChunkTotal = totalSize + 0x100; // 0x80*2
		} else if ( totalSize > 116) { // 0x3A*2
			calculatedChunkTotal = totalSize + 0x80;  // 0x80
		} else {
			calculatedChunkTotal = totalSize;
		}
		*chunkSizePtr = calculatedChunkTotal;
	} else {
		// if not the last packet
		if ( chunkSize == 0x3A ) {
			printf("Send total sent so far, swap if needed\r\n");
			if ( totalSent < 0x100 ) {
				buf[4] = 0x00;
				buf[5] = (uint8_t) totalSent;
			} else {
				*chunkSizePtr = totalSent;
			}
		} else {
			// If the LAST chunk, calculate based on our chunk totals, not the actual total
			printf("Last chunk! Calculate based on total sent with | 0x80 modifier\r\n");
			uint16_t calculatedChunkSent = 0;
			if ( totalSent > 116 ) { // > 0x3A*2, then we had two 0xBAs
				calculatedChunkSent = totalSent + 0x100; // 0x80*2
			} else if ( totalSent > 58 ) { // > 0x3A, then we had a 0xBA
				calculatedChunkSent = totalSent + 0x80;  // 0x80
			} else {
				calculatedChunkSent = totalSent;
			}
			*chunkSizePtr = calculatedChunkSent;
		}
	}
	printf("Chunk Size: %u\r\n", chunkSize);
	memcpy(&buf[6], &from[chunkIdx*0x3A], chunkSize);
	return chunkSize + 6; // we did not hit the end of the chunk
}

// Keep Alive Packets
void sendKeepAlive(uint32_t now ) {
	if ( xbox_one_powered_on == true && (now - keep_alive_timer) > 5000  ) {	
		printf("Sending Keep Alive\r\n");
		GipHeader_t keepAlivePacket;
		memset(&keepAlivePacket, 0, sizeof(GipHeader_t));
		keepAlivePacket.command = GIP_KEEPALIVE;
		keepAlivePacket.internal = 1;
		keep_alive_sequence++; // will rollover
		if ( keep_alive_sequence == 0 )
			keep_alive_sequence = 1;
		keepAlivePacket.sequence = keep_alive_sequence;
		keepAlivePacket.length = 4;
		uint8_t buf[8];
		memset(buf, 0, sizeof(buf));
		memcpy(buf, &keepAlivePacket, sizeof(GipHeader_t));
		buf[4] = 0x80;
		printf("Buffer: \r\n");
		for(uint32_t i = 0; i < 8; i++) {
			if ( i % 16 == 0) printf("\r\n");
			printf("%02x ", buf[i]);
		}
		printf("\r\n");
		send_xbone_report(buf, 8);
		keep_alive_timer = now;
	}
}

void tick_xbone_usb() {
	uint32_t now = to_ms_since_boot(get_absolute_time());

	// 5 second keep alive
	sendKeepAlive(now);

	XboxOneState state = XBOneData::getInstance().xboneState;
	if ( state == XboxOneState::reset_state ) {
		if ( (now - timer_wait_for_auth) > 500 ) {
			// Been longer than 5 minutes?
			//System::reboot(System::BootMode::WEBCONFIG);
				//------------- Endpoint Descriptor -------------//
			
			// Config endpoint
			printf("Setting Xbox State as ready to announce!\r\n");
			changeState(XboxOneState::ready_to_announce);
		}
	}
	if (state == ready_to_announce && sending == false ) {
		printf("Sending ready to announce!\r\n");
		GipAnnounce_t announcePacket;
		GIP_HEADER((&announcePacket), GIP_ANNOUNCE, 1, 1);
		memcpy(&announcePacket.unk1[0], announce, sizeof(announce));
		uint64_t micro = to_us_since_boot(get_absolute_time());
		memcpy(&announcePacket.serial[0], &micro, sizeof(uint8_t)*3);
		send_xbone_report(&announcePacket, sizeof(announcePacket));
		sending = true;
		changeState(idle_state);
	} else if ( state == send_descriptor && sending == false && waiting_ack == false) {
		uint8_t len = 0;
		uint8_t buf[0x40];
		printf("Total Descriptor Sent So Far %u\r\n", descriptor.sent);
		if ( descriptor.sent == sizeof(xboxOneDescriptor)) {
			printf("Descriptor is done, send back our final ack\r\n");
			// Calculate total chunk sent which is not actual bytes sent
			uint16_t calculatedChunkSize = 0;
			uint16_t bufferSize = sizeof(xboxOneDescriptor);
			if ( bufferSize > 174 ) { // 0x3A*3
				calculatedChunkSize = bufferSize + 0x100; // 0x80*2
			} else if ( bufferSize > 116) { // 0x3A*2
				calculatedChunkSize = bufferSize + 0x80;  // 0x80
			} else {
				calculatedChunkSize = bufferSize;
			}
			len = 6; // 6 bytes
			buf[0] = GIP_DEVICE_DESCRIPTOR;
			buf[1] = 0xa0; // acknowledge
			buf[2] = 0x01;
			buf[3] = 0x00; // no length
			*((uint16_t*)&buf[4]) = calculatedChunkSize;
			changeState(idle_state);
		} else {
			len = generateXGIPChunk(buf, xboxOneDescriptor, GIP_DEVICE_DESCRIPTOR, descriptor.idx, sizeof(xboxOneDescriptor), descriptor.seq);
			printf("Returned a XGIP Chunk of %u\r\n", len);
			if ( len < 0x3A ) {
				printf("Finished sending descriptor!\r\n");
			} else {
				descriptor.idx++;
			}
			descriptor.sent += (len-6); // first 6 bytes are header data
		}
		printf("Buffer: \r\n");
		for(uint32_t i = 0; i < len; i++) {
			if ( i % 16 == 0) printf("\r\n");
			printf("%02x ", buf[i]);
		}
		printf("\r\n");
		send_xbone_report(buf, len);
		sending = true;
	} else if ( state == XboxOneState::send_auth && sending == false ) {
		//printf("Waiting for auth....\r\n");
	}
}
