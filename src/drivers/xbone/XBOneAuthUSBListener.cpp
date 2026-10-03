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

// Drained only on core 0: TinyUSB host is not safe to call from both cores
typedef struct {
	uint8_t report[XBONE_ENDPOINT_SIZE];
	uint16_t len;
} report_queue_t;

#define REPORT_QUEUE_DEPTH 32
static queue_t report_queue;
static bool report_queue_ready = false;
static uint32_t lastReportQueue = 0;
#define REPORT_QUEUE_INTERVAL 15

// Controllers are relayed raw. Mayflash-family dongles (announce vendor 0x33DF,
// or USB VID 0x0079) use the reconstructed-message flow. USB VID alone can't tell
// them apart (Brook XB2 = 045E:02EA, Booter X = PowerA's VID).
static uint8_t manufacturer_desc[64];
static constexpr uint16_t MAYFLASH_ANNOUNCE_VID = 0x33DF;
static constexpr uint16_t MAGICBOOTS_USB_VID = 0x0079;
static bool relayManufacturer = false;
static bool dongleDetected = false;

static void update_path(XboxOneAuthData * authData) {
    if ( authData != nullptr )
        authData->auth_passthrough_enabled = relayManufacturer || !dongleDetected;
}

#define XBONE_ANNOUNCE_TIMEOUT_MS 250

// Always relayed, regardless of announce
static const char * const RELAY_MANUFACTURERS[] = { "Microsoft", "Brook", "PowerA" };

static void manufacturer_string_cb(tuh_xfer_t * xfer) {
    XboxOneAuthData * authData = (XboxOneAuthData *)xfer->user_data;
    if ( authData == nullptr || xfer->result != XFER_RESULT_SUCCESS || xfer->actual_len < 2 ) {
        return;
    }

    const uint16_t len = TU_MIN(manufacturer_desc[0], (uint16_t)xfer->actual_len);
    const uint16_t chars = (len > 2) ? (len - 2) / 2 : 0;
    for (const char * name : RELAY_MANUFACTURERS) {
        const size_t n = strlen(name);
        if ( chars < n ) continue;
        bool match = true;
        for (size_t i = 0; i < n && match; i++) {
            match = manufacturer_desc[2 + (i * 2)] == (uint8_t)name[i] && manufacturer_desc[3 + (i * 2)] == 0;
        }
        if ( match ) {
            relayManufacturer = true;
            update_path(authData);
            return;
        }
    }
}

void XBOneAuthUSBListener::setup() {
    xboxOneAuthData = nullptr;
    xbone_dev_addr = 0;
    xbone_instance = 0;
    if ( !report_queue_ready ) {
        queue_init(&report_queue, sizeof(report_queue_t), REPORT_QUEUE_DEPTH);
        report_queue_ready = true;
    }
    mounted = false;
    heardFromDevice = false;
    initKicked = false;
    mountTime = 0;
}

void XBOneAuthUSBListener::setAuthData(XboxOneAuthData * authData ) {
    xboxOneAuthData = authData;
    xboxOneAuthData->dongle_ready = false;
}

