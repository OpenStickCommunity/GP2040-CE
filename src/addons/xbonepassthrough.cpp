#include "addons/xbonepassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"

#include "xbone_driver.h"
#include "xgip_protocol.h"
#include "xinput_host.h"

#define XBONE_EXTENSION_DEBUG true

// unknown power-on
static uint8_t xb1_power_on[] = {0x06, 0x62, 0x45, 0xb8, 0x77, 0x26, 0x2c, 0x55,
                                 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f};

static uint8_t xb1_rumble_on[] = {0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xeb};

static uint8_t xb1_auth_on[] = {0x01,0x01};

// Data passed between PS Passthrough and TinyUSB Host callbacks

bool XBOnePassthroughAddon::available() {
#if XBONE_EXTENSION_DEBUG==true
    //printf("Check Xbox One is Enabled?\n");
#endif
    const XBOnePassthroughOptions& xboneOptions = Storage::getInstance().getAddonOptions().xbonePassthroughOptions;
	return xboneOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void XBOnePassthroughAddon::setup() {

#if XBONE_EXTENSION_DEBUG==true
    //printf("XBOne Passthrough Addon Enabled\n");
#endif

    // Packet Chunk Size set to 0
    packet_chunk_received = 0;
    ready_to_auth = false;
}

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
    uint8_t dev_addr;
    uint8_t instance;
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
	uint32_t timestamp;
} report_queue_t;

static std::queue<report_queue_t> report_queue;

void XBOnePassthroughAddon::process() {
    if ( ready_to_auth == true ) {
        if ( XboxOneData::getInstance().xboneState == XboxOneState::send_auth_console_to_dongle ) {
            //queue_host_report();
            // setup our console->dongle auth counters
            //printf("[XBOnePassthroughAddon::process] Got auth console to dongle request, sending buffer (%u)\r\n", XboxOneData::getInstance().authLen);
            authXGIP.reset();
            if ( XboxOneData::getInstance().authLen > GIP_MAX_CHUNK_SIZE ) {
                //printf("[XBOnePassthroughAddon::process] Auth buffer will be chunked\r\n");
                authXGIP.setAttributes(XboxOneData::getInstance().authType, XboxOneData::getInstance().authSequence, 1, true, 1);
                authXGIP.setData(XboxOneData::getInstance().authBuffer, XboxOneData::getInstance().authLen);
                XboxOneData::getInstance().xboneState = XboxOneState::wait_auth_console_to_dongle;
            } else {
                //printf("[XBOnePassthroughAddon::process] Auth buffer sent to dongle as one report\r\n");
                authXGIP.setAttributes(XboxOneData::getInstance().authType, XboxOneData::getInstance().authSequence, 1, false, XboxOneData::getInstance().authLen > 2);
                authXGIP.setData(XboxOneData::getInstance().authBuffer, XboxOneData::getInstance().authLen);
                queue_host_report(authXGIP.generatePacket(), authXGIP.getPacketLength());
                XboxOneData::getInstance().xboneState = XboxOneState::idle_state;
            }

/*
            // on first auth send, the Xbox does this:
            // XBONE REQUEST GIP_AUTH:  06,20,02,02,01,01    which means data[0] == 0x01 is probably auth status, and data[1] == 0x01 get ready to send me auth
            // on final auth confirm, the Xbox does this:
            // XBONE REQUEST GIP_AUTH:  06,20,02,02,01,00    which means data[0] == 0x01 is probably auth status, and data[1] == 0x00 auth done?
            // let's just try it?
            if (XboxOneData::getInstance().authSequence == 1) {
                uint8_t authStart[] = {0x01, 0x01}; // 0x01, 0x00 is finish
                authXGIP.incrementSequence();
                authXGIP.setAttributes(GIP_AUTH, authXGIP.getSequence(), 1, false, 0);
                authXGIP.setData(authStart, sizeof(authStart));
                queue_host_report(authXGIP.generatePacket(), authXGIP.getPacketLength());    
            }
*/
            //  06 30 xx 02 01 00
            /*
            if (XboxOneData::getInstance().authLen == 2) {
                printf("Check: %02X %02X\r\n", XboxOneData::getInstance().authBuffer[0], XboxOneData::getInstance().authBuffer[1]);
                 // Power-on with 0x01
                requestXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 5, 1, false, 0);
                requestXGIP.setData(xb1_auth_on, sizeof(xb1_auth_on));
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());                   
            }*/
        } else if ( XboxOneData::getInstance().xboneState == XboxOneState::wait_auth_console_to_dongle) {
            //printf("[XBOnePassthroughAddon::process] wait_auth_console_to_dongle sending chunk to Dongle\r\n");
            queue_host_report(authXGIP.generatePacket(), authXGIP.getPacketLength());
            if ( authXGIP.endOfChunk() == true ) {
                XboxOneData::getInstance().xboneState = XboxOneState::idle_state;
            }
        }
    }

    // DOn't do this unless our dongle is ready!
    if ( !report_queue.empty() ) {
		// send the first report off our queue
		//printf("[XBOnePassthroughAddon::process] Sending queued report to dongle size: %u (%u)\r\n", report_queue.front().len, report_queue.front().timestamp);
		if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, report_queue.front().report, report_queue.front().len) )
			report_queue.pop();
		else
			printf("[XBOnePassthroughAddon::process] FAILED: Keeping it on the queue to send again\r\n");
	}
}

