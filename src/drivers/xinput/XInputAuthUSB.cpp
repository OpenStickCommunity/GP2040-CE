#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/xinput/XInputAuthUSB.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/xinput/XInputDescriptors.h"
#include "drivers/shared/xinput_host.h"

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
	uint8_t report[XINPUT_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;
static uint32_t lastReportQueueSent = 0;
#define REPORT_QUEUE_INTERVAL 15

void XInputAuthUSB::initialize() {
    dongle_ready = false;
    mounted = false;
}

bool XInputAuthUSB::available() {
	return (PeripheralManager::getInstance().isUSBEnabled(0));
}

void XInputAuthUSB::process() {
    if ( mounted == false ) // do nothing if we have not mounted an xbox one dongle
        return;
/*
    // Do not begin processing console auth unless we have the dongle ready
    if ( dongle_ready == true ) {
        if ( XboxOneData::getInstance().getState() == XboxOneState::send_auth_console_to_dongle ) {
            uint8_t isChunked = ( XboxOneData::getInstance().getAuthLen() > GIP_MAX_CHUNK_SIZE );
            uint8_t needsAck = (XboxOneData::getInstance().getAuthLen() > 2);
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(XboxOneData::getInstance().getAuthType(), XboxOneData::getInstance().getSequence(), 1, isChunked, needsAck);
            outgoingXGIP.setData(XboxOneData::getInstance().getAuthBuffer(), XboxOneData::getInstance().getAuthLen());
            XboxOneData::getInstance().setState(XboxOneState::wait_auth_console_to_dongle);
        } else if ( XboxOneData::getInstance().getState() == XboxOneState::wait_auth_console_to_dongle) {
            queue_host_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true ) {
                XboxOneData::getInstance().setState(XboxOneState::auth_idle_state);
            }
        }
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ( !report_queue.empty() && (now - lastReportQueueSent) > REPORT_QUEUE_INTERVAL  ) {
        if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, report_queue.front().report, report_queue.front().len) ) {
			report_queue.pop();
            lastReportQueueSent = now;
        } else { // FAILED: Keeping it on the queue to send again
            sleep_ms(REPORT_QUEUE_INTERVAL);
        }
	}
*/
}

void XInputAuthUSB::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    if ( controllerType == xinput_type_t::XBOX360) {
        xinput_dev_addr = dev_addr;
        xinput_instance = instance;
        mounted = true;
    }
}

void XInputAuthUSB::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    mounted = false;
}

void XInputAuthUSB::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false )
        return;
    /*
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
                XboxOneData::getInstance().setAuthData(incomingXGIP.getData(), incomingXGIP.getDataLength(), incomingXGIP.getSequence(),
                                                        incomingXGIP.getCommand(), XboxOneState::send_auth_dongle_to_console);
                incomingXGIP.reset();
            }
            break;
        case GIP_ACK_RESPONSE:
        default:
            break;
    };
    */
}

void XInputAuthUSB::queue_host_report(void* report, uint16_t len) {
    report_queue_t new_queue;
    memcpy(new_queue.report, report, len);
    new_queue.len = len;
    report_queue.push(new_queue);
}
