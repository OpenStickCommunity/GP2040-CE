#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/xbone/XBOneAuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/xbone/XBOneDescriptors.h"
#include "drivers/shared/xgip_protocol.h"
#include "drivers/shared/xinput_host.h"

// power-on states and rumble-on with everything disabled
static uint8_t xb1_power_on[] = {0x06, 0x62, 0x45, 0xb8, 0x77, 0x26, 0x2c, 0x55,
                                 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f};
static uint8_t xb1_power_on_single[] = {0x00};
static uint8_t xb1_rumble_on[] = {0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xeb};

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;
static uint32_t lastReportQueueSent = 0;
#define REPORT_QUEUE_INTERVAL 15

void XBOneAuthUSBListener::setup() {
    dongle_ready = false;
    mounted = false;
    xboxOneAuthData = nullptr;
}

void XBOneAuthUSBListener::setAuthData(XboxOneAuthData * authData ) {
    xboxOneAuthData = authData;
}

void XBOneAuthUSBListener::process() {
    if ( mounted == false || xboxOneAuthData == nullptr) // do nothing if we have not mounted an xbox one dongle
        return;

    // Do not begin processing console auth unless we have the dongle ready
    if ( dongle_ready == true ) {
        if ( xboxOneAuthData->xboneState == XboxOneState::send_auth_console_to_dongle ) {
            uint8_t isChunked = ( xboxOneAuthData->authLen > GIP_MAX_CHUNK_SIZE );
            uint8_t needsAck = (xboxOneAuthData->authLen > 2);
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(xboxOneAuthData->authType, xboxOneAuthData->authSequence, 1, isChunked, needsAck);
            outgoingXGIP.setData(xboxOneAuthData->authBuffer, xboxOneAuthData->authLen);
            xboxOneAuthData->xboneState = XboxOneState::wait_auth_console_to_dongle;
        } else if ( xboxOneAuthData->xboneState == XboxOneState::wait_auth_console_to_dongle) {
            queue_host_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true ) {
                xboxOneAuthData->xboneState = XboxOneState::auth_idle_state;
            }
        }
    }

    process_report_queue();
}

void XBOneAuthUSBListener::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    if ( controllerType == xinput_type_t::XBOXONE) {
        xbone_dev_addr = dev_addr;
        xbone_instance = instance;
        incomingXGIP.reset();
        outgoingXGIP.reset();
        mounted = true;
    }
}

void XBOneAuthUSBListener::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    mounted = false;
}

void XBOneAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false || xboxOneAuthData == nullptr)
        return;

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
        case GIP_ANNOUNCE:
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, 1, 1, false, 0);
            queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            break;
        case GIP_DEVICE_DESCRIPTOR:
            if ( incomingXGIP.endOfChunk() == true ) {
                outgoingXGIP.reset();  // Power-on full string
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false, 0);
                outgoingXGIP.setData(xb1_power_on, sizeof(xb1_power_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
                outgoingXGIP.reset();  // Power-on with 0x00
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false, 0);
                outgoingXGIP.setData(xb1_power_on_single, sizeof(xb1_power_on_single));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
                outgoingXGIP.reset();  // Rumble Support to enable dongle
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
                memcpy(xboxOneAuthData->authBuffer, incomingXGIP.getData(), incomingXGIP.getDataLength());
				xboxOneAuthData->authLen = incomingXGIP.getDataLength();
				xboxOneAuthData->authType = incomingXGIP.getCommand();
				xboxOneAuthData->authSequence = incomingXGIP.getSequence();
				xboxOneAuthData->xboneState = XboxOneState::send_auth_dongle_to_console;
                incomingXGIP.reset();
            }
            break;
        case GIP_ACK_RESPONSE:
        default:
            break;
    };
}

void XBOneAuthUSBListener::queue_host_report(void* report, uint16_t len) {
    report_queue_t new_queue;
    memcpy(new_queue.report, report, len);
    new_queue.len = len;
    report_queue.push(new_queue);
}

void XBOneAuthUSBListener::process_report_queue() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ( !report_queue.empty() && (now - lastReportQueueSent) > REPORT_QUEUE_INTERVAL  ) {
        if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, report_queue.front().report, report_queue.front().len) ) {
			report_queue.pop();
            lastReportQueueSent = now;
        } else { // FAILED: Keeping it on the queue to send again
            sleep_ms(REPORT_QUEUE_INTERVAL);
        }
	}
}
