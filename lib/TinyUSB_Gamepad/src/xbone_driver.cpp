/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "xbone_driver.h"
#include "gamepad/descriptors/XBOneDescriptors.h"

#include "system.h"

#define ENDPOINT_SIZE 64

#define CFG_TUD_XBONE 8
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

static bool waiting_ack=false;
static uint32_t waiting_ack_timeout=0;
uint8_t xbone_out_buffer[XBONE_OUT_SIZE] = {};
static uint32_t timer_wait_for_announce = 0;
static uint32_t xbox_one_powered_on = false;

// Sent report queue every 15 milliseconds
static uint32_t lastReportQueueSent = 0;
#define REPORT_QUEUE_INTERVAL 15

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;

#define XGIP_ACK_WAIT_TIMEOUT 2000

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;         // optional Out endpoint
    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_XINPUT_TX_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_XINPUT_RX_BUFSIZE];
} xboned_interface_t;

CFG_TUSB_MEM_SECTION static xboned_interface_t _xboned_itf[CFG_TUD_XBONE];
static inline uint8_t get_index_by_itfnum(uint8_t itf_num) {
    for (uint8_t i = 0; i < CFG_TUD_XBONE; i++) {
        if (itf_num == _xboned_itf[i].itf_num) return i;
    }

    return 0xFF;
}

typedef enum {
	IDLE_STATE = 0,
	READY_ANNOUNCE,
	WAIT_DESCRIPTOR_REQUEST,
	SEND_DESCRIPTOR,
	SETUP_AUTH
} XboxOneDriverState;

static XboxOneDriverState xboneDriverState;

static XGIPProtocol outgoingXGIP;
static XGIPProtocol incomingXGIP;

// Check if Auth is completed (start is 0x01, 0x01, and invalid is 0x01, 0x07)
const uint8_t authReady[] = {0x01, 0x00};

// Xbox One Announce
static uint8_t announcePacket[] = {
    0x00, 0x2a, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0xdf, 0x33, 0x14, 0x00, 0x01, 0x00, 0x01, 0x00, 
	0x17, 0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 
	0x01, 0x00, 0x01, 0x00};

// Xbox One Descriptor
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

// Windows requires a Descriptor Single for Xbox One
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

void queue_xbone_report(void *report, uint16_t report_size) {
	report_queue_t item;
	memcpy(item.report, report, report_size);
	item.len = report_size;
	report_queue.push(item);
}

void set_ack_wait() {
	waiting_ack = true;
	waiting_ack_timeout = to_ms_since_boot(get_absolute_time()); // 2 second time-out
}

static void xbone_reset(uint8_t rhport) {
	(void)rhport;
	timer_wait_for_announce = to_ms_since_boot(get_absolute_time());
	xbox_one_powered_on = false;
	while(!report_queue.empty())
		report_queue.pop();

	xboneDriverState = XboxOneDriverState::READY_ANNOUNCE;

	// close any endpoints that are open
	tu_memclr(&_xboned_itf, sizeof(_xboned_itf));
}

static void xbone_init(void) {
	xbone_reset(TUD_OPT_RHPORT);
}

static uint16_t xbone_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
	uint16_t drv_len = 0;
    if (TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass) {
        TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass, 0);
		
		drv_len = sizeof(tusb_desc_interface_t) +
                  (itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_len >= drv_len, 0);

        // Find available interface
        xboned_interface_t *p_xbone = NULL;
        for (uint8_t i = 0; i < CFG_TUD_XBONE; i++) {
            if (_xboned_itf[i].ep_in == 0 && _xboned_itf[i].ep_out == 0) {
				p_xbone = &_xboned_itf[i];
                break;
            }
        }

        TU_VERIFY(p_xbone, 0);
		uint8_t const *p_desc = (uint8_t const *)itf_desc;

		// Xbox One interface  (subclass = 0x47, protocol = 0xD0)
        if (itf_desc->bInterfaceSubClass == 0x47 &&
                   itf_desc->bInterfaceProtocol == 0xD0) {
			p_desc = tu_desc_next(p_desc);
            TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, itf_desc->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_xbone->ep_out, &p_xbone->ep_in), 0);

            p_xbone->itf_num = itf_desc->bInterfaceNumber;

            // Prepare for output endpoint
            if (p_xbone->ep_out) {
                if (!usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf, sizeof(p_xbone->epout_buf))) {
                    TU_LOG_FAILED();
                    TU_BREAKPOINT();
                }
            }
		}
    }

    return drv_len;
}

static bool xbone_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return true;
}

static bool xbone_control_complete(uint8_t rhport, tusb_control_request_t const *request) {
	(void)rhport;
	(void)request;
	return true;
}