void XBOnePassthroughAddon::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    xbone_dev_addr = dev_addr;
    xbone_instance = instance;

#if XBONE_EXTENSION_DEBUG==true
    //printf("XBOnePassthroughAddon Saved values! dev_addr %u instance %u controllerType %u subType %u\r\n", dev_addr, instance, controllerType, subtype);
#endif
}

void XBOnePassthroughAddon::unmount(uint8_t dev_addr) {
}

void XBOnePassthroughAddon::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    incomingXGIP.parse(report, len);
    if ( incomingXGIP.validate() == false ) {
        printf("[XBOnePassthroughAddon::report_received] INVALID REPORT, DROPPING\r\n");
        incomingXGIP.reset();
        return;
    }


    /*printf("[XBOnePassthroughAddon::report_received] Receive Report:\r\n");
    for(uint32_t i=0; i < len; i++)
    {
        if (i%16 == 0) printf("\r\n  ");
        printf("%02X ", ((uint8_t*)report)[i]);
    }
    printf("\r\n");*/

    switch ( incomingXGIP.getCommand() ) {
        case GIP_ACK_RESPONSE:
            // Only do something if we're waiting
            break;
        case GIP_ANNOUNCE:
            //printf("[XBOnePassthroughAddon::report_received] Dongle announced, getting descriptor packet\r\n");
            requestXGIP.reset();
            requestXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, 1, 1, false, 0);
            tuh_xinput_send_report(dev_addr, instance, (uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
            break;
        case GIP_DEVICE_DESCRIPTOR:
            if ( incomingXGIP.endOfChunk() == true ) {
                //printf("[XBOnePassthroughAddon::report_received] Done getting device descriptor, send power on signals\r\n");

                // Power-on full string
                requestXGIP.reset();
                requestXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false, 0);
                requestXGIP.setData(xb1_power_on, sizeof(xb1_power_on));
                //tuh_xinput_send_report(dev_addr, instance, (uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
                
                // Power-on with 0x01
                requestXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false, 0);
                requestXGIP.setData({0x00}, 1);
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());

                // Rumble Support to enable dongle
                requestXGIP.setAttributes(GIP_CMD_RUMBLE, 1, 0, false, 0); // not internal function
                requestXGIP.setData(xb1_rumble_on, sizeof(xb1_rumble_on));
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());

                ready_to_auth = true;
            }
            break;
        case GIP_AUTH:
        case GIP_FINAL_AUTH:
			// Save this auth to our hand-off
            //printf("[XBOnePassthroughAddon::report_received] AUTH Received\r\n");

            // if chunked == 1, internal == 1, and length = 0, this is the end of an ACK?? this happens once
           if ( incomingXGIP.getChunked() == false || incomingXGIP.endOfChunk() == true ) {
                uint8_t * buf = incomingXGIP.getData();
                uint16_t bufLen = incomingXGIP.getDataLength();
                //printf("[XBOnePassthroughAddon::report_received]: Finished chunk, loading up %04x bytes to send to console\r\n", bufLen);
                memcpy(XboxOneData::getInstance().authBuffer, buf, bufLen);
                XboxOneData::getInstance().authLen = bufLen;
                XboxOneData::getInstance().authSequence = incomingXGIP.getSequence();
                XboxOneData::getInstance().authType = incomingXGIP.getCommand();
                XboxOneData::getInstance().xboneState = XboxOneState::send_auth_dongle_to_console;
            }// else {
                //printf("[XBOnePassthroughAddon::report_received]: Keep retrieving bytes from chunk\r\n");
            //}

            break;
        default:
            break;
    };

    if ( incomingXGIP.ackRequired() == true ) {
        //printf("[XBOnePassthroughAddon::report_received] Ack required, sending ack back\r\n");
        tuh_xinput_send_report(dev_addr, instance, (uint8_t*)incomingXGIP.generateAckPacket(), incomingXGIP.getPacketLength());
    }
}

void XBOnePassthroughAddon::report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    //printf("[XBOnePassthroughAddon::report_received] report_sent (%u)\r\n", len);
}

void XBOnePassthroughAddon::queue_host_report(void* report, uint16_t len) {
    //tuh_xinput_send_report(xbone_dev_addr, xbone_instance, (uint8_t*)report, len);
    if ( report_queue.size() < 8 ) {
		report_queue_t new_queue;
		memcpy(new_queue.report, report, len);
		new_queue.len = len;
		new_queue.timestamp = to_ms_since_boot(get_absolute_time());
		report_queue.push(new_queue);
	} else {
		printf("[queue_host_report ERROR] Cannot queue more than 8 reports\r\n");
	}
}
