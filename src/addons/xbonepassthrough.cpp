#include "addons/xbonepassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"

#include "xbone_driver.h"
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
    printf("Check Xbox One is Enabled?\n");
#endif
    const XBOnePassthroughOptions& xboneOptions = Storage::getInstance().getAddonOptions().xbonePassthroughOptions;
	return xboneOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void XBOnePassthroughAddon::setup() {

#if XBONE_EXTENSION_DEBUG==true
    printf("XBOne Passthrough Addon Enabled\n");
#endif

    // Packet Chunk Size set to 0
    packet_chunk_received = 0;
    awaiting_cb = false; // did we receive the sign state yet
}

void XBOnePassthroughAddon::process() {
    if (awaiting_cb)
        return;
}

void XBOnePassthroughAddon::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    xbone_dev_addr = dev_addr;
    xbone_instance = instance;

#if XBONE_EXTENSION_DEBUG==true
    printf("XBOnePassthroughAddon Saved values! dev_addr %u instance %u controllerType %u subType %u\r\n", dev_addr, instance, controllerType, subtype);
#endif
}

void XBOnePassthroughAddon::unmount(uint8_t dev_addr) {
    awaiting_cb = false; // did we receive the sign state yet
}

void XBOnePassthroughAddon::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    printf("[dev %u: instance %02x] X-Input Report:", dev_addr, instance);
    for(uint32_t i=0; i<len; i++)
    {
        if (i%16 == 0) printf("\r\n  ");
        printf("%02X ", report[i]);
    }
    printf("\r\n");

    GipHeader_t * header = (GipHeader_t*)report;
    switch ( header->command ) {
        case GIP_ANNOUNCE:
            printf("GIP_ANNOUNCE : sending descriptor request\r\n");
            GIP_HEADER((&xb1_descriptor_request), GIP_DEVICE_DESCRIPTOR, true, 1);
            tuh_xinput_send_report(dev_addr, instance, (uint8_t*)&xb1_descriptor_request, sizeof(xb1_descriptor_request));
            break;
        case GIP_DEVICE_DESCRIPTOR:
            printf("GIP_DEVICE_DESCRIPTOR :\r\n");
            if ( header->length == 0 ) {
                printf("Device Descriptor received! Power on controller!\r\n");
                packet_chunk_received = 0;
                
                GIP_HEADER((&xb1_power_desc), GIP_POWER_MODE_DEVICE_CONFIG, true, 2);
                memcpy(&xb1_power_desc.unknown[0], xb1_power_on, sizeof(xb1_power_on));
                tuh_xinput_send_report(dev_addr, instance, (uint8_t*)&xb1_power_desc, sizeof(xb1_power_desc));
                break;
            }
            if ( header->chunked != 1 ) {
                printf("Something went wrong, abort!\r\n");
                return;
            }
            // Should be chunked, let's check if its the start
            if ( header->chunkStart == 1 ) { // Start of Descriptor
                descriptor_size = *((uint16_t*)&report[4]); // descriptor size is in the first 2 bytes
                packet_chunk_received = header->length;
            } else { // Add to our total chunks received
                packet_chunk_received += header->length;
            }
            // Do we need to ack?
            if ( header->needsAck == 1 ) {
                send_xbone_ack(dev_addr, instance, header->sequence, packet_chunk_received, descriptor_size);
            }
            break;
        case GIP_AUTH:
            printf("GIP_AUTH : Received auth from controller\r\n");
            break;
        default:
            // unknown controller command
            break;
    };

    // Save a state, but for now just check for our announce status
    awaiting_cb = false;
}

void XBOnePassthroughAddon::report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    GipHeader_t * header = (GipHeader_t*)report;
    printf("[dev %u: instance %02x] X-Input Sent:", dev_addr, instance);
    for(uint32_t i=0; i<len; i++)
    {
        if (i%16 == 0) printf("\r\n  ");
        printf("%02X ", report[i]);
    }
    printf("\r\n");
    switch ( header->command ) {
        case GIP_POWER_MODE_DEVICE_CONFIG:
            if ( header->length == 0x0f ) {
                // send second request
                printf("Sending second half of power-up string\r\n");
                GIP_HEADER((&xb1_power_mode), GIP_POWER_MODE_DEVICE_CONFIG, true, 3);
                xb1_power_mode.subcommand = 0x00;
                tuh_xinput_send_report(dev_addr, instance, (uint8_t*)&xb1_power_mode, sizeof(xb1_power_mode));
            } else if ( header->length == 0x01 ) {
                // send rumble cmd
                printf("Turn off all rumble\r\n");
                GIP_HEADER((&xb1_rumble), GIP_CMD_RUMBLE, false, 1);
                xb1_rumble.subCommand = 0;
                xb1_rumble.flags = 0x0f;
                xb1_rumble.leftTrigger = 0x00;
                xb1_rumble.rightTrigger = 0x00;
                xb1_rumble.leftMotor = 0x00;
                xb1_rumble.rightMotor = 0x00;
                xb1_rumble.duration = 0xff;
                xb1_rumble.delay = 0x00;
                xb1_rumble.repeat = 0xeb;
                tuh_xinput_send_report(dev_addr, instance, (uint8_t*)&xb1_rumble, sizeof(xb1_rumble));
            }
            break;
        case GIP_CMD_RUMBLE:
            printf("Ready for Auth!!!!\r\n");
            break;
        default:
            // unknown controller command
            break;
    };
}

// send an ACK packet
void XBOnePassthroughAddon::send_xbone_ack(uint8_t dev_addr, uint8_t instance, uint8_t sequence, uint16_t received, uint16_t total_size) {
    Gip_Ack_t ackPacket;
    memset(&ackPacket, 0x00, sizeof(Gip_Ack_t));
    GIP_ACK_HEADER((&ackPacket), sequence);
    ackPacket.innerCommand = GIP_DEVICE_DESCRIPTOR;
    ackPacket.innerInternal = 1;
    if ( received > 0x100 ) {
        ackPacket.bytesReceived = received - 0x100;
    } else {
        ackPacket.bytesReceived = received;
    }
    ackPacket.remainingBuffer = total_size - received;
    tuh_xinput_send_report(dev_addr, instance, (uint8_t*)&ackPacket, sizeof(Gip_Ack_t));
}
