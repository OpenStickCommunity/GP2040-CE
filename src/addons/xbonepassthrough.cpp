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

// Data passed between PS Passthrough and TinyUSB Host callbacks

bool XBOnePassthroughAddon::available() {
    const XBOnePassthroughOptions& xboneOptions = Storage::getInstance().getAddonOptions().xbonePassthroughOptions;
	return xboneOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void XBOnePassthroughAddon::setup() {
    dongle_ready = false;
}

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;
static uint32_t lastReportQueueSent = 0;
#define REPORT_QUEUE_INTERVAL 15
#define REPORT_QUEUE_MAX 16

void XBOnePassthroughAddon::process() {
    // Do not begin processing console auth unless we have the dongle ready
    if ( dongle_ready == true ) {
        if ( XboxOneData::getInstance().xboneState == XboxOneState::send_auth_console_to_dongle ) {
            uint8_t isChunked = ( XboxOneData::getInstance().authLen > GIP_MAX_CHUNK_SIZE );
            uint8_t needsAck = (XboxOneData::getInstance().authLen > 2);
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(XboxOneData::getInstance().authType, XboxOneData::getInstance().authSequence, 1, isChunked, needsAck);
            outgoingXGIP.setData(XboxOneData::getInstance().authBuffer, XboxOneData::getInstance().authLen);
            XboxOneData::getInstance().xboneState = XboxOneState::wait_auth_console_to_dongle;
        } else if ( XboxOneData::getInstance().xboneState == XboxOneState::wait_auth_console_to_dongle) {
            queue_host_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true ) {
                XboxOneData::getInstance().xboneState = XboxOneState::idle_state;
            }
        }
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ( !report_queue.empty() && (now - lastReportQueueSent) > REPORT_QUEUE_INTERVAL  ) {
        if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, report_queue.front().report, report_queue.front().len) ) {
			report_queue.pop();
            lastReportQueueSent = now;
        } else { // FAILED: Keeping it on the queue to send again
            sleep_ms(50);
        }
	}
}

void XBOnePassthroughAddon::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    xbone_dev_addr = dev_addr;
    xbone_instance = instance;
}

void XBOnePassthroughAddon::unmount(uint8_t dev_addr) {
}

void XBOnePassthroughAddon::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    incomingXGIP.parse(report, len);
    if ( incomingXGIP.validate() == false ) {
        sleep_ms(50); // First packet is invalid, drop and wait for dongle to boot
        incomingXGIP.reset();
        return;
    }

    // Setup an ack before we change anything about the incoming packet
    if ( incomingXGIP.ackRequired() == true ) {
        queue_host_report((uint8_t*)incomingXGIP.generateAckPacket(), incomingXGIP.getPacketLength());
    }

    switch ( incomingXGIP.getCommand() ) {
        case GIP_ACK_RESPONSE:
            // Only do something if we're waiting
            break;
        case GIP_ANNOUNCE:
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, 1, 1, false, 0);
            queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            break;
        case GIP_DEVICE_DESCRIPTOR:
            if ( incomingXGIP.endOfChunk() == true ) {
                // Power-on full string
                outgoingXGIP.reset();
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false, 0);
                outgoingXGIP.setData(xb1_power_on, sizeof(xb1_power_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
                
                // Power-on with 0x01
                outgoingXGIP.reset();
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false, 0);
                outgoingXGIP.setData({0x00}, 1);
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                // Rumble Support to enable dongle
                outgoingXGIP.reset();
                outgoingXGIP.setAttributes(GIP_CMD_RUMBLE, 1, 0, false, 0); // not internal function
                outgoingXGIP.setData(xb1_rumble_on, sizeof(xb1_rumble_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                dongle_ready = true;
            }
            break;
        case GIP_AUTH:
        case GIP_FINAL_AUTH:
           if ( incomingXGIP.getChunked() == false || 
                    (incomingXGIP.getChunked() == true && incomingXGIP.endOfChunk() == true )) {
                uint8_t * buf = incomingXGIP.getData();
                uint16_t bufLen = incomingXGIP.getDataLength();
                memcpy(XboxOneData::getInstance().authBuffer, buf, bufLen);
                XboxOneData::getInstance().authLen = bufLen;
                XboxOneData::getInstance().authSequence = incomingXGIP.getSequence();
                XboxOneData::getInstance().authType = incomingXGIP.getCommand();
                XboxOneData::getInstance().xboneState = XboxOneState::send_auth_dongle_to_console;
                incomingXGIP.reset();
            }
            break;
        default:
            break;
    };
}

void XBOnePassthroughAddon::report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
}

void XBOnePassthroughAddon::queue_host_report(void* report, uint16_t len) {
    if ( report_queue.size() < REPORT_QUEUE_MAX ) {
		report_queue_t new_queue;
		memcpy(new_queue.report, report, len);
		new_queue.len = len;
		report_queue.push(new_queue);
	} else {
		//printf("[queue_host_report ERROR] Cannot queue more than 8 reports\r\n");
	}
}
