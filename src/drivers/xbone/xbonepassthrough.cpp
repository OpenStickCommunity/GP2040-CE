#include "addons/xbonepassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"

#include "xbone_driver.h"
#include "xgip_protocol.h"
#include "xinput_host.h"

#define XBONE_EXTENSION_DEBUG true

// power-on states and rumble-on with everything disabled
static uint8_t xb1_power_on[] = {0x06, 0x62, 0x45, 0xb8, 0x77, 0x26, 0x2c, 0x55,
                                 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f};
static uint8_t xb1_power_on_single[] = {0x00};
static uint8_t xb1_rumble_on[] = {0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xeb};

bool XBOnePassthroughAddon::available() {
    const XBOnePassthroughOptions& xboneOptions = Storage::getInstance().getAddonOptions().xbonePassthroughOptions;
    const GamepadOptions& gamepadOptions = Storage::getInstance().GetGamepad()->getOptions();
	return xboneOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0) && (gamepadOptions.inputMode == INPUT_MODE_XBONE);
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

void XBOnePassthroughAddon::process() {
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
}

void XBOnePassthroughAddon::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    xbone_dev_addr = dev_addr;
    xbone_instance = instance;
    incomingXGIP.reset();
    outgoingXGIP.reset();
}

void XBOnePassthroughAddon::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
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
}

void XBOnePassthroughAddon::queue_host_report(void* report, uint16_t len) {
    report_queue_t new_queue;
    memcpy(new_queue.report, report, len);
    new_queue.len = len;
    report_queue.push(new_queue);
}
