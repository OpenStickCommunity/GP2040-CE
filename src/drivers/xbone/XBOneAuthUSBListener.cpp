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
static uint8_t xb1_led_on[] = {0x00, 0x01, 0x14}; // 0x01 - LED on, 0x14 - Brightness

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;
static uint32_t lastReportQueue = 0;
#define REPORT_QUEUE_INTERVAL 15

void XBOneAuthUSBListener::setup() {
    xboxOneAuthData = nullptr;
    xbone_dev_addr = 0;
    xbone_instance = 0;
}

void XBOneAuthUSBListener::setAuthData(XboxOneAuthData * authData ) {
    xboxOneAuthData = authData;
    xboxOneAuthData->dongle_ready = false;
}

void XBOneAuthUSBListener::process() {
    // Do nothing if auth data or dongle are not ready 
    if ( mounted == false || xboxOneAuthData == nullptr) // do nothing if we have not mounted an xbox one dongle
        return;

    // Received a packet from the console (or Windows) to dongle
    if ( xboxOneAuthData->xboneState == GPAuthState::send_auth_console_to_dongle ) {
        uint8_t isChunked = ( xboxOneAuthData->consoleBuffer.length > GIP_MAX_CHUNK_SIZE );
        uint8_t needsAck = ( xboxOneAuthData->consoleBuffer.length > 2 );
        outgoingXGIP.reset();
        outgoingXGIP.setAttributes(xboxOneAuthData->consoleBuffer.type,
            xboxOneAuthData->consoleBuffer.sequence, 1, isChunked, needsAck);
        outgoingXGIP.setData(xboxOneAuthData->consoleBuffer.data, xboxOneAuthData->consoleBuffer.length);
        xboxOneAuthData->consoleBuffer.reset();
        xboxOneAuthData->xboneState = GPAuthState::wait_auth_console_to_dongle;
    }

    // Process waiting (always on first frame)
    if ( xboxOneAuthData->xboneState == GPAuthState::wait_auth_console_to_dongle) {
        queue_host_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
        if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true) {
            xboxOneAuthData->xboneState = GPAuthState::auth_idle_state;
        }
    }

    // Process the report queue
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
    if ( dev_addr == xbone_dev_addr ) {
        // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
        mounted = false;
        while (!report_queue.empty()) report_queue.pop();
        incomingXGIP.reset();
        outgoingXGIP.reset();
        xboxOneAuthData->dongle_ready = false; // not ready for auth if we unmounted
    }
}

void XBOneAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false || xboxOneAuthData == nullptr || dev_addr != xbone_dev_addr || instance != xbone_instance ) {
        return;
    }

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
            if ( incomingXGIP.endOfChunk() == true && xboxOneAuthData->dongle_ready != true) {
                outgoingXGIP.reset();  // Power-on full string
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false, 0);
                outgoingXGIP.setData(xb1_power_on, sizeof(xb1_power_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // Power-on with 0x00
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false, 0);
                outgoingXGIP.setData(xb1_power_on_single, sizeof(xb1_power_on_single));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // LED On
                outgoingXGIP.setAttributes(GIP_CMD_LED_ON, 1, 0, false, 0); // not internal function
                outgoingXGIP.setData(xb1_led_on, sizeof(xb1_led_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // Rumble Support to enable dongle
                outgoingXGIP.setAttributes(GIP_CMD_RUMBLE, 1, 0, false, 0); // not internal function
                outgoingXGIP.setData(xb1_rumble_on, sizeof(xb1_rumble_on));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                // Dongle is ready!
                xboxOneAuthData->dongle_ready = true; // dongle is ready
            }
            break;
        case GIP_AUTH:
        case GIP_FINAL_AUTH:
            if ( incomingXGIP.getChunked() == false || 
                (incomingXGIP.getChunked() == true && incomingXGIP.endOfChunk() == true )) {
                xboxOneAuthData->dongleBuffer.setBuffer(incomingXGIP.getData(), incomingXGIP.getDataLength(),
                    incomingXGIP.getSequence(), incomingXGIP.getCommand());
                xboxOneAuthData->xboneState = GPAuthState::send_auth_dongle_to_console;
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
    if ( mounted == true && !report_queue.empty() && (now - lastReportQueue) > REPORT_QUEUE_INTERVAL  ) {
        if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, report_queue.front().report, report_queue.front().len) ) {
			report_queue.pop();
            lastReportQueue = now; // last time we checked report queue
        } else {
            sleep_ms(REPORT_QUEUE_INTERVAL);
        }
	}
}