void XBOneAuthUSBListener::process() {
    // Do nothing if auth data or dongle are not ready
    if ( mounted == false || xboxOneAuthData == nullptr) // do nothing if we have not mounted an xbox one dongle
        return;

    if ( xboxOneAuthData->auth_passthrough )
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

    if ( xboxOneAuthData->xboneState == GPAuthState::wait_auth_console_to_dongle ) {
        queue_host_report(outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
        if ( outgoingXGIP.getChunked() == false || outgoingXGIP.endOfChunk() == true) {
            xboxOneAuthData->xboneState = GPAuthState::auth_idle_state;
        }
    }

}

// Core 0 only
void XBOneAuthUSBListener::processHost() {
    if ( mounted == false || xboxOneAuthData == nullptr )
        return;

    // Some pads (PowerA, original Xbox One pad) never send ANNOUNCE: ask for the
    // descriptor ourselves so they get powered on
    if ( !heardFromDevice && !initKicked &&
        (to_ms_since_boot(get_absolute_time()) - mountTime) > XBONE_ANNOUNCE_TIMEOUT_MS ) {
        initKicked = true;
        static const uint8_t descriptorRequest[] = { GIP_DEVICE_DESCRIPTOR, 0x20, 0x01, 0x00 };
        queue_host_report((void*)descriptorRequest, sizeof(descriptorRequest));
    }

    if ( xboxOneAuthData->auth_passthrough ) {
        XBOneRelayPacket packet;
        while ( queue_try_peek(&xboxOneAuthData->relayToDevice, &packet) ) {
            if ( !tuh_xinput_send_report(xbone_dev_addr, xbone_instance, packet.data, packet.len) )
                return;
            queue_try_remove(&xboxOneAuthData->relayToDevice, &packet);
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
        uint16_t usbVid = 0, usbPid = 0;
        tuh_vid_pid_get(dev_addr, &usbVid, &usbPid);
        relayManufacturer = false;
        dongleDetected = (usbVid == MAGICBOOTS_USB_VID);
        update_path(xboxOneAuthData);
        xboxOneAuthData->auth_passthrough = false;
        heardFromDevice = false;
        initKicked = false;
        mountTime = to_ms_since_boot(get_absolute_time());
        mounted = true;
        xboxOneAuthData->deviceMounted = true;
        if ( xboxOneAuthData->consoleAuthOrphaned && !xboxOneAuthData->authCompleted )
            xboxOneAuthData->reconnectRequested = true;

        memset(manufacturer_desc, 0, sizeof(manufacturer_desc));
        tuh_descriptor_get_manufacturer_string(dev_addr, 0x0409, manufacturer_desc,
            sizeof(manufacturer_desc), manufacturer_string_cb, (uintptr_t)xboxOneAuthData);
    }
}

void XBOneAuthUSBListener::unmount(uint8_t dev_addr) {
    if ( dev_addr == xbone_dev_addr ) {
        // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
        mounted = false;
        xboxOneAuthData->deviceMounted = false;
        report_queue_t dropped;
        while ( queue_try_remove(&report_queue, &dropped) );
        incomingXGIP.reset();
        outgoingXGIP.reset();
        xboxOneAuthData->auth_passthrough_enabled = false;
        xboxOneAuthData->auth_passthrough = false;
        XBOneRelayPacket packet;
        while ( queue_try_remove(&xboxOneAuthData->relayToDevice, &packet) );
        while ( queue_try_remove(&xboxOneAuthData->relayToConsole, &packet) );
        xboxOneAuthData->dongle_ready = false; // not ready for auth if we unmounted
    }
}

void XBOneAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false || xboxOneAuthData == nullptr || dev_addr != xbone_dev_addr || instance != xbone_instance ) {
        return;
    }

    if ( xboxOneAuthData->auth_passthrough ) {
        if ( xbone_is_auth_packet(report, len) ) {
            XBOneRelayPacket packet;
            packet.len = TU_MIN(len, (uint16_t)XBONE_RELAY_PACKET_SIZE);
            memcpy(packet.data, report, packet.len);
            if ( !queue_try_add(&xboxOneAuthData->relayToConsole, &packet) )
                xboxOneAuthData->relayDropped++;
            return;
        }
    }

    incomingXGIP.parse(report, len);
    if ( incomingXGIP.validate() == false ) {
        sleep_ms(50); // First packet is invalid, drop and wait for dongle to boot
        incomingXGIP.reset();
        return;
    }

    heardFromDevice = true;

    // Setup an ack before we change anything about the incoming packet
    if ( incomingXGIP.ackRequired() == true ) {
        queue_host_report((uint8_t*)incomingXGIP.generateAckPacket(), incomingXGIP.getPacketLength());
    }

    switch ( incomingXGIP.getCommand() ) {
        case GIP_ANNOUNCE:
            // Announce data: vendor at [8..9]
            if ( incomingXGIP.getDataLength() >= 12 ) {
                const uint16_t announceVid = incomingXGIP.getData()[8] | (incomingXGIP.getData()[9] << 8);
                if ( announceVid == MAYFLASH_ANNOUNCE_VID ) {
                    dongleDetected = true;
                    update_path(xboxOneAuthData);
                }
            }
            outgoingXGIP.reset();
            outgoingXGIP.setAttributes(GIP_DEVICE_DESCRIPTOR, 1, 1, false, 0);
            queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());
            break;
        case GIP_DEVICE_DESCRIPTOR:
            if ( incomingXGIP.endOfChunk() == true && xboxOneAuthData->dongle_ready != true) {
                outgoingXGIP.reset();  // Power-on full string
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 2, 1, false, 0);
                outgoingXGIP.setData(XBOXONE_POWER_ON, sizeof(XBOXONE_POWER_ON));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // Power-on with 0x00
                outgoingXGIP.setAttributes(GIP_POWER_MODE_DEVICE_CONFIG, 3, 1, false, 0);
                outgoingXGIP.setData(XBOXONE_POWER_ON_SINGLE, sizeof(XBOXONE_POWER_ON_SINGLE));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // LED On
                outgoingXGIP.setAttributes(GIP_CMD_LED_ON, 1, 0, false, 0); // not internal function
                outgoingXGIP.setData(XBOXONE_LED_ON, sizeof(XBOXONE_LED_ON));
                queue_host_report((uint8_t*)outgoingXGIP.generatePacket(), outgoingXGIP.getPacketLength());

                outgoingXGIP.reset();  // Rumble Support to enable dongle
                outgoingXGIP.setAttributes(GIP_CMD_RUMBLE, 1, 0, false, 0); // not internal function
                outgoingXGIP.setData(XBOXONE_RUMBLE_ON, sizeof(XBOXONE_RUMBLE_ON));
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
    new_queue.len = TU_MIN(len, (uint16_t)XBONE_ENDPOINT_SIZE);
    memcpy(new_queue.report, report, new_queue.len);
    queue_try_add(&report_queue, &new_queue);
}

void XBOneAuthUSBListener::process_report_queue() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    report_queue_t item;
    if ( (now - lastReportQueue) > REPORT_QUEUE_INTERVAL && queue_try_peek(&report_queue, &item) ) {
        if ( tuh_xinput_send_report(xbone_dev_addr, xbone_instance, item.report, item.len) ) {
            queue_try_remove(&report_queue, &item);
            lastReportQueue = now;
        }
    }
}
