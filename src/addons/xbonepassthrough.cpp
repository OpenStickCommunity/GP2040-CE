#include "addons/xbonepassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"

#include "xbone_driver.h"
#include "xgip_protocol.h"
#include "xinput_host.h"

#define XBONE_EXTENSION_DEBUG true

static GipPowerMode_t xb1_power_mode;
static GipPowerModeDesc_t xb1_power_desc;
static GipDescriptor_t xb1_descriptor_request;
static GipRumble_t xb1_rumble;

// unknown power-on
static uint8_t xb1_power_on[] = {0x06, 0x62, 0x45, 0xb8, 0x77, 0x26, 0x2c, 0x55,
                                 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f};

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
    if ( XboxOneData::getInstance().xboneState == XboxOneState::send_auth_console_to_dongle ) {
        //queue_host_report();
        // setup our console->dongle auth counters
        XboxOneData::getInstance().xboneState = XboxOneState::idle_state;
    }

    //if ( XboxOneData::getInstance().console_to_host_ready == true ) {
    //    XboxOneData::getInstance().console_to_host_len = 0;
    //    XboxOneData::getInstance().console_to_host_ready = false;
    //}
    // DOn't do this unless our dongle is ready!
    if ( !report_queue.empty() ) {
		// send the first report off our queue
		printf("[XBOnePassthroughAddon::process] Sending queued report to dongle size: %u (%u)\r\n", report_queue.front().len, report_queue.front().timestamp);
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
        incomingXGIP.reset();
        return;
    }

    switch ( incomingXGIP.getCommand() ) {
        case GIP_ACK_RESPONSE:
            // Only do something if we're waiting
            break;
        case GIP_ANNOUNCE:
            requestXGIP.reset();
            requestXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, 1, 1, false);
            tuh_xinput_send_report(dev_addr, instance, (uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
            break;
        case GIP_DEVICE_DESCRIPTOR:
            if ( incomingXGIP.endOfChunk() == true ) {
                // Power-on full string
                requestXGIP.reset();
                requestXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false);
                requestXGIP.setData(xb1_power_on, sizeof(xb1_power_on));
                //tuh_xinput_send_report(dev_addr, instance, (uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
                
                // Power-on with 0x01
                requestXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false);
                requestXGIP.setData({0x00}, 1);
                queue_host_report((uint8_t*)requestXGIP.generatePacket(), requestXGIP.getPacketLength());
            } else if ( incomingXGIP.ackRequired() == true ) {
                tuh_xinput_send_report(dev_addr, instance, (uint8_t*)incomingXGIP.generateAckPacket(), incomingXGIP.getPacketLength());
            }
            break;
        case GIP_AUTH:
			// Save this auth to our hand-off
            break;
        default:
            break;
    };
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