bool xbone_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result,
                     uint32_t xferred_bytes) {
    (void)result;
    uint8_t itf = 0;
    xboned_interface_t *p_xbone = _xboned_itf;

    for (;; itf++, p_xbone++) {
        if (itf >= TU_ARRAY_SIZE(_xboned_itf)) return false;
        if (ep_addr == p_xbone->ep_out || ep_addr == p_xbone->ep_in) break;
    }

    if (ep_addr == p_xbone->ep_out) {
		// Parse incoming packet and verify its valid
		incomingXGIP.parse(p_xbone->epout_buf, xferred_bytes);

		// Setup an ack before we change anything about the incoming packet
		if ( incomingXGIP.ackRequired() == true ) {
			queue_xbone_report((uint8_t*)incomingXGIP.generateAckPacket(), incomingXGIP.getPacketLength());
		}

		uint8_t command = incomingXGIP.getCommand();
		if ( command == GIP_ACK_RESPONSE ) {
			waiting_ack = false;
		} else if ( command == GIP_DEVICE_DESCRIPTOR ) {
			// setup descriptor packet
			outgoingXGIP.reset(); // reset if anything was in there
			outgoingXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, incomingXGIP.getSequence(), 1, 1, 0);
			outgoingXGIP.setData(xboxOneDescriptor, sizeof(xboxOneDescriptor));
			xboneDriverState = XboxOneDriverState::SEND_DESCRIPTOR;
		} else if ( command == GIP_POWER_MODE_DEVICE_CONFIG || command == GIP_CMD_WAKEUP || command == GIP_CMD_RUMBLE ) {
			xbox_one_powered_on = true;
		} else if ( command == GIP_AUTH || command == GIP_FINAL_AUTH) {
			if (incomingXGIP.getDataLength() == 2 && memcmp(incomingXGIP.getData(), authReady, sizeof(authReady))==0 )
				XboxOneData::getInstance().setAuthCompleted(true);
			if ( (incomingXGIP.getChunked() == true && incomingXGIP.endOfChunk() == true) ||
					(incomingXGIP.getChunked() == false )) {
				XboxOneData::getInstance().setAuthData(incomingXGIP.getData(), incomingXGIP.getDataLength(), incomingXGIP.getSequence(),
														incomingXGIP.getCommand(), XboxOneState::send_auth_console_to_dongle);
				incomingXGIP.reset();
			}
		}

        TU_ASSERT(usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf,
                                 sizeof(p_xbone->epout_buf)));
    } else if (ep_addr == p_xbone->ep_in) {
        // Nothing needed
    }
    return true;
}

// DevCompatIDsOne sends back XGIP10 data when requested by Windows
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
		}
		tud_control_xfer(rhport, request, (void*)buf, len);
	} else {
		tud_control_xfer(rhport, request, (void*)buf, request->wLength);
	}
	return true;
}

// Send a packet to our Xbox One driver end-point
bool send_xbone_report(void *report, uint16_t report_size) {
    uint8_t itf = 0;
    xboned_interface_t *p_xbone = _xboned_itf;
	bool ret = false;
	for (;; itf++, p_xbone++) {
        if (itf >= TU_ARRAY_SIZE(_xboned_itf)) {
			return false;
		}
        if (p_xbone->ep_in)
			break;
    }
	if ( tud_ready() &&											// Is the device ready?
		(p_xbone->ep_in != 0) && (!usbd_edpt_busy(TUD_OPT_RHPORT, p_xbone->ep_in))) // Is the IN endpoint available?
	{
		usbd_edpt_claim(0, p_xbone->ep_in);										// Take control of IN endpoint
		ret = usbd_edpt_xfer(0, p_xbone->ep_in, (uint8_t *)report, report_size); 	// Send report buffer
		usbd_edpt_release(0, p_xbone->ep_in);										// Release control of IN endpoint
	}

	return ret;
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

// Update our Xbox One driver as things need to happen under-the-hood
void xbone_driver_update() {
	uint32_t now = to_ms_since_boot(get_absolute_time());

	if ( !report_queue.empty() ) {	
		if ( (now - lastReportQueueSent) > REPORT_QUEUE_INTERVAL ) {
			if ( send_xbone_report(report_queue.front().report, report_queue.front().len) ) {
				report_queue.pop();
				lastReportQueueSent = now;
			} else {
				sleep_ms(REPORT_QUEUE_INTERVAL);
			}
		}
	}

	// Do not add logic until our ACK returns
	if ( waiting_ack == true ) {
		if ((now - waiting_ack_timeout) < XGIP_ACK_WAIT_TIMEOUT) {
			return;
		} else { // ACK wait time out
			waiting_ack = false;
		}
	}

	switch(xboneDriverState) {
		case READY_ANNOUNCE:
			// Xbox One announce must wait around 0.5s before sending
			if ( now - timer_wait_for_announce > 500 ) {
				memcpy((void*)&announcePacket[3], &now, 3);
				outgoingXGIP.setAttributes(GIP_ANNOUNCE, 1, 1, 0, 0);
				outgoingXGIP.setData(announcePacket, sizeof(announcePacket));
				queue_xbone_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
				xboneDriverState = WAIT_DESCRIPTOR_REQUEST;
			}
			break;
		case SEND_DESCRIPTOR:
			queue_xbone_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
			if ( outgoingXGIP.endOfChunk() == true ) {
				xboneDriverState = SETUP_AUTH;
			}
			if ( outgoingXGIP.getPacketAck() == 1 ) { // ACK can happen at different chunks
				set_ack_wait();
			}
			break;
		case SETUP_AUTH:
			if ( XboxOneData::getInstance().getState() == XboxOneState::send_auth_dongle_to_console ) {
				bool isChunked = (XboxOneData::getInstance().getAuthLen() > GIP_MAX_CHUNK_SIZE);
				outgoingXGIP.reset();
				outgoingXGIP.setAttributes(XboxOneData::getInstance().getAuthType(), XboxOneData::getInstance().getSequence(), 1, isChunked, 1);
				outgoingXGIP.setData(XboxOneData::getInstance().getAuthBuffer(), XboxOneData::getInstance().getAuthLen());
				XboxOneData::getInstance().setState(wait_auth_dongle_to_console);
			} else if ( XboxOneData::getInstance().getState() == XboxOneState::wait_auth_dongle_to_console ) {
				queue_xbone_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
				if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true ) {
					XboxOneData::getInstance().setState(XboxOneState::auth_idle_state);
				}
				if ( outgoingXGIP.getPacketAck() == 1 ) { // ACK can happen at different chunks
					set_ack_wait();
				}
			}
			break;
		case IDLE_STATE:
		default:
			break;
	};
}
